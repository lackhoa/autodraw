#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE

grep -r "TODO" ../code
grep -r "nocheckin" ../code
grep -r "nono" ../code
grep -r "TODO(kv)" ../4coder_kv/
grep -r "nocheckin" ../4coder_kv/
grep -r "nono" ../4coder_kv/
