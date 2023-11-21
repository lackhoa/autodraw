#! /usr/local/bin/python3
import os
import subprocess
from subprocess import PIPE, STDOUT
import sys

def run(command):
    process = subprocess.run(command, capture_output=True)
    if len(process.stdout):
        print(process.stdout.decode("utf-8"))
    if len(process.stderr):
        print(process.stderr.decode("utf-8"))
    if process.returncode != 0:
        exit(1)
    return process

# cd to script directory
script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(script_dir)

try:
    os.makedirs('build', exist_ok=True)
    os.chdir('build')

    run_only = (len(sys.argv) > 1 and sys.argv[1] == 'run')
    if not run_only:
        # Set compiler and linker flags
        optimization = ['-O0']
        constants = ['-DAUTO_MAC', '-DAUTO_INTERNAL=1', '-DAUTO_DIAGNOSTICS=1']
        warnings = ['-Wall', '-Wimplicit-int-float-conversion', '-Wno-unused-function', '-Wno-missing-braces', '-Wno-unused-parameter', '-Wno-unused-but-set-variable', '-Wno-unused-variable', '-Wno-switch', '-Wno-writable-strings', '-Wno-c++17-extensions', '-Wno-pointer-to-int-cast', '-Wno-tautological-constant-out-of-range-compare', '-Wno-reorder-init-list', '-Wno-macro-redefined', '-Wno-deprecated-declarations']
        common_compiler_flags = ["-g", "-mavx2", "-std=gnu++20"] + optimization + constants + warnings

        # Compile
        includes = ['-I../libs']
        run(['clang', '-c', '../code/osx-main.mm', '-o', 'autodraw.o'] + includes + common_compiler_flags)

        run(['xcrun', '-sdk', 'macosx', 'metal', '-c', '../code/shaders.metal', '-o', 'shaders.air'])
        run(['xcrun', '-sdk', 'macosx', 'metallib', 'shaders.air', '-o', 'shaders.metallib'])

        # Link
        linked_libs = []
        frameworks=['Metal', 'Cocoa', 'QuartzCore']
        framework_flags=[]
        for framework in frameworks:
            framework_flags += ["-framework", framework]
        run(['clang', 'autodraw.o', '-o', 'autodraw', '-lstdc++'] + linked_libs + framework_flags)
        print('Build complete!')

    run(['./autodraw'])

except Exception as e:
    print(f'Error: {e}')
