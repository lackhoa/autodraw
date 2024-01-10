#pragma once

#include "kv.h"
#include "4coder_default_include.cpp"

#define GET_VIEW_AND_BUFFER \
  View_ID   view = get_active_view(app, Access_ReadVisible); \
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible)

inline void kv_print_message(Application_Links *app, char *message) 
{
  print_message(app, SCu8(message));
}

inline void
kv_buffer_replace_range(Application_Links *app, Buffer_ID buffer, i64 min, i64 max, String_Const_u8 replacement)
{
  buffer_replace_range(app, buffer, Ii64(min, max), replacement);
}

inline void kv_buffer_delete_range(Application_Links *app, Buffer_ID buffer, i64 min, i64 max)
{
  buffer_replace_range(app, buffer, Ii64(min, max), string_u8_empty);
}

inline void kv_buffer_delete_pos(Application_Links *app, Buffer_ID buffer, i64 min)
{
  buffer_replace_range(app, buffer, Ii64(min, min+1), string_u8_empty);
}

inline String_Const_u8
push_buffer_dir_name(Application_Links *app, Arena *arena, Buffer_ID buffer)
{
  String_Const_u8 filename = push_buffer_file_name(app, arena, buffer);
  return string_remove_last_folder(filename);
}
