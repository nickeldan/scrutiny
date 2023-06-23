#pragma once

#include "internal.h"

#ifdef SCR_MONKEYPATCH

bool
findFunction(const char *func_name, linkedList *got_entries);

void
patchGoalFree(void *item);

#endif  // SCR_MONKEYPATCH
