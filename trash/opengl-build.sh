#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE

mkdir build 2> /dev/null
pushd build

# Optimization="-Ofast -march=native"
Optimization=-O0
Constants="-DReaMac -DREA_INTERNAL=1 -DREA_DIAGNOSTICS=1"
Warnings="-Wall -Wunused-parameter -Wimplicit-int-float-conversion -Wno-unused-function -Wno-missing-braces -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-switch -Wno-writable-strings -Wno-c++17-extensions -Wno-pointer-to-int-cast -Wno-tautological-constant-out-of-range-compare -Wno-reorder-init-list -Wno-macro-redefined -Wno-deprecated-declarations"
CommonCompilerFlags="-g -mavx2 -std=gnu++20 $Optimization $Constants $Warnings -I '../libs' -I /usr/local/lib/raylib-4.5.0-debug"

# Compile
clang -c "../code/osx_main.mm" -o "autodraw.o" $CommonCompilerFlags || exit 1

# Link
LinkedLibs="/usr/local/lib/raylib-4.5.0-debug/libraylib.a"
Frameworks="-framework GLUT -framework OpenGL -framework IOKit -framework CoreVideo -framework cocoa"
clang "autodraw.o" -o "autodraw" -lstdc++ $CommonCompilerFlags $LinkedLibs $Frameworks || exit 1

popd

echo build complete!
