#ifndef SCRUTINY_MONKEYPATCH_H
#define SCRUTINY_MONKEYPATCH_H

#include "internal.h"

#ifdef SCR_MONKEYPATCH

bool
findFunction(const char *func_name, gear *got_entries);

#endif  // SCR_MONKEYPATCH

#endif  // SCRUTINY_MONKEYPATCH_H
