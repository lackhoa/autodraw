#!/bin/bash
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $HERE
PROJECT_ROOT="$(realpath ..)"
FCODER_ROOT="$HOME/4ed"

function g {
    SEARCH="${1}"
    LOCATION="${2}"
    if [ -z "${SEARCH}" ]; then
        echo "typo"
        exit 1
    fi

    if [ -z "${LOCATION}" ]; then
        LOCATION="$(pwd)"
    fi

    grep -rI --color=auto --exclude-dir=".git" "${SEARCH}" "${LOCATION}"
}

g "TODO" ${PROJECT_ROOT}/code
g "nono" ${PROJECT_ROOT}/code

g "TODO(kv)" ${PROJECT_ROOT}/4coder_kv/
g "nono"     ${PROJECT_ROOT}/4coder_kv/

g "TODO(kv)" ${FCODER_ROOT}/code
g "nono"     ${FCODER_ROOT}/code
