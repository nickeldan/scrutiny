#!/bin/sh -ex

make tests

find . -path ./packages -prune -o -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format --dry-run --Werror

scripts/check_visibility.py
