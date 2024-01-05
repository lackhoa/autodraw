#pragma once

#include "4coder_vim/4coder_vim_include.h"
#include "4coder_byp_colors.cpp"
#include "kv.h"

#define GET_VIEW_AND_BUFFER \
  View_ID   view = get_active_view(app, Access_ReadVisible); \
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible)

inline void kv_print_message(Application_Links *app, char *message) {
  print_message(app, SCu8(message));
}
