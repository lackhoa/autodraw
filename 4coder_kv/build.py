#!/usr/bin/env python3

import os
import subprocess
from subprocess import PIPE, STDOUT
import sys
import time

def run(command, update_env={}):
    # print(command)
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
CUSTOM=f'{HOME}/4ed/code/custom'
AUTODRAW=f'{HOME}/AutoDraw'
SOURCE=f'{HERE}/4coder_kv.cpp'

try:
    os.chdir(f'{FCODER_ROOT}')
    print(f'Workdir: {os.getcwd()}')

    run_only       = (len(sys.argv) > 1 and sys.argv[1] == 'run')
    full_rebuild   = (len(sys.argv) > 1 and sys.argv[1] == 'full')  # hopefully never have to be used

    DEBUG_MODE = True
    ADDRESS_SANITIZER_ON = False

    INCLUDES=f'-I{HERE}/custom_patch -I{CUSTOM} -I{AUTODRAW}/libs -I{AUTODRAW}/4coder_kv/libs -I{AUTODRAW}/code'
    OPTIMIZATION='-O0' if DEBUG_MODE else '-O2'
    opts = f"-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g -DOS_MAC=1 -DOS_WINDOWS=0 -DOS_LINUX=0 {INCLUDES} {OPTIMIZATION}"
    arch = "-m64"
    debug="-g" if DEBUG_MODE else ""
    COPY_TO_STABLE = False

    if run_only:
        dyld_insert_libraries="DYLD_INSERT_LIBRARIES=/usr/local/Cellar/llvm/17.0.6/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib" if ADDRESS_SANITIZER_ON else ""
        command = f'{dyld_insert_libraries} {FCODER_ROOT}/4ed > /dev/null'
        # Does emacs output pipe slow down 4coder? Very possible!
        os.chdir(f"{AUTODRAW}")
        run(command) # Run in a new process group

    else:
        if False:  # Lexer generator
            print('Lexer: Generate (one-time thing)')
            OPTIMIZATION="-O0"
            run(f'clang++ {HERE}/4coder_kv_skm_lexer_gen.cpp {arch} {opts} {debug} -Wno-tautological-compare -std=c++11 {OPTIMIZATION} -o {HERE}/lexer_generator')
            #
            print('running lexer generator')
            run(f'mkdir -p {HERE}/generated && {HERE}/lexer_generator {HERE}/generated')
            run(f'rm {HERE}/lexer_generator & rm -rf {HERE}/lexer_generator.dSYM')

        sanitize_address = '-fsanitize=address' if ADDRESS_SANITIZER_ON else ''
        preproc_file="4coder_command_metadata.i"
        meta_macros="-DMETA_PASS"
        if True:
            print('preproc_file: Generate')
            run(f'clang++ -I{CUSTOM} {meta_macros} {arch} {opts} {debug} -std=c++11 "{SOURCE}" -E -o {preproc_file}')
            #
            print('Meta-generator: Compile & Link')
            run(f'ccache clang++ -c "{CUSTOM}/4coder_metadata_generator.cpp" -I"{CUSTOM}" {opts} -O2 -std=c++11 -o "{CUSTOM}/metadata_generator.o"')
            #
            run(f'clang++ -I"{CUSTOM}" "{CUSTOM}/metadata_generator.o" -o "{CUSTOM}/metadata_generator"')
            #
            print('Meta-generator: Run')
            run(f'"{CUSTOM}/metadata_generator" -R "{CUSTOM}" "{os.getcwd()}/{preproc_file}"')

        print('custom_4coder.so: Compile & Link')
        run(f'ccache clang++ -c "{SOURCE}" -I"{CUSTOM}" {arch} {opts} {debug} -std=c++11 -fPIC -o custom_4coder.o {sanitize_address}')
        #
        FRAMEWORKS="-framework Metal -framework Cocoa -framework QuartzCore"
        run(f'clang++ "custom_4coder.o" "{AUTODRAW}/build/autodraw.o" -shared -o "custom_4coder.so" {FRAMEWORKS} {sanitize_address}')
        if COPY_TO_STABLE:
            print(f'copy to stable')
            run(f'cp custom_4coder.so ~/4coder_stable/')

        print("NOTE: Setup 4coder config files")
        run(f'ln -sf "{HERE}/config.4coder" "{FCODER_ROOT}/config.4coder"')
        run(f'ln -sf "{HERE}/theme-kv.4coder" "{FCODER_ROOT}/themes/theme-kv.4coder"')

        run(f'rm -f "{CUSTOM}/metadata_generator.o" "{CUSTOM}/metadata_generator" {preproc_file}')

        print('Build complete!')

except Exception as e:
    print(f'Error: {e}')
