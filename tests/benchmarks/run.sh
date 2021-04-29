#!/bin/bash

for var in "$@"; do
    llc -relocation-model=pic -filetype=obj "./$var.bc"
    gcc "$var.o" -o "$var.out"
    valgrind --tool=memcheck --leak-check=yes --show-reachable=yes "./$var.out"
done