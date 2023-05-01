#ifndef SCRUTINY_MONKEYPATCH_H
#define SCRUTINY_MONKEYPATCH_H

#ifdef SCR_MONKEYPATCH

#include <stdbool.h>

#include <gear/gear.h>

bool
findFunction(const char *func_name, gear *got_entries);

#endif  // SCR_MONKEYPATCH

#endif  // SCRUTINY_MONKEYPATCH_H
