#include "monkeypatch.h"

#ifdef SCR_MONKEYPATCH

#ifndef __linux__
#error "Monkeypatching is only available on Linux."
#endif

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <elfjack/elfjack.h>
#include <reap/reap.h>

struct fileRecord {
    linkedListNode node;
    char *path;
    ejAddr file_start;
    ino_t inode;
    dev_t device;
    unsigned int is_elf : 1;
};

struct patchInfo {
    linkedListNode node;
    char *func_name;
    void *real_addr;
};

static linkedList file_records;
static linkedList patched_functions;
static struct fileRecord *scrutiny_record;

static void
fileRecordFree(void *item)
{
    struct fileRecord *record = item;

    free(record->path);
}

static void
patchInfoFree(void *item)
{
    struct patchInfo *info = item;

    free(info->func_name);
}

static void
freeMonkeypatchData(void)
{
    linkedListFree(&file_records, fileRecordFree);
    linkedListFree(&patched_functions, patchInfoFree);
}

static bool
haveSeen(dev_t device, ino_t inode)
{
    struct fileRecord *record;

    LINKED_LIST_ITERATE(&file_records, record)
    {
        if (record->inode == inode && record->device == device) {
            return true;
        }
    }

    return false;
}

static void
searchForGotEntry(const ejElfInfo *info, const char *func_name, linkedList *got_entries, ejAddr file_start)
{
    scrGotEntry *entry;
    ejAddr addr;

    addr = ejFindGotEntry(info, func_name);
    if (addr == EJ_ADDR_NOT_FOUND) {
        return;
    }

    entry = linkedListNodeNew(sizeof(*entry));
    entry->entry = (void *)(uintptr_t)ejResolveAddress(info, addr, file_start);
    linkedListAppend(got_entries, &entry->node);
}

static void
addPatchInfo(const char *func_name, ejAddr addr)
{
    struct patchInfo *info;

    LINKED_LIST_ITERATE(&patched_functions, info)
    {
        if (strcmp(info->func_name, func_name) == 0) {
            return;
        }
    }

    info = linkedListNodeNew(sizeof(*info));
    info->func_name = strdup(func_name);
    if (!info->func_name) {
        exit(1);
    }
    info->real_addr = (void *)(uintptr_t)addr;

    linkedListAppend(&patched_functions, &info->node);
}

static bool
populateRecords(const char *func_name, linkedList *got_entries)
{
    int ret;
    ejAddr addr = EJ_ADDR_NOT_FOUND;
    char path[PATH_MAX];
    reapMapIterator *iterator;
    reapMapResult result;

    atexit(freeMonkeypatchData);

    if (reapMapIteratorCreate(getpid(), &iterator) != REAP_RET_OK) {
        fprintf(stderr, "reapMapIteratorCreate: %s\n", reapGetError());
        exit(1);
    }

    while ((ret = reapMapIteratorNext(iterator, &result, path, sizeof(path))) == REAP_RET_OK) {
        struct fileRecord *record;
        ejElfInfo info;

        if (path[0] != '/' || haveSeen(result.device, result.inode)) {
            continue;
        }

        record = linkedListNodeNew(sizeof(*record));
        record->device = result.device;
        record->inode = result.inode;

        if (ejParseElf(path, &info) == EJ_RET_OK) {
            record->is_elf = true;
            record->file_start = result.start;
            record->path = strdup(path);
            if (!record->path) {
                exit(1);
            }

            if (!scrutiny_record && ejFindFunction(&info, "scrRun") != EJ_ADDR_NOT_FOUND) {
                scrutiny_record = record;
            }
            else if (addr == EJ_ADDR_NOT_FOUND &&
                     (addr = ejFindFunction(&info, func_name)) != EJ_ADDR_NOT_FOUND) {
                addr = ejResolveAddress(&info, addr, result.start);
            }
            else {
                searchForGotEntry(&info, func_name, got_entries, result.start);
            }

            ejReleaseInfo(&info);
        }
        else {
            record->path = NULL;
            record->is_elf = false;
        }

        linkedListAppend(&file_records, &record->node);
    }

    reapMapIteratorDestroy(iterator);
    if (ret != REAP_RET_DONE) {
        fprintf(stderr, "reapMapIteratorNext: %s\n", reapGetError());
        exit(1);
    }

    if (!scrutiny_record) {
        fprintf(stderr, "libscrutiny.so not found in memory\n");
        exit(1);
    }

    if (addr == EJ_ADDR_NOT_FOUND) {
        return false;
    }

    addPatchInfo(func_name, addr);

    return true;
}

bool
findFunction(const char *func_name, linkedList *got_entries)
{
    ejAddr addr = EJ_ADDR_NOT_FOUND;
    struct fileRecord *record;

    if (!file_records.head) {
        return populateRecords(func_name, got_entries);
    }

    LINKED_LIST_ITERATE(&file_records, record)
    {
        ejElfInfo info;

        if (!record->is_elf || record == scrutiny_record) {
            continue;
        }

        if (ejParseElf(record->path, &info) != EJ_RET_OK) {
            fprintf(stderr, "Failed to parse %s: %s\n", record->path, ejGetError());
            exit(1);
        }

        if (addr == EJ_ADDR_NOT_FOUND && (addr = ejFindFunction(&info, func_name)) != EJ_ADDR_NOT_FOUND) {
            addr = ejResolveAddress(&info, addr, record->file_start);
        }
        else {
            searchForGotEntry(&info, func_name, got_entries, record->file_start);
        }
        ejReleaseInfo(&info);
    }

    if (addr == EJ_ADDR_NOT_FOUND) {
        return false;
    }

    addPatchInfo(func_name, addr);

    return true;
}

void *
scrPatchedFunction(const char *func_name)
{
    struct patchInfo *info;

    LINKED_LIST_ITERATE(&patched_functions, info)
    {
        if (strcmp(info->func_name, func_name) == 0) {
            return info->real_addr;
        }
    }

    return NULL;
}

void
patchGoalFree(void *item)
{
    scrPatchGoal *goal = item;

    free(goal->func_name);
    linkedListFree(&goal->got_entries, NULL);
}

#else  // SCR_MONKEYPATCH

#include <stddef.h>

void *
scrPatchedFunction(const char *func_name)
{
    (void)func_name;
    return NULL;
}

#endif  // SCR_MONKEYPATCH
