#!/bin/bash

for var in "$@"; do
    clang -Xclang -disable-O0-optnone -O0 -emit-llvm -c "./$var.c" -o "./$var.bc"
    opt -mem2reg "./$var.bc" -o "./$var-m2r.bc" 
    llc -relocation-model=pic -filetype=obj "./$var-m2r.bc"
    gcc "$var-m2r.o" -o "$var-m2r.out"
    valgrind --tool=memcheck --leak-check=yes --show-reachable=yes "./$var-m2r.out"
done