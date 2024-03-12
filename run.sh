#!/usr/bin/env bash

mkdir -p build && cd build

if [ "$name" == "nix-shell" ]; then
    cmake .. && make && ./mblock
else
    nix-shell ../shell.nix --run "cmake .. && make && ./mblock"
fi

cd ..