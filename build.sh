#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE
set -e

mkdir -p build 2> /dev/null
cd build

# Optimization="-Ofast -march=native"
Optimization=-O0
Constants="-DReaMac -DREA_INTERNAL=1 -DREA_DIAGNOSTICS=1"
Warnings="-Wall -Wunused-parameter -Wimplicit-int-float-conversion -Wno-unused-function -Wno-missing-braces -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-switch -Wno-writable-strings -Wno-c++17-extensions -Wno-pointer-to-int-cast -Wno-tautological-constant-out-of-range-compare -Wno-reorder-init-list -Wno-macro-redefined -Wno-deprecated-declarations"
Includes="-I '../libs'"
CommonCompilerFlags="-g -mavx2 -std=gnu++20 $Optimization $Constants $Warnings $Includes"

# Turn on Metal API Validation. Great for catching API misuse, remove for Release build
METAL_DEVICE_WRAPPER_TYPE=1

# Compile
clang -c "../code/osx-main.mm" -o "autodraw.o" $CommonCompilerFlags $Includes

# Metal
xcrun -sdk macosx metal -c ../code/shaders.metal -o shaders.air
xcrun -sdk macosx metallib shaders.air -o shaders.metallib

# Link
LinkedLibs=""
Frameworks="-framework Metal -framework Cocoa -framework QuartzCore"
clang "autodraw.o" -o "autodraw" -lstdc++ $LinkedLibs $Frameworks

echo build complete!

./autodraw
