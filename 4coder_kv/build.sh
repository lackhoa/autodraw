#!/bin/bash

# NOTE: Script originally taken from 4ed/code/custom/bin/kv_buildsuper_x64-mac.sh (v4.1.7)

set -e
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
FCODER_ROOT="${HOME}/4ed/build"
CODE_HOME="$(dirname "${FCODER_ROOT}/custom/bin")"

echo "Workdir: ${FCODER_ROOT}"
cd "${FCODER_ROOT}"

SOURCE="${HERE}/4coder_kv.cpp"

# NOTE(yuval): Removed -Wno-writable-strings as it is the same as -Wno-write-strings
opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g -DOS_MAC=1 -DOS_WINDOWS=0 -DOS_LINUX=0"
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
ccache clang++ -c "$SOURCE" -I"$CODE_HOME" $arch $opts $debug -std=c++11 -fPIC -o custom_4coder.o -fsanitize=address
#
echo "NOTE: LINKING"
AUTODRAW_STATIC_LIB_PATH="${HOME}/AutoDraw/build"
FRAMEWORKS="-framework Metal -framework Cocoa -framework QuartzCore"
clang++ custom_4coder.o "${AUTODRAW_STATIC_LIB_PATH}/autodraw.o" -shared -o "custom_4coder.so" ${FRAMEWORKS} -fsanitize=address

rm -f "$CODE_HOME/metadata_generator"
rm -f $preproc_file
