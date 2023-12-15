#!/usr/bin/env python3

import os
import subprocess
from subprocess import PIPE, STDOUT
import sys
import time

def run(command):
    print()
    print(' '.join(command))
    begin = time.time()
    process = subprocess.run(command, capture_output=True)
    end = time.time()
    print(f'Time taken: {end - begin:.3f} seconds')
    if len(process.stdout):
        print(process.stdout.decode("utf-8"))
    if len(process.stderr):
        print(process.stderr.decode("utf-8"))
    if process.returncode != 0:
        exit(1)
    return process

def mtime(path):
    try:
        return os.path.getmtime(path)
    except:
        return 0

# cd to script directory
script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(script_dir)

try:
    os.makedirs('build', exist_ok=True)
    os.chdir('build')

    run_only       = (len(sys.argv) > 1 and sys.argv[1] == 'run')
    full_rebuild   = (len(sys.argv) > 1 and sys.argv[1] == 'full')  # hopefully never have to be used

    if run_only:
        command = './autodraw'
        # process = subprocess.Popen(command, close_fds=True, preexec_fn=os.setsid) # Run in a new process group
        run(command) # Run in a new process group
    else:
        # Set compiler and linker flags
        DEBUG_MODE = False

        optimization_flag = '-O0' if DEBUG_MODE else '-O2'
        constants = ['-DAUTO_MAC', '-DAUTO_INTERNAL=1', '-DAUTO_DIAGNOSTICS=1']
        warnings = ['-Werror', '-Wall', '-Wimplicit-int-float-conversion', '-Wno-unused-function', '-Wno-missing-braces', '-Wno-unused-parameter', '-Wno-unused-but-set-variable', '-Wno-unused-variable', '-Wno-switch', '-Wno-writable-strings', '-Wno-c++17-extensions', '-Wno-pointer-to-int-cast', '-Wno-tautological-constant-out-of-range-compare', '-Wno-reorder-init-list', '-Wno-macro-redefined', '-Wno-deprecated-declarations', '-Wno-unknown-attributes']
        includes = ['-I../libs']
        sanitizer = ['-fsanitize=address'] if DEBUG_MODE else []
        common_compiler_flags = ['-g', '-mavx2', '-std=gnu++20', '-fno-exceptions', '-fvisibility=hidden', optimization_flag] + includes + constants + warnings

        todo_llvm_path = "/usr/local/Cellar/llvm/17.0.6" # llvm-config --libdir
        print("NOTE: codegen: compile")
        run(['ccache', 'clang++', '-c', '../code/ad_codegen.cpp', '-o', 'generator.o', f'-I{todo_llvm_path}/include'] + common_compiler_flags + sanitizer)
        #
        print("NOTE: codegen: link")
        run(['clang++', 'generator.o', '-o', 'generator', f'-L{todo_llvm_path}/lib', f'-I{todo_llvm_path}/include', '-lclang'] + sanitizer)
        #
        print("NOTE: codegen: run")
        current_directory = os.getcwd()
        run(["./generator"])

        print('NOTE: Compile + Link the game to produce a dylib')
        run(['clang++', '-dynamiclib', '../code/game.cpp', '-olibgame.dylib'] +
            common_compiler_flags + sanitizer)

        print('NOTE: Compile osx app')
        run(['ccache', 'clang++', '-c', '../code/osx-main.mm', '-o', 'autodraw.o'] + includes + common_compiler_flags)

        print("NOTE: Link osx app")
        linked_libs = []
        frameworks=['Metal', 'Cocoa', 'QuartzCore']
        framework_flags=[]
        for framework in frameworks:
            framework_flags += ["-framework", framework]
        run(['clang++', 'autodraw.o', '-o', 'autodraw'] + linked_libs + framework_flags)

        print('NOTE: Compile shaders')
        run(['xcrun', '-sdk', 'macosx', 'metal', '-c', '../code/shaders.metal', '-o', 'shaders.air'])
        run(['xcrun', '-sdk', 'macosx', 'metallib', 'shaders.air', '-o', 'shaders.metallib'])

        print('Build complete!')

except Exception as e:
    print(f'Error: {e}')
