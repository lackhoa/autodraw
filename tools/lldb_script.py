# To run this file, run in lldb:
# command script import ~/AutoDraw/tools/lldb_script.py
# or just type "py" if you have that alias setup

# todo I want this: https://stackoverflow.com/questions/28510221/can-i-tell-lldb-to-remove-the-active-breakpoint

import lldb

def __lldb_init_module(debugger, internal_dict):
    for i in range(debugger.GetNumTargets()):
        target = debugger.GetTargetAtIndex(i)
        target.DeleteAllBreakpoints()
    debugger.HandleCommand('command source ../tools/lldb-commands.txt')
    debugger.HandleCommand('command script import ../tools/lldb_formatters.py')
    debugger.HandleCommand(f'type summary add --python-function lldb_script.print_v3 v3')
    debugger.HandleCommand(f'type summary add --python-function lldb_script.print_v2 v2')

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

# NOTES
# run "thread backtrace" to examine the call stack
# type "f" to view current line
