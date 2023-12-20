# To run this file, run in lldb:
# command script import ~/AutoDraw/tools/lldb_script.py

# todo I want this: https://stackoverflow.com/questions/28510221/can-i-tell-lldb-to-remove-the-active-breakpoint

# todo https://stackoverflow.com/questions/23812021/how-to-define-a-stop-hook-for-lldb-in-lldbinit

import lldb

def __lldb_init_module(debugger, internal_dict):
    for i in range(debugger.GetNumTargets()):
        target = debugger.GetTargetAtIndex(i)
        target.DeleteAllBreakpoints()
    debugger.HandleCommand('command alias py command script import ~/AutoDraw/tools/lldb_script.py')
    debugger.HandleCommand('command source ~/AutoDraw/tools/lldb_breakpoints.txt')
    # debugger.HandleCommand(f'target stop-hook add --one-liner "target symbols add libgame.dylib.dSYM"')
    debugger.HandleCommand(f'type summary add --python-function lldb_script.print_v3 v3')
    debugger.HandleCommand(f'type summary add --python-function lldb_script.print_v2 v2')
    debugger.HandleCommand(f'env DYLD_INSERT_LIBRARIES=/usr/local/Cellar/llvm/17.0.6/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib')  # todo hard-coded

def print_v3(value, internal_dict, options):
   x = value.GetChildMemberWithName('x').GetValue()
   y = value.GetChildMemberWithName('y').GetValue()
   z = value.GetChildMemberWithName('z').GetValue()
   return f'[{x}, {y}, {z}]'

def print_v2(value, internal_dict, options):
   x = value.GetChildMemberWithName('x').GetValue()
   y = value.GetChildMemberWithName('y').GetValue()
   return f'[{x}, {y}]'

# target = lldb.debugger.GetSelectedTarget()
# target.GetProcess().GetSelectedThread().GetSelectedFrame().GetLineEntry().GetFileSpec()

# Commands you might forget:
# "py" to run my python script
# "thread backtrace" to examine the call stack
# "f" to view current line
# "diplay" to watch variable
# "jump" to skip over assertions
