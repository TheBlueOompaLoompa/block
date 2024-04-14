#!/usr/bin/env bash

mkdir -p build && cd build

if [ "$name" == "nix-shell" ]; then
    cmake .. && make; $1
else
    nix-shell ../shell.nix --run "cmake .. && make -j$(nproc); $1"
fi

cd ..