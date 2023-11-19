#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE

mkdir build 2> /dev/null
pushd build

# Optimization="-Ofast -march=native"
Optimization=-O0
Constants="-DReaMac -DREA_INTERNAL=1 -DREA_DIAGNOSTICS=1"
Warnings="-Wall -Wunused-parameter -Wimplicit-int-float-conversion -Wno-unused-function -Wno-missing-braces -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-switch -Wno-writable-strings -Wno-c++17-extensions -Wno-pointer-to-int-cast -Wno-tautological-constant-out-of-range-compare -Wno-reorder-init-list -Wno-macro-redefined -Wno-deprecated-declarations"
Includes="-I '../libs'"
CommonCompilerFlags="-g -mavx2 -std=gnu++20 $Optimization $Constants $Warnings $Includes"

# Compile
clang -c "../code/osx-main.mm" -o "autodraw.o" $CommonCompilerFlags $Includes || exit 1

# Link
LinkedLibs=""
Frameworks="-framework Metal -framework MetalKit -framework cocoa"
clang "autodraw.o" -o "autodraw" -lstdc++ $LinkedLibs $Frameworks || exit 1

popd

echo build complete!
