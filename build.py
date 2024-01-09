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
    process = subprocess.run(command, capture_output=True, env=env)
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

FCODER_PATH = '/Users/khoa/4ed/build'  # todo hardcoded

# cd to script directory
script_dir   = os.path.dirname(os.path.realpath(__file__))
project_root = "/Users/khoa/AutoDraw"  # todo hard-coded

try:
    os.makedirs('build', exist_ok=True)
    os.chdir(f'{project_root}/build')

    run_only       = (len(sys.argv) > 1 and sys.argv[1] == 'run')
    full_rebuild   = (len(sys.argv) > 1 and sys.argv[1] == 'full')  # hopefully never have to be used

    if run_only:
        command = './autodraw'
        # process = subprocess.Popen(command, close_fds=True, preexec_fn=os.setsid) # Run in a new process group
        run(command) # Run in a new process group
    else:
        # Set compiler and linker flags
        DEBUG_MODE = False
        SANITIZE_ADDRESS_ON = False
        #
        optimization_flag = '-O0' if DEBUG_MODE else '-O2'
        debug_flags = ['-g'] if DEBUG_MODE else []
        constants = ['-DAUTO_MAC', '-DAUTO_INTERNAL=1', '-DAUTO_DIAGNOSTICS=1']
        includes = ['-I../libs']
        sanitize_address = ['-fsanitize=address'] if SANITIZE_ADDRESS_ON else []
        #
        warnings = ['-Werror', '-Wall', '-Wextra', '-Wimplicit-int-float-conversion', '-Wno-unused-function',
                    '-Wno-missing-braces', '-Wno-unused-parameter', '-Wno-unused-but-set-variable',
                    '-Wno-unused-variable', '-Wno-switch', '-Wno-writable-strings',
                    '-Wno-c++17-extensions', '-Wno-pointer-to-int-cast',
                    '-Wno-tautological-constant-out-of-range-compare', '-Wno-reorder-init-list',
                    '-Wno-macro-redefined', '-Wno-deprecated-declarations', '-Wno-unknown-attributes']

        # todo leave out "-g" in release mode
        common_compile_flags = ['-g', '-std=gnu++20', '-fno-exceptions', '-fvisibility=hidden', optimization_flag] + includes + constants + warnings + debug_flags

        todo_llvm_path = "/usr/local/Cellar/llvm/17.0.6" # llvm-config --libdir
        print("codegen: compile & link")
        run(['ccache', 'clang++', '-c', f'{project_root}/code/ad_codegen.cpp', '-o', 'generator.o', '-O2', f'-I{todo_llvm_path}/include'] +
            common_compile_flags +
            sanitize_address)
        #
        run(['clang++', 'generator.o', '-o', 'generator', f'-L{todo_llvm_path}/lib', f'-I{todo_llvm_path}/include', '-lclang'] +
            sanitize_address)
        #
        print("codegen: run")
        current_directory = os.getcwd()
        run(["./generator"])

        print('libgame.dylib: Compile + Link')
        run(['clang++', '-dynamiclib', f'{project_root}/code/game.cpp', '-olibgame.dylib'] +
            common_compile_flags +
            sanitize_address)

        print('autodraw: compile & link')
        #
        run(['ccache', 'clang++', '-c', f'{project_root}/code/osx-main.mm', '-o', 'autodraw.o'] +
            includes +
            common_compile_flags +
            sanitize_address)

        #
        frameworks=['Metal', 'Cocoa', 'QuartzCore']
        framework_flags=[]
        for framework in frameworks:
            framework_flags += ["-framework", framework]
        #
        run(['clang++', 'autodraw.o', '-o', 'autodraw'] + framework_flags + sanitize_address)

        print("NOTE: Link library with 4coder")
        # todo: Hard coded build.sh path
        run([f'{project_root}/4coder_kv/build.py'])

        print('NOTE: Compile shaders')
        run(['xcrun', '-sdk', 'macosx', 'metal', '-c', '../code/shaders.metal', '-o', 'shaders.air'])
        run(['xcrun', '-sdk', 'macosx', 'metallib', 'shaders.air', '-o', 'shaders.metallib'])

        print('Build complete!')

except Exception as e:
    print(f'Error: {e}')