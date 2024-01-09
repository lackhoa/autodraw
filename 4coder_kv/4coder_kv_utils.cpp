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

#define HISTORY_MERGE_SCOPE \
  History_Record_Index HISTORY_MERGE_INDEX = buffer_history_get_current_state_index(app, buffer); \
  defer(auto current_index = buffer_history_get_current_state_index(app, buffer); buffer_history_merge_record_range(app, buffer, HISTORY_MERGE_INDEX+1, current_index, RecordMergeFlag_StateInRange_MoveStateForward); );
