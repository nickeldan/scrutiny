#!/usr/bin/env python3

import re
import subprocess

SYM_PATTERN = re.compile(r"\s+T\s+(\w+)$")
VIS_PATTERN = re.compile(r"\s*(\w+);$")


def get_symbols():
    p = subprocess.run("nm -D libscrutiny.so", shell=True, check=True, stdout=subprocess.PIPE, text=True)
    for line in p.stdout.splitlines():
        match = SYM_PATTERN.search(line)
        if match:
            yield match.group(1)


def get_vis_funcs():
    with open("src/vis.map") as f:
        for line in f:
            match = VIS_PATTERN.match(line)
            if match:
                yield match.group(1)


def main():
    symbols = set(get_symbols())
    error = False
    for func in get_vis_funcs():
        if func not in symbols:
            print(f"{func} not in library.")
            error = True

    return int(error)


if __name__ == "__main__":
    exit(main())
