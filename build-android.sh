#!/bin/bash
# QuickCache Android Build Script
export CC=aarch64-linux-android-clang
export CXX=aarch64-linux-android-clang++
make clean
make
echo "Built for Android ARM64"
