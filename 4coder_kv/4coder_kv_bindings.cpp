#include "4coder_byp_hooks.cpp"
#include "4coder_byp_build.cpp"

function void
byp_essential_mapping(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
	MappingScope();
	SelectMapping(mapping);

	SelectMap(global_id);
	BindCore(byp_startup, CoreCode_Startup);
	BindCore(vim_try_exit, CoreCode_TryExit);
	BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
	BindMouseWheel(mouse_wheel_scroll);
	BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
	//BindCore(vim_file_externally_modified, CoreCode_FileExternallyModified);

	SelectMap(file_id);
	ParentMap(global_id);
	BindTextInput(byp_write_text_input);
	BindMouse(click_set_cursor_and_mark, MouseCode_Left);
	BindMouseRelease(click_set_cursor, MouseCode_Left);
	BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
	BindMouseMove(click_set_cursor_if_lbutton);

	SelectMap(code_id);
	ParentMap(file_id);
}

// NOTE(kv): The VimBind function doesn't let us overwrite bindings.
// So I hoisted also "vim_default_bindings" out here.
function void
kv_vim_bindings(Application_Links *app)
{
#define N bit_1
#define I bit_2
#define V bit_3
#define MAP 0

	u32 Ctl  = KeyMod_Ctl;
	u32 Sft  = KeyMod_Sft;
    u32 Meta = OS_MAC ? KeyMod_Cmd : KeyMod_Alt;
    Key_Code leader = KeyCode_BackwardSlash;

	VimBind(MAP, vim_normal_mode,                          KeyCode_Escape);
	VimBind(MAP, vim_inc_buffer_peek,                 (Ctl|KeyCode_RightBracket));
	VimBind(MAP, vim_dec_buffer_peek,                 (Ctl|KeyCode_LeftBracket));
	VimBind(MAP, vim_toggle_show_buffer_peek,         (Ctl|KeyCode_Return));
	VimBind(MAP, vim_toggle_show_buffer_peek,     (Ctl|Sft|KeyCode_Return));
	VimBind(MAP, vim_scoll_buffer_peek_up,        (Ctl|Sft|KeyCode_U));
	VimBind(MAP, vim_scoll_buffer_peek_down,      (Ctl|Sft|KeyCode_D));

	/// Rebinds
	VimBind(N|MAP, undo,                                   KeyCode_U);
	VimBind(N|MAP, undo,                              (Ctl|KeyCode_Z));
	VimBind(N|MAP, redo,                              (Ctl|KeyCode_R));
	VimBind(N|MAP, save,                              (Ctl|KeyCode_S));
	VimBind(N|MAP, vim_open_file_in_quotes,         SUB_G, KeyCode_F);
	VimBind(N|MAP, vim_interactive_open_or_new,   (Ctl|Sft|KeyCode_O));
	VimBind(N|MAP, quick_swap_buffer,             (Sft|Ctl|KeyCode_6));
	VimBind(N|MAP, vim_goto_definition,               (Ctl|KeyCode_W));
	VimBind(N|MAP, vim_next_4coder_jump,              (Meta|KeyCode_N));
	VimBind(N|MAP, vim_prev_4coder_jump,          (Sft|Meta|KeyCode_N));
	VimBind(N|MAP, vim_first_4coder_jump,         (Sft|Meta|KeyCode_M));
	VimBind(N|I|MAP, view_buffer_other_panel,         (Ctl|KeyCode_1));
	VimBind(N|I|MAP, swap_panels,                     (Ctl|KeyCode_2));
	VimBind(I|MAP, word_complete_drop_down,           (Ctl|KeyCode_N));
	//VimBind(I|MAP, word_complete_drop_down,           (Ctl|KeyCode_P));

	/// Mode Binds
	VimBind(N|V|MAP, vim_modal_i,                          KeyCode_I);
	VimBind(N|V|MAP, vim_command_mode,                (Sft|KeyCode_Semicolon));
	VimBind(N|MAP,   vim_insert_begin,                (Sft|KeyCode_I));
	VimBind(N|MAP,   vim_replace_mode,                (Sft|KeyCode_R));
	VimBind(N|V|MAP, vim_visual_mode,                      KeyCode_V);
	VimBind(N|V|MAP, vim_visual_mode,                 (Sft|KeyCode_V));
	VimBind(N|V|MAP, vim_visual_mode,                 (Ctl|KeyCode_V));
	VimBind(N|MAP,   vim_prev_visual,               SUB_G, KeyCode_V);
	VimBind(N|MAP,   vim_newline_below,                    KeyCode_O);
	VimBind(N|MAP,   vim_newline_above,               (Sft|KeyCode_O));
	VimBind(V|MAP,   vim_visual_insert,               (Sft|KeyCode_A));
	VimBind(V|MAP,   vim_visual_insert,               (Sft|KeyCode_I));
	VimBind(I|MAP,   vim_insert_command,          (Ctl|Sft|KeyCode_O));

	/// Sub Mode Binds
	VimBind(N|V|MAP, vim_submode_g,                        KeyCode_G);
	VimBind(N|V|MAP, vim_submode_z,                        KeyCode_Z);
	VimBind(N|V|MAP, vim_submode_leader,                   leader);

	/// Request Binds
	VimBind(N|V|MAP, vim_request_yank,                     KeyCode_Y);
	VimBind(N|V|MAP, vim_request_delete,                   KeyCode_D);
	VimBind(N|V|MAP, vim_request_change,                   KeyCode_C);
	VimBind(N|V|MAP, vim_delete_end,                  (Sft|KeyCode_D));
	VimBind(N|V|MAP, vim_change_end,                  (Sft|KeyCode_C));
	VimBind(N|V|MAP, vim_yank_end,                    (Sft|KeyCode_Y));
	VimBind(N|V|MAP, vim_request_auto_indent,              KeyCode_Equal);
	VimBind(N|V|MAP, vim_lowercase,            SUB_G,      KeyCode_U);
	VimBind(N|V|MAP, vim_uppercase,            SUB_G, (Sft|KeyCode_U));
	VimBind(N|V|MAP, vim_toggle_case,          SUB_G, (Sft|KeyCode_Tick));
	VimBind(N|V|MAP, vim_request_indent,              (Sft|KeyCode_Period));
	VimBind(N|V|MAP, vim_request_outdent,             (Sft|KeyCode_Comma));
	VimBind(N|V|MAP, vim_request_fold,              SUB_Z, KeyCode_F);
	VimBind(N|V|MAP, fold_toggle_cursor,            SUB_Z, KeyCode_A);
	VimBind(N|V|MAP, fold_pop_cursor,               SUB_Z, KeyCode_D);
	VimBind(V|MAP,   vim_toggle_case,                 (Sft|KeyCode_Tick));
	VimBind(V|MAP,   vim_lowercase,                        KeyCode_U);
	VimBind(V|MAP,   vim_uppercase,                   (Sft|KeyCode_U));
	VimBind(V|MAP,   vim_replace_range_next,               KeyCode_R);

	/// Edit Binds
	VimBind(N|MAP,   vim_paste_before,               (Sft|KeyCode_P));
	VimBind(N|MAP,   vim_paste_after,                     KeyCode_P);
	VimBind(N|MAP,   vim_backspace_char,             (Sft|KeyCode_X));
	VimBind(N|MAP,   vim_delete_char,                     KeyCode_X);
	VimBind(N|MAP,   vim_replace_next_char,               KeyCode_R);
	VimBind(N|V|MAP, vim_combine_line,                (Sft|KeyCode_J));
	VimBind(N|V|MAP, vim_combine_line,         SUB_G, (Sft|KeyCode_J));
	VimBind(N|MAP,   vim_last_command,                     KeyCode_Period);
	VimBind(N|V|MAP, vim_select_register,             (Sft|KeyCode_Quote));
	VimBind(N|MAP,   vim_toggle_char,                 (Sft|KeyCode_Tick));
	VimBind(I|MAP,   vim_select_register,             (Ctl|KeyCode_R));
	VimBind(I|MAP,   vim_delete_to_begin,             (Ctl|KeyCode_U));
	VimBind(V|MAP,   vim_move_selection_up,           (Meta|KeyCode_K));
	VimBind(V|MAP,   vim_move_selection_down,         (Meta|KeyCode_J));

	/// Digit Binds
	VimBind(N|V|MAP, vim_modal_0,                          KeyCode_0);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_1);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_2);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_3);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_4);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_5);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_6);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_7);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_8);
	VimBind(N|V|MAP, vim_digit,                            KeyCode_9);
	VimBind(N|V|MAP, vim_digit_del,                        KeyCode_Backspace);
	VimBind(N|V|MAP, vim_digit_del,                        KeyCode_Delete);

	/// Movement Binds
	VimBind(N|V|MAP, vim_left,                             KeyCode_H);
	VimBind(N|V|MAP, vim_down,                             KeyCode_J);
	VimBind(N|V|MAP, vim_up,                               KeyCode_K);
	VimBind(N|V|MAP, vim_right,                            KeyCode_L);
	VimBind(N|V|MAP, vim_end_line,                    (Sft|KeyCode_4));
	VimBind(N|V|MAP, vim_begin_line,                  (Sft|KeyCode_6));
	VimBind(N|V|MAP, vim_forward_word,                     KeyCode_W);
	VimBind(N|V|MAP, vim_forward_WORD,                (Sft|KeyCode_W));
	VimBind(N|V|MAP, vim_backward_word,                    KeyCode_B);
	VimBind(N|V|MAP, vim_backward_WORD,               (Sft|KeyCode_B));
	VimBind(N|V|MAP, vim_forward_end,                      KeyCode_E);
	VimBind(N|V|MAP, vim_forward_END,                 (Sft|KeyCode_E));
	VimBind(N|V|MAP, vim_backward_end,         SUB_G,      KeyCode_E);
	VimBind(N|V|MAP, vim_backward_END,         SUB_G, (Sft|KeyCode_E));

	VimBind(N|V|MAP, vim_file_top,             SUB_G,     KeyCode_G);
	VimBind(N|V|MAP, vim_goto_line,                   (Sft|KeyCode_G));
	VimBind(N|V|MAP, vim_goto_column,                 (Sft|KeyCode_BackwardSlash));
	VimBind(N|V|MAP, vim_modal_percent,               (Sft|KeyCode_5));
	VimBind(N|V|MAP, vim_bounce,                      (Ctl|KeyCode_5));
	VimBind(N|V|MAP, vim_set_seek_char,                    KeyCode_F);
	VimBind(N|V|MAP, vim_set_seek_char,               (Sft|KeyCode_F));
	VimBind(N|V|MAP, vim_paragraph_up,                     KeyCode_LeftBracket);
	VimBind(N|V|MAP, vim_paragraph_down,                   KeyCode_RightBracket);
	VimBind(N|V|MAP, vim_screen_top,                  (Sft|KeyCode_H));
	VimBind(N|V|MAP, vim_screen_bot,                  (Sft|KeyCode_L));
	VimBind(N|V|MAP, vim_screen_mid,                  (Sft|KeyCode_M));
	VimBind(V|MAP,   cursor_mark_swap,                     KeyCode_O);
	VimBind(V|MAP,   vim_block_swap,                  (Sft|KeyCode_O));

	VimBind(N|MAP, vim_search_identifier,         (Ctl|Sft|KeyCode_8));
	VimBind(N|MAP, vim_search_identifier,             (Sft|KeyCode_8));
	VimBind(N|MAP, vim_clear_search,          SUB_Leader,  KeyCode_Space);
	VimBind(N|MAP, vim_start_search_forward,               KeyCode_ForwardSlash);
	VimBind(N|MAP, vim_start_search_backward,         (Sft|KeyCode_ForwardSlash));
	VimBind(N|MAP, vim_to_next_pattern,                    KeyCode_N);
	VimBind(N|MAP, vim_to_prev_pattern,               (Sft|KeyCode_N));
	VimBind(N|MAP, vim_in_next_pattern,        SUB_G,      KeyCode_N);
	VimBind(N|MAP, vim_in_prev_pattern,        SUB_G, (Sft|KeyCode_N));

	VimBind(N|MAP, vim_prev_jump,                     (Ctl|KeyCode_O));
	VimBind(N|MAP, vim_next_jump,                     (Ctl|KeyCode_I));

	/// Screen Adjust Binds
	VimBind(N|V|MAP, vim_half_page_up,                (Ctl|KeyCode_U));
	VimBind(N|V|MAP, vim_half_page_down,              (Ctl|KeyCode_D));
	VimBind(N|V|MAP, vim_whole_page_up,               (Ctl|KeyCode_B));
	VimBind(N|V|MAP, vim_whole_page_down,             (Ctl|KeyCode_F));
	VimBind(N|V|MAP, vim_line_down,               (Ctl|Sft|KeyCode_Y));
	VimBind(N|V|MAP, vim_line_down,                   (Ctl|KeyCode_Y));
	VimBind(N|V|MAP, vim_line_up,                 (Ctl|Sft|KeyCode_E));
	VimBind(N|V|MAP, vim_line_up,                     (Ctl|KeyCode_E));
	VimBind(N|V|MAP, vim_scroll_screen_top,         SUB_Z, KeyCode_T);
	VimBind(N|V|MAP, vim_scroll_screen_mid,         SUB_Z, KeyCode_Z);
	VimBind(N|V|MAP, vim_scroll_screen_bot,         SUB_Z, KeyCode_B);

	/// Miscellaneous Binds
	VimBind(N|V|MAP, vim_set_mark,                         KeyCode_M);
	VimBind(N|V|MAP, vim_goto_mark,                        KeyCode_Tick);
	VimBind(N|V|MAP, vim_goto_mark,                        KeyCode_Quote);
	VimBind(N|V|MAP, vim_toggle_macro,                     KeyCode_Q);
	VimBind(N|V|MAP, vim_play_macro,                  (Sft|KeyCode_2));

	/// Window Binds (TODO(BYP): Do the rest of these, and do them properly)
	VimBind(N|MAP, change_active_panel_backwards,      Ctl|KeyCode_H);
	VimBind(N|MAP, change_active_panel_backwards,      Ctl|KeyCode_J);
	VimBind(N|MAP, change_active_panel,                Ctl|KeyCode_K);
	VimBind(N|MAP, change_active_panel,                Ctl|KeyCode_L);

    // KV binds
	VimBind(N|  MAP,  save,                    KeyCode_Return);
	VimBind(N|  MAP,  byp_space,               KeyCode_Space);
	VimBind(N|  MAP,  vim_insert_end,          KeyCode_A);
    VimBind(  V|MAP,  vim_end_line,            KeyCode_A);
    VimBind(N|V|MAP,  vim_line_start,     Meta|KeyCode_I);
    VimBind(N|  MAP,  kv_shift_character,      KeyCode_Comma);

	VimBind(N|MAP,   byp_request_comment,   SUB_G,       KeyCode_ForwardSlash);
	VimBind(N|MAP,   byp_request_uncomment, SUB_G,  (Sft|KeyCode_ForwardSlash));

	VimBind(N|V|MAP, vim_leader_d, SUB_Leader,       KeyCode_D);
	VimBind(N|V|MAP, vim_leader_c, SUB_Leader,       KeyCode_C);
	VimBind(N|V|MAP, vim_leader_D, SUB_Leader,  (Sft|KeyCode_D));
	VimBind(N|V|MAP, vim_leader_C, SUB_Leader,  (Sft|KeyCode_C));

#undef I
#undef N
#undef V
#undef MAP
}

function void
byp_default_bindings(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
	MappingScope();
	SelectMapping(mapping);

	SelectMap(global_id);

	Bind(project_go_to_root_directory,  KeyCode_H, KeyCode_Control);
	Bind(save_all_dirty_buffers,        KeyCode_S, KeyCode_Control, KeyCode_Shift);
	Bind(byp_build_project,             KeyCode_M, KeyCode_Alt);
	Bind(execute_any_cli,               KeyCode_Z, KeyCode_Alt);
	Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
	Bind(quick_swap_buffer,             KeyCode_BackwardSlash, KeyCode_Alt);
	Bind(exit_4coder,                   KeyCode_F4, KeyCode_Alt);

	Bind(byp_project_fkey_command, KeyCode_F1);
	Bind(byp_project_fkey_command, KeyCode_F2);
	Bind(byp_project_fkey_command, KeyCode_F3);
	Bind(byp_project_fkey_command, KeyCode_F4);
	Bind(byp_project_fkey_command, KeyCode_F5);
	Bind(byp_project_fkey_command, KeyCode_F6);
	Bind(byp_project_fkey_command, KeyCode_F7);
	Bind(byp_project_fkey_command, KeyCode_F8);
	Bind(byp_project_fkey_command, KeyCode_F9);
	Bind(byp_project_fkey_command, KeyCode_F10);
	Bind(byp_project_fkey_command, KeyCode_F11);
	Bind(byp_project_fkey_command, KeyCode_F12);
	Bind(byp_project_fkey_command, KeyCode_F13);
	Bind(byp_project_fkey_command, KeyCode_F14);
	Bind(byp_project_fkey_command, KeyCode_F15);
	Bind(byp_project_fkey_command, KeyCode_F16);
	Bind(toggle_fullscreen, KeyCode_F11);

	Bind(vim_interactive_open_or_new,                   KeyCode_O, KeyCode_Control);
	Bind(byp_test,                                      KeyCode_BackwardSlash, KeyCode_Control);
	Bind(increase_face_size,                            KeyCode_Equal, KeyCode_Control);
	Bind(decrease_face_size,                            KeyCode_Minus, KeyCode_Control);
	Bind(byp_reset_face_size,                           KeyCode_0, KeyCode_Control);
	Bind(vim_proj_cmd_lister,                           KeyCode_X, KeyCode_Alt, KeyCode_Shift);
	Bind(byp_toggle_set_col_ruler,                      KeyCode_BackwardSlash, KeyCode_Control, KeyCode_Shift);


	SelectMap(file_id);
	ParentMap(global_id);

	Bind(delete_char,                                   KeyCode_Delete);
	Bind(backspace_char,                                KeyCode_Backspace);
	Bind(move_up,                                       KeyCode_Up);
	Bind(move_down,                                     KeyCode_Down);
	Bind(move_left,                                     KeyCode_Left);
	Bind(move_right,                                    KeyCode_Right);
	Bind(seek_end_of_line,                              KeyCode_End);
	Bind(right_adjust_view,                             KeyCode_Home);
	Bind(move_up_to_blank_line_end,                     KeyCode_Up, KeyCode_Control);
	Bind(move_down_to_blank_line_end,                   KeyCode_Down, KeyCode_Control);
	Bind(backspace_alpha_numeric_boundary,              KeyCode_Backspace, KeyCode_Control);
	Bind(delete_alpha_numeric_boundary,                 KeyCode_Delete, KeyCode_Control);
	Bind(snipe_backward_whitespace_or_token_boundary,   KeyCode_Backspace, KeyCode_Alt);
	Bind(snipe_forward_whitespace_or_token_boundary,    KeyCode_Delete, KeyCode_Alt);
	Bind(set_mark,                                      KeyCode_Space, KeyCode_Control);
	Bind(delete_range,                                  KeyCode_D, KeyCode_Control);
	Bind(delete_line,                                   KeyCode_D, KeyCode_Control, KeyCode_Shift);
	Bind(search,                                        KeyCode_F, KeyCode_Control);
	Bind(list_all_locations,                            KeyCode_F, KeyCode_Control, KeyCode_Shift);
	Bind(list_all_substring_locations_case_insensitive, KeyCode_F, KeyCode_Alt);
	Bind(list_all_locations_of_selection,               KeyCode_G, KeyCode_Control, KeyCode_Shift);
	Bind(snippet_lister,                                KeyCode_J, KeyCode_Control);
	Bind(kill_buffer,                                   KeyCode_K, KeyCode_Control, KeyCode_Shift);
	Bind(duplicate_line,                                KeyCode_L, KeyCode_Control);
	Bind(cursor_mark_swap,                              KeyCode_M, KeyCode_Control);
	Bind(query_replace,                                 KeyCode_Q, KeyCode_Control);
	Bind(query_replace_identifier,                      KeyCode_Q, KeyCode_Control, KeyCode_Shift);
	Bind(query_replace_selection,                       KeyCode_Q, KeyCode_Alt);
	Bind(save,                                          KeyCode_S, KeyCode_Control);
	Bind(save_all_dirty_buffers,                        KeyCode_S, KeyCode_Control, KeyCode_Shift);
	Bind(search_identifier,                             KeyCode_T, KeyCode_Control);
	Bind(list_all_locations_of_identifier,              KeyCode_T, KeyCode_Control, KeyCode_Shift);
	Bind(paste_and_indent,                              KeyCode_V, KeyCode_Control);
	Bind(cut,                                           KeyCode_X, KeyCode_Control);
	Bind(redo,                                          KeyCode_Y, KeyCode_Control);
	Bind(undo,                                          KeyCode_Z, KeyCode_Control);
	Bind(goto_jump_at_cursor,                           KeyCode_Return);
	Bind(goto_jump_at_cursor_same_panel,                KeyCode_Return, KeyCode_Shift);
	Bind(view_jump_list_with_lister,                    KeyCode_Period, KeyCode_Control, KeyCode_Shift);

	SelectMap(code_id);
	ParentMap(file_id);

	Bind(comment_line_toggle,                           KeyCode_Semicolon, KeyCode_Control);
	Bind(word_complete,                                 KeyCode_Tab);
	Bind(if0_off,                                       KeyCode_I, KeyCode_Alt);
	Bind(open_matching_file_cpp,                        KeyCode_2, KeyCode_Alt);
}
