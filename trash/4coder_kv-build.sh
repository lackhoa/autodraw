#!/bin/bash

# NOTE: Script originally taken from 4ed/code/custom/bin/kv_buildsuper_x64-mac.sh (v4.1.7)
#
# todo: Compilation is slow (~5 seconds), but the slowness isn't from my side,
# since the game also links with ~10k lines library, and it only takes ~2
# seconds.

set -e
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
FCODER_ROOT="$HOME/4coder"
CODE_HOME="$HOME/4ed/code/custom"
AUTODRAW_ROOT="$HOME/AutoDraw"

cd "${FCODER_ROOT}"
echo "Workdir: $(pwd)"

SOURCE="${HERE}/4coder_kv.cpp"

if [ "$1" = "run" ]; then
    export DYLD_INSERT_LIBRARIES="/usr/local/Cellar/llvm/17.0.6/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib"
    ${FCODER_ROOT}/4ed > /dev/null  # Does emacs output pipe slow down 4coder? Very possible!
else
    opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g -DOS_MAC=1 -DOS_WINDOWS=0 -DOS_LINUX=0 -I${HERE}/../libs"
    arch=-m64

    debug=-g

    preproc_file=4coder_command_metadata.i
    meta_macros="-DMETA_PASS"
    #
    echo "Meta-generator: Preproc"
    clang++ -I"$CODE_HOME" $meta_macros $arch $opts $debug -std=c++11 "$SOURCE" -E -o $preproc_file
    #
    echo "Meta-generator: Compile & Link"
    clang++ -I"$CODE_HOME" $opts $debug -std=c++11 "$CODE_HOME/4coder_metadata_generator.cpp" -o "$CODE_HOME/metadata_generator"
    #
    echo "Meta-generator: Run"
    "$CODE_HOME/metadata_generator" -R "$CODE_HOME" "$PWD/$preproc_file"
    #
    echo "NOTE: COMPILING"
    ccache clang++ -c "$SOURCE" -I"$CODE_HOME" $arch $opts $debug -std=c++11 -fPIC -o custom_4coder.o
    #
    echo "NOTE: LINKING"
    FRAMEWORKS="-framework Metal -framework Cocoa -framework QuartzCore"
    clang++ "custom_4coder.o" "$AUTODRAW_ROOT/build/autodraw.o" -shared -o "custom_4coder.so" ${FRAMEWORKS}

    rm -f "$CODE_HOME/metadata_generator"
    rm -f $preproc_file

    echo "NOTE: Setup 4coder config files"
    ln -sf "${HERE}/config.4coder" "${FCODER_ROOT}/config.4coder"
fi
