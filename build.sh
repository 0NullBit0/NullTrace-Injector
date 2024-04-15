#!/bin/bash

REM build script for unix by nullbit

ABIS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")
BUILD_DIR="tmp_wbuild"

for ABI in "${ABIS[@]}"; do
    echo "Building for ABI: $ABI"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    mkdir "$BUILD_DIR"
    cmake -S . -B "BUILD_DIR" -G "Ninja" -DANDROID_ABI="$ABI"
    cmake --build "$BUILD_DIR"
done

read -n 1 -s -r -p "Press any key to continue..."
echo
