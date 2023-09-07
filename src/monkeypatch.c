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
    char *path;
    ejAddr file_start;
    ino_t inode;
    dev_t device;
    unsigned int is_elf : 1;
};

struct patchInfo {
    char *func_name;
    void *real_addr;
};

static gear file_records;
static gear patched_functions;
static size_t scrutiny_idx;

static void
freeMonkeypatchData(void)
{
    struct fileRecord *record;
    struct patchInfo *info;

    GEAR_FOR_EACH(&file_records, record)
    {
        free(record->path);
    }
    gearReset(&file_records);

    GEAR_FOR_EACH(&patched_functions, info)
    {
        free(info->func_name);
    }
    gearReset(&patched_functions);
}

static bool
haveSeen(dev_t device, ino_t inode)
{
    struct fileRecord *record;

    GEAR_FOR_EACH(&file_records, record)
    {
        if (record->inode == inode && record->device == device) {
            return true;
        }
    }

    return false;
}

static void
searchForGotEntry(const ejElfInfo *info, const char *func_name, gear *got_entries, ejAddr file_start)
{
    ejAddr entry;
    void *ptr;

    entry = ejFindGotEntry(info, func_name);
    if (entry == EJ_ADDR_NOT_FOUND) {
        return;
    }

    ptr = (void *)(uintptr_t)ejResolveAddress(info, entry, file_start);
    if (gearAppend(got_entries, &ptr) != GEAR_RET_OK) {
        exit(1);
    }
}

static void
addPatchInfo(const char *func_name, ejAddr addr)
{
    struct patchInfo info;
    struct patchInfo *ptr;

    GEAR_FOR_EACH(&patched_functions, ptr)
    {
        if (strcmp(ptr->func_name, func_name) == 0) {
            return;
        }
    }

    info.func_name = strdup(func_name);
    if (!info.func_name) {
        exit(1);
    }
    info.real_addr = (void *)(uintptr_t)addr;

    if (gearAppend(&patched_functions, &info) != GEAR_RET_OK) {
        exit(1);
    }
}

static bool
populateRecords(const char *func_name, const char *file_substring, gear *got_entries)
{
    int ret;
    bool found_scrutiny = false;
    ejAddr func_addr = EJ_ADDR_NOT_FOUND;
    char path[PATH_MAX];
    reapMapIterator *iterator;
    reapMapResult result;

    gearInit(&file_records, sizeof(struct fileRecord));
    gearSetExpansion(&file_records, 5, 5);
    gearInit(&patched_functions, sizeof(struct patchInfo));
    atexit(freeMonkeypatchData);

    if (reapMapIteratorCreate(getpid(), &iterator) != REAP_RET_OK) {
        fprintf(stderr, "reapMapIteratorCreate: %s\n", reapGetError());
        exit(1);
    }

    while ((ret = reapMapIteratorNext(iterator, &result, path, sizeof(path))) == REAP_RET_OK) {
        struct fileRecord record = {0};
        ejElfInfo info;

        if (path[0] != '/' || haveSeen(result.device, result.inode)) {
            continue;
        }

        record.device = result.device;
        record.inode = result.inode;

        if (ejParseElf(path, &info) == EJ_RET_OK) {
            record.is_elf = true;
            record.file_start = result.start;
            record.path = strdup(path);
            if (!record.path) {
                exit(1);
            }

            if (!found_scrutiny && ejFindFunction(&info, "scrRun") != EJ_ADDR_NOT_FOUND) {
                scrutiny_idx = file_records.length;
                found_scrutiny = true;
            }
            else if (func_addr == EJ_ADDR_NOT_FOUND &&
                     (func_addr = ejFindFunction(&info, func_name)) != EJ_ADDR_NOT_FOUND) {
                func_addr = ejResolveAddress(&info, func_addr, result.start);
            }
            else if (!file_substring || strstr(path, file_substring)) {
                searchForGotEntry(&info, func_name, got_entries, result.start);
            }

            ejReleaseInfo(&info);
        }

        if (gearAppend(&file_records, &record) != GEAR_RET_OK) {
            exit(1);
        }
    }

    reapMapIteratorDestroy(iterator);
    if (ret != REAP_RET_DONE) {
        fprintf(stderr, "reapMapIteratorNext: %s\n", reapGetError());
        exit(1);
    }

    if (!found_scrutiny) {
        fprintf(stderr, "libscrutiny.so not found in memory\n");
        exit(1);
    }

    if (func_addr == EJ_ADDR_NOT_FOUND) {
        return false;
    }

    addPatchInfo(func_name, func_addr);

    return true;
}

bool
findFunction(const char *func_name, const char *file_substring, gear *got_entries)
{
    size_t idx;
    ejAddr func_addr = EJ_ADDR_NOT_FOUND;
    struct fileRecord *record;

    if (file_records.item_size == 0) {
        return populateRecords(func_name, file_substring, got_entries);
    }

    GEAR_FOR_EACH_WITH_INDEX(&file_records, record, idx)
    {
        ejElfInfo info;

        if (!record->is_elf || idx == scrutiny_idx) {
            continue;
        }

        if (ejParseElf(record->path, &info) != EJ_RET_OK) {
            fprintf(stderr, "Failed to parse %s: %s\n", record->path, ejGetError());
            exit(1);
        }

        if (func_addr == EJ_ADDR_NOT_FOUND &&
            (func_addr = ejFindFunction(&info, func_name)) != EJ_ADDR_NOT_FOUND) {
            func_addr = ejResolveAddress(&info, func_addr, record->file_start);
        }
        else if (!file_substring || strstr(record->path, file_substring)) {
            searchForGotEntry(&info, func_name, got_entries, record->file_start);
        }
        ejReleaseInfo(&info);
    }

    if (func_addr == EJ_ADDR_NOT_FOUND) {
        return false;
    }

    addPatchInfo(func_name, func_addr);

    return true;
}

void *
scrPatchedFunction(const char *func_name)
{
    struct patchInfo *info;

    GEAR_FOR_EACH(&patched_functions, info)
    {
        if (strcmp(info->func_name, func_name) == 0) {
            return info->real_addr;
        }
    }

    return NULL;
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
