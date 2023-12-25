#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE
set -e

grep -r "TODO" ../code
grep -r "TODO(kv)" ../4coder_kv/

