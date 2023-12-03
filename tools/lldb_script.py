# To run this file, run in lldb:
# command script import ~/AutoDraw/tools/lldb_script.py

# todo I want this: https://stackoverflow.com/questions/28510221/can-i-tell-lldb-to-remove-the-active-breakpoint

import lldb

def __lldb_init_module(debugger, internal_dict):
    for i in range(debugger.GetNumTargets()):
        target = debugger.GetTargetAtIndex(i)
        target.DeleteAllBreakpoints()
    debugger.HandleCommand('command source ../tools/lldb-commands.txt')

# target = lldb.debugger.GetSelectedTarget()
# target.GetProcess().GetSelectedThread().GetSelectedFrame().GetLineEntry().GetFileSpec()

# NOTES
# run "thread backtrace" to examine the call stack, which is bizarre
