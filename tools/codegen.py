#! /usr/local/bin/python3

for key,action in [('H', "Left"), ("L", "Right")]:
    print(f"case kVK_ANSI_{key}: {{")
    print(f"action_state[GameActionMove{action}].is_down = is_down;")
    print("break;")
    print("}")
    print()
