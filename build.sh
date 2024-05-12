#!/usr/bin/env bash

mkdir -p build && cd build

if [ "$name" == "nix-shell" ]; then
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && make; $1
else
    nix-shell ../shell.nix --run "cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. && make -j$(nproc); $1"
fi

cd ..
