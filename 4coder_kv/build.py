#!/usr/bin/env python3

import os
import subprocess
from subprocess import PIPE, STDOUT
import sys
import time

def run(command, update_env={}):
    # print(' '.join(command))
    begin = time.time()
    env = os.environ.copy()
    env.update(update_env)
    process = subprocess.run(command, shell=True, capture_output=True, env=env)
    end = time.time()
    print(f'Time taken: {end - begin:.3f} seconds')
    if len(process.stdout):
        print(process.stdout.decode("utf-8"))
    if len(process.stderr):
        print(process.stderr.decode("utf-8"))
    if process.returncode != 0:
        exit(1)
    print()
    return process

def mtime(path):
    try:
        return os.path.getmtime(path)
    except:
        return 0

# cd to script directory
HOME=os.path.expanduser("~")
HERE = os.path.dirname(os.path.realpath(__file__))
FCODER_ROOT=f'{HOME}/4coder'
CODE_HOME=f'{HOME}/4ed/code/custom'
AUTODRAW_ROOT=f'{HOME}/AutoDraw'
SOURCE=f'{HERE}/4coder_kv.cpp'

try:
    os.chdir(f'{FCODER_ROOT}')
    print(f'Workdir: {os.getcwd()}')

    run_only       = (len(sys.argv) > 1 and sys.argv[1] == 'run')
    full_rebuild   = (len(sys.argv) > 1 and sys.argv[1] == 'full')  # hopefully never have to be used

    DEBUG_MODE = False
    ADDRESS_SANITIZER_ON = False

    if run_only:
        dyld_insert_libraries="DYLD_INSERT_LIBRARIES=/usr/local/Cellar/llvm/17.0.6/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib" if ADDRESS_SANITIZER_ON else ""
        command = f'{dyld_insert_libraries} {FCODER_ROOT}/4ed > /dev/null'
        # Does emacs output pipe slow down 4coder? Very possible!
        os.chdir(f"{AUTODRAW_ROOT}")
        run(command) # Run in a new process group
    else:
        sanitize_address = '-fsanitize=address' if ADDRESS_SANITIZER_ON else ''
        #
        opts=f"-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g -DOS_MAC=1 -DOS_WINDOWS=0 -DOS_LINUX=0 -I{HERE}/../libs"
        arch="-m64"
        debug="-g"

        preproc_file="4coder_command_metadata.i"
        meta_macros="-DMETA_PASS"
        #
        print('preproc_file: Generate')
        run(f'clang++ -I{CODE_HOME} {meta_macros} {arch} {opts} {debug} -std=c++11 "{SOURCE}" -E -o {preproc_file}')
        #
        print('Meta-generator: Compile & Link')
        run(f'ccache clang++ -c "{CODE_HOME}/4coder_metadata_generator.cpp" -I"{CODE_HOME}" {opts} -O2 -std=c++11 -o "{CODE_HOME}/metadata_generator.o"')
        #
        run(f'clang++ -I"{CODE_HOME}" "{CODE_HOME}/metadata_generator.o" -o "{CODE_HOME}/metadata_generator"')
        #
        print('Meta-generator: Run')
        run(f'"{CODE_HOME}/metadata_generator" -R "{CODE_HOME}" "{os.getcwd()}/{preproc_file}"')
        #
        print('custom_4coder.so: Compile & Link')
        run(f'ccache clang++ -c "{SOURCE}" -I"{CODE_HOME}" {arch} {opts} {debug} -std=c++11 -fPIC -o custom_4coder.o {sanitize_address}')
        #
        FRAMEWORKS="-framework Metal -framework Cocoa -framework QuartzCore"
        run(f'clang++ "custom_4coder.o" "{AUTODRAW_ROOT}/build/autodraw.o" -shared -o "custom_4coder.so" {FRAMEWORKS} {sanitize_address}')

        run(f'rm -f "{CODE_HOME}/metadata_generator.o" "{CODE_HOME}/metadata_generator" {preproc_file}')
        print("NOTE: Setup 4coder config files")
        run(f'ln -sf "{HERE}/config.4coder" "{FCODER_ROOT}/config.4coder"')
        run(f'ln -sf "{HERE}/theme-kv.4coder" "{FCODER_ROOT}/themes/theme-kv.4coder"')

        print('Build complete!')

except Exception as e:
    print(f'Error: {e}')
