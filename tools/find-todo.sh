#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE
PROJECT_ROOT="$(realpath ..)"

grep -r "TODO" $PROJECT_ROOT/code
grep -r "nocheckin" $PROJECT_ROOT/code
grep -r "nono" $PROJECT_ROOT/code
grep -r "TODO(kv)" $PROJECT_ROOT/4coder_kv/
grep -r "nocheckin" $PROJECT_ROOT/4coder_kv/
grep -r "nono" $PROJECT_ROOT/4coder_kv/
