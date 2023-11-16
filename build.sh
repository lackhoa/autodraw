#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE

mkdir build 2> /dev/null
pushd build

# Optimization="-Ofast -march=native"
Optimization=-O0
Constants="-DReaMac -DREA_INTERNAL=1 -DREA_DIAGNOSTICS=1"
Warnings="-Wall -Wunused-parameter -Wimplicit-int-float-conversion -Wno-unused-function -Wno-missing-braces -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-switch -Wno-writable-strings -Wno-c++17-extensions -Wno-pointer-to-int-cast -Wno-tautological-constant-out-of-range-compare -Wno-reorder-init-list -Wno-macro-redefined -Wno-deprecated-declarations"
CommonCompilerFlags="-g -mavx2 -std=gnu++20 $Optimization $Constants $Warnings -I '../libs'"

# Compile
clang -c "../code/osx_main.mm" -o "autodraw.o" $CommonCompilerFlags || exit 1

# Make default Metal Lib
xcrun -sdk macosx metal -o "add.ir" -c "../code/add.metal" || exit 1
xcrun -sdk macosx metallib -o "default.metallib" "add.ir" || exit 1

# Link
LinkedLibs="-framework cocoa -framework Metal -framework MetalKit -framework QuartzCore -framework Foundation"
clang "autodraw.o" -o "autodraw" -lstdc++ $CommonCompilerFlags $LinkedLibs || exit 1

popd

echo build complete!
