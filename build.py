#! /usr/local/bin/python3
import os
import subprocess
from subprocess import PIPE, STDOUT
import sys
import time

def run(command):
    print(' '.join(command))
    process = subprocess.run(command, capture_output=True)
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

    run_only     = (len(sys.argv) > 1 and sys.argv[1] == 'run')
    full_rebuild = (len(sys.argv) > 1 and sys.argv[1] == 'full')

    # src_mtime = max(mtime("../code/osx-main.mm"), mtime("../code/platform.h"))
    # dst_mtime = mtime("autodraw.o")
    build_osx_main = full_rebuild

    if run_only:
        command = './autodraw'
        # run in a new process group
        process = subprocess.Popen(command, close_fds=True, preexec_fn=os.setsid)
    else:
        # Set compiler and linker flags
        optimization = ['-O0']
        constants = ['-DAUTO_MAC', '-DAUTO_INTERNAL=1', '-DAUTO_DIAGNOSTICS=1']
        warnings = ['-Wall', '-Wimplicit-int-float-conversion', '-Wno-unused-function', '-Wno-missing-braces', '-Wno-unused-parameter', '-Wno-unused-but-set-variable', '-Wno-unused-variable', '-Wno-switch', '-Wno-writable-strings', '-Wno-c++17-extensions', '-Wno-pointer-to-int-cast', '-Wno-tautological-constant-out-of-range-compare', '-Wno-reorder-init-list', '-Wno-macro-redefined', '-Wno-deprecated-declarations', '-Wno-unknown-attributes']
        common_compiler_flags = ["-g", "-mavx2", "-std=gnu++20"] + optimization + constants + warnings

        # Compile the game to a dll
        includes = ['-I../libs']
        run(['clang', '-dynamiclib', '../code/game.cpp', '-o', 'libgame.dylib'] + common_compiler_flags)

        # Compile the app (Apple headers is so slow to compile, we need to do it separately)
        if build_osx_main:
            run(['clang', '-c', '../code/osx-main.mm', '-o', 'autodraw.o'] + includes + common_compiler_flags)

        run(['xcrun', '-sdk', 'macosx', 'metal', '-c', '../code/shaders.metal', '-o', 'shaders.air'])
        run(['xcrun', '-sdk', 'macosx', 'metallib', 'shaders.air', '-o', 'shaders.metallib'])

        # Link
        linked_libs = ['-lstdc++']
        frameworks=['Metal', 'Cocoa', 'QuartzCore']
        framework_flags=[]
        for framework in frameworks:
            framework_flags += ["-framework", framework]
        if build_osx_main:
            run(['clang', 'autodraw.o', '-o', 'autodraw'] + linked_libs + framework_flags)
        print('Build complete!')

except Exception as e:
    print(f'Error: {e}')
