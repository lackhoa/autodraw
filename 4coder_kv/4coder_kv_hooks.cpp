#include "4coder_kv_core.cpp"

BUFFER_HOOK_SIG(kv_file_save)
{
  default_file_save(app, buffer_id);
  vim_file_save(app, buffer_id);
  return 0;
}

BUFFER_HOOK_SIG(kv_new_file)
{
	Scratch_Block scratch(app);
	String_Const_u8 file_name = push_buffer_base_name(app, scratch, buffer_id);
	if(string_match(string_postfix(file_name, 4), string_u8_litexpr(".bat"))){
		Buffer_Insertion insert = begin_buffer_insertion_at_buffered(app, buffer_id, 0, scratch, KB(16));
		insertf(&insert, "@echo off" "\n");
		end_buffer_insertion(&insert);
		return 0;
	}

    // todo bash shell here

	return 0;
}

function Tick_Function kv_tick;
function void kv_tick(Application_Links *app, Frame_Info frame_info)
{
  default_tick(app, frame_info);

  // NOTE(kv): vim
  vim_animate_filebar(app, frame_info);
  vim_animate_cursor(app, frame_info);
  vim_cursor_blink++;

  // NOTE(kv): autosave
  f32 AUTOSAVE_PERIOD_SECONDS = 5.0f;
  seconds_since_last_keystroke += frame_info.literal_dt;
  if (seconds_since_last_keystroke > AUTOSAVE_PERIOD_SECONDS)
  {
    seconds_since_last_keystroke = 0;
    b32 saved_at_least_one_buffer = false;
    {
      ProfileScope(app, "save all dirty buffers");
      Scratch_Block scratch(app);
      for (Buffer_ID buffer = get_buffer_next(app, 0, Access_ReadWriteVisible);
           buffer != 0;
           buffer = get_buffer_next(app, buffer, Access_ReadWriteVisible))
      {
        if (buffer_get_dirty_state(app, buffer) == DirtyState_UnsavedChanges)
        {
          saved_at_least_one_buffer = true;
          Temp_Memory temp = begin_temp(scratch);
          String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer);
          buffer_save(app, buffer, file_name, 0);
          end_temp(temp);
        }
      }
    }
    if (saved_at_least_one_buffer) {
      print_message_cstr(app, "auto-saved all dirty buffers\n");
    }
  }
}
