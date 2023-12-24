#include "4coder_vim/4coder_vim_include.h"
#include "4coder_kv_commands.cpp"

// #include "4coder_byp_hooks.cpp"

function void
kv_essential_mapping(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
	MappingScope();
	SelectMapping(mapping);

	SelectMap(global_id);
	BindCore(default_startup, CoreCode_Startup);
	BindCore(vim_try_exit, CoreCode_TryExit);
	BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
	BindMouseWheel(mouse_wheel_scroll);
	BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
	//BindCore(vim_file_externally_modified, CoreCode_FileExternallyModified);

	SelectMap(file_id);
	ParentMap(global_id);
	BindTextInput(write_text_input);
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
  u32 N = bit_1;
  u32 I = bit_2;
  u32 V = bit_3;
  u32 MAP = 0;
#define BIND(...) if (!VimBind(__VA_ARGS__)) { printf("Keymap conflict at line %d!!!\n", __LINE__); }

	u32 Ctl  = KeyMod_Ctl;
	u32 Shift  = KeyMod_Sft;
    u32 Meta = OS_MAC ? KeyMod_Cmd : KeyMod_Alt;
    Key_Code leader = KeyCode_BackwardSlash;

	BIND(MAP, vim_normal_mode,                          KeyCode_Escape);
	BIND(MAP, vim_inc_buffer_peek,                 (Ctl|KeyCode_RightBracket));
	BIND(MAP, vim_dec_buffer_peek,                 (Ctl|KeyCode_LeftBracket));
	BIND(MAP, vim_toggle_show_buffer_peek,         (Ctl|KeyCode_Return));
	BIND(MAP, vim_toggle_show_buffer_peek,    Ctl|Shift|KeyCode_Return);
	BIND(MAP, vim_scoll_buffer_peek_up,       Ctl|Shift|KeyCode_U);
	BIND(MAP, vim_scoll_buffer_peek_down,     Ctl|Shift|KeyCode_D);

	/// Rebinds
	BIND(N|MAP, undo,                                   KeyCode_U);
	BIND(N|MAP, undo,                              (Ctl|KeyCode_Z));
	BIND(N|MAP, redo,                              (Ctl|KeyCode_R));
	BIND(N|MAP, save,                              (Ctl|KeyCode_S));
	BIND(N|MAP, vim_open_file_in_quotes,         SUB_G, KeyCode_F);
	BIND(N|MAP, vim_interactive_open_or_new,   (Ctl|Shift|KeyCode_O));
	BIND(N|MAP, quick_swap_buffer,             (Shift|Ctl|KeyCode_6));
	BIND(N|MAP, vim_goto_definition,               (Ctl|KeyCode_W));
	BIND(N|MAP, vim_next_4coder_jump,              (Meta|KeyCode_N));
	BIND(N|MAP, vim_prev_4coder_jump,          (Shift|Meta|KeyCode_N));
	BIND(N|MAP, vim_first_4coder_jump,         (Shift|Meta|KeyCode_M));
	BIND(N|I|MAP, view_buffer_other_panel,         (Ctl|KeyCode_1));
	BIND(N|I|MAP, swap_panels,                     (Ctl|KeyCode_2));
	// BIND(I|MAP, word_complete_drop_down,           (Ctl|KeyCode_N));
	//VimBind(I|MAP, word_complete_drop_down,           (Ctl|KeyCode_P));

	/// Mode Binds
	BIND(N|V|MAP, vim_modal_i,                          KeyCode_I);
	BIND(N|V|MAP, vim_command_mode,                (Shift|KeyCode_Semicolon));
	BIND(N|MAP,   vim_insert_begin,                (Shift|KeyCode_I));
	BIND(N|MAP,   vim_replace_mode,                (Shift|KeyCode_R));
	BIND(N|V|MAP, vim_visual_mode,                      KeyCode_V);
	BIND(N|V|MAP, vim_visual_mode,                 (Shift|KeyCode_V));
	BIND(N|V|MAP, vim_visual_mode,                 (Ctl|KeyCode_V));
	BIND(N|MAP,   vim_prev_visual,               SUB_G, KeyCode_V);
	BIND(N|MAP,   vim_newline_below,                    KeyCode_O);
	BIND(N|MAP,   vim_newline_above,               (Shift|KeyCode_O));
	// BIND(V|MAP,   vim_visual_insert,               (Shift|KeyCode_A));
	// BIND(V|MAP,   vim_visual_insert,               (Shift|KeyCode_I));
	// BIND(I|MAP,   vim_insert_command,          (Ctl|Shift|KeyCode_O));

	/// Sub Mode Binds
	BIND(N|V|MAP, vim_submode_g,                        KeyCode_G);
	BIND(N|V|MAP, vim_submode_z,                        KeyCode_Z);
	BIND(N|V|MAP, vim_submode_leader,                   leader);

	/// Request Binds
	BIND(N|V|MAP, vim_request_yank,                     KeyCode_Y);
	BIND(N|V|MAP, vim_request_delete,                   KeyCode_D);
	BIND(N|V|MAP, vim_request_change,                   KeyCode_C);
	BIND(N|V|MAP, vim_delete_end,                  (Shift|KeyCode_D));
	BIND(N|V|MAP, vim_change_end,                  (Shift|KeyCode_C));
	BIND(N|V|MAP, vim_yank_end,                    (Shift|KeyCode_Y));
	BIND(N|V|MAP, vim_request_auto_indent,              KeyCode_Equal);
	BIND(N|V|MAP, vim_lowercase,            SUB_G,      KeyCode_U);
	BIND(N|V|MAP, vim_uppercase,            SUB_G, (Shift|KeyCode_U));
	BIND(N|V|MAP, vim_toggle_case,          SUB_G, (Shift|KeyCode_Tick));
    BIND(  V|MAP, vim_toggle_case,                      KeyCode_Comma);
	BIND(N|V|MAP, vim_request_indent,              (Shift|KeyCode_Period));
	BIND(N|V|MAP, vim_request_outdent,             (Shift|KeyCode_Comma));
	BIND(N|V|MAP, vim_request_fold,              SUB_Z, KeyCode_F);
	BIND(N|V|MAP, fold_toggle_cursor,            SUB_Z, KeyCode_A);
	BIND(N|V|MAP, fold_pop_cursor,               SUB_Z, KeyCode_D);
	BIND(V|MAP,   vim_toggle_case,                 (Shift|KeyCode_Tick));
	BIND(V|MAP,   vim_lowercase,                        KeyCode_U);
	BIND(V|MAP,   vim_uppercase,                   (Shift|KeyCode_U));
	BIND(V|MAP,   vim_replace_range_next,               KeyCode_R);

	/// Edit Binds
	BIND(N|MAP,   vim_paste_before,                    KeyCode_P);
	BIND(N|MAP,   vim_backspace_char,           (Shift|KeyCode_X));
	BIND(N|MAP,   vim_delete_char,                     KeyCode_X);
	BIND(N|MAP,   vim_replace_next_char,               KeyCode_R);
	BIND(N|V|MAP, vim_combine_line,             (Shift|KeyCode_J));
	BIND(N|V|MAP, vim_combine_line,      SUB_G, (Shift|KeyCode_J));
	BIND(N|MAP,   vim_last_command,                    KeyCode_Period);
	BIND(N|V|MAP, vim_select_register,          (Shift|KeyCode_Quote));
	BIND(N|MAP,   vim_toggle_char,              (Shift|KeyCode_Tick));
	BIND(I|MAP,   vim_select_register,            (Ctl|KeyCode_R));
	BIND(I|MAP,   vim_delete_to_begin,            (Ctl|KeyCode_U));
	BIND(V|MAP,   vim_move_selection_up,         (Meta|KeyCode_K));
	BIND(V|MAP,   vim_move_selection_down,       (Meta|KeyCode_J));

	/// Digit Binds
	BIND(N|V|MAP, vim_modal_0,                          KeyCode_0);
	BIND(N|V|MAP, vim_digit,                            KeyCode_1);
	BIND(N|V|MAP, vim_digit,                            KeyCode_2);
	BIND(N|V|MAP, vim_digit,                            KeyCode_3);
	BIND(N|V|MAP, vim_digit,                            KeyCode_4);
	BIND(N|V|MAP, vim_digit,                            KeyCode_5);
	BIND(N|V|MAP, vim_digit,                            KeyCode_6);
	BIND(N|V|MAP, vim_digit,                            KeyCode_7);
	BIND(N|V|MAP, vim_digit,                            KeyCode_8);
	BIND(N|V|MAP, vim_digit,                            KeyCode_9);
	BIND(N|V|MAP, vim_digit_del,                        KeyCode_Backspace);
	BIND(N|V|MAP, vim_digit_del,                        KeyCode_Delete);

	/// Movement Binds
	BIND(N|V|MAP, vim_left,                             KeyCode_H);
	BIND(N|V|MAP, vim_down,                             KeyCode_J);
	BIND(N|V|MAP, vim_up,                               KeyCode_K);
	BIND(N|V|MAP, vim_right,                            KeyCode_L);
	BIND(N|V|MAP, vim_end_line,                    (Shift|KeyCode_4));
	BIND(N|V|MAP, vim_begin_line,                  Meta|KeyCode_I);
	BIND(N|V|MAP, vim_forward_word,                     KeyCode_W);
	BIND(N|V|MAP, vim_forward_WORD,                (Shift|KeyCode_W));
	BIND(N|V|MAP, vim_backward_word,                    KeyCode_B);
	BIND(N|V|MAP, vim_backward_WORD,               (Shift|KeyCode_B));
	BIND(N|V|MAP, vim_forward_end,                      KeyCode_E);
	BIND(N|V|MAP, vim_forward_END,                 (Shift|KeyCode_E));
	BIND(N|V|MAP, vim_backward_end,         SUB_G,      KeyCode_E);
	BIND(N|V|MAP, vim_backward_END,         SUB_G, (Shift|KeyCode_E));

	BIND(N|V|MAP, vim_file_top,             SUB_G,     KeyCode_G);
	BIND(N|V|MAP, vim_goto_line,                   (Shift|KeyCode_G));
	BIND(N|V|MAP, vim_goto_column,                 (Shift|KeyCode_BackwardSlash));
	BIND(N|V|MAP, vim_modal_percent,               (Shift|KeyCode_5));
	BIND(N|V|MAP, vim_bounce,                      (Ctl|KeyCode_5));
	BIND(N|V|MAP, vim_set_seek_char,                    KeyCode_F);
	BIND(N|V|MAP, vim_set_seek_char,               (Shift|KeyCode_F));
	BIND(N|V|MAP, vim_paragraph_up,                     KeyCode_LeftBracket);
	BIND(N|V|MAP, vim_paragraph_down,                   KeyCode_RightBracket);
	BIND(N|V|MAP, vim_screen_top,                  (Shift|KeyCode_H));
	BIND(N|V|MAP, vim_screen_bot,                  (Shift|KeyCode_L));
	BIND(N|V|MAP, vim_screen_mid,                  (Shift|KeyCode_M));
	BIND(V|MAP,   cursor_mark_swap,                     KeyCode_O);
	BIND(V|MAP,   vim_block_swap,                  (Shift|KeyCode_O));

	BIND(N|MAP, vim_search_identifier,         (Ctl|Shift|KeyCode_8));
	BIND(N|MAP, vim_search_identifier,             (Shift|KeyCode_8));
	BIND(N|MAP, vim_clear_search,          SUB_Leader,  KeyCode_Space);
	BIND(N|MAP, vim_start_search_forward,               KeyCode_ForwardSlash);
	BIND(N|MAP, vim_start_search_backward,         (Shift|KeyCode_ForwardSlash));
	BIND(N|MAP, vim_to_next_pattern,                    KeyCode_N);
	BIND(N|MAP, vim_to_prev_pattern,               (Shift|KeyCode_N));
	BIND(N|MAP, vim_in_next_pattern,        SUB_G,      KeyCode_N);
	BIND(N|MAP, vim_in_prev_pattern,        SUB_G, (Shift|KeyCode_N));

	BIND(N|MAP, vim_prev_jump,                     (Ctl|KeyCode_O));
	BIND(N|MAP, vim_next_jump,                     (Ctl|KeyCode_I));

	/// Screen Adjust Binds
	BIND(N|V|MAP, vim_half_page_up,                (Ctl|KeyCode_U));
	BIND(N|V|MAP, vim_half_page_down,              (Ctl|KeyCode_D));
	BIND(N|V|MAP, vim_whole_page_up,               (Ctl|KeyCode_B));
	BIND(N|V|MAP, vim_whole_page_down,             (Ctl|KeyCode_F));
	BIND(N|V|MAP, vim_line_down,               (Ctl|Shift|KeyCode_Y));
	BIND(N|V|MAP, vim_line_down,                   (Ctl|KeyCode_Y));
	BIND(N|V|MAP, vim_line_up,                 (Ctl|Shift|KeyCode_E));
	BIND(N|V|MAP, vim_line_up,                     (Ctl|KeyCode_E));
	BIND(N|V|MAP, vim_scroll_screen_top,         SUB_Z, KeyCode_T);
	BIND(N|V|MAP, vim_scroll_screen_mid,         SUB_Z, KeyCode_Z);
	BIND(N|V|MAP, vim_scroll_screen_bot,         SUB_Z, KeyCode_B);

	/// Miscellaneous Binds
	BIND(N|V|MAP, vim_set_mark,                         KeyCode_M);
	BIND(N|V|MAP, vim_goto_mark,                        KeyCode_Tick);
	BIND(N|V|MAP, vim_goto_mark,                        KeyCode_Quote);
	BIND(N|V|MAP, vim_toggle_macro,                     KeyCode_Q);
	BIND(N|V|MAP, vim_play_macro,                  (Shift|KeyCode_2));

	/// Window Binds (TODO(BYP): Do the rest of these, and do them properly)
	BIND(N|MAP, change_active_panel_backwards,      Ctl|KeyCode_H);
	BIND(N|MAP, change_active_panel_backwards,      Ctl|KeyCode_J);
	BIND(N|MAP, change_active_panel,                Ctl|KeyCode_K);
	BIND(N|MAP, change_active_panel,                Ctl|KeyCode_L);

    // KV binds
	BIND(N|  MAP,  save,                    KeyCode_Return);
	BIND(N|  MAP,  byp_space,               KeyCode_Space);
	BIND(N|  MAP,  vim_insert_end,          KeyCode_A);
    BIND(  V|MAP,  vim_end_line,            KeyCode_A);
    BIND(N  |MAP,  vim_end_line,        Shift|KeyCode_A);
    BIND(N|  MAP,  kv_shift_character,      KeyCode_Comma);
    BIND(N|  MAP,  exit_4coder,        Meta|KeyCode_Q);

	BIND(N|MAP,   byp_request_comment,   SUB_G,       KeyCode_ForwardSlash);
	BIND(N|MAP,   byp_request_uncomment, SUB_G,  (Shift|KeyCode_ForwardSlash));

	BIND(N|V|MAP, vim_leader_d, SUB_Leader,       KeyCode_D);
	BIND(N|V|MAP, vim_leader_c, SUB_Leader,       KeyCode_C);
	BIND(N|V|MAP, vim_leader_D, SUB_Leader,  (Shift|KeyCode_D));
	BIND(N|V|MAP, vim_leader_C, SUB_Leader,  (Shift|KeyCode_C));

#undef BIND
}

// function void
// byp_default_bindings(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
// 	MappingScope();
// 	SelectMapping(mapping);

// 	SelectMap(global_id);

// 	Bind(project_go_to_root_directory,  KeyCode_H, KeyCode_Control);
// 	Bind(save_all_dirty_buffers,        KeyCode_S, KeyCode_Control, KeyCode_Shift);
// 	Bind(byp_build_project,             KeyCode_M, KeyCode_Alt);
// 	Bind(execute_any_cli,               KeyCode_Z, KeyCode_Alt);
// 	Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
// 	Bind(quick_swap_buffer,             KeyCode_BackwardSlash, KeyCode_Alt);
// 	Bind(exit_4coder,                   KeyCode_F4, KeyCode_Alt);

// 	Bind(byp_project_fkey_command, KeyCode_F1);
// 	Bind(byp_project_fkey_command, KeyCode_F2);
// 	Bind(byp_project_fkey_command, KeyCode_F3);
// 	Bind(byp_project_fkey_command, KeyCode_F4);
// 	Bind(byp_project_fkey_command, KeyCode_F5);
// 	Bind(byp_project_fkey_command, KeyCode_F6);
// 	Bind(byp_project_fkey_command, KeyCode_F7);
// 	Bind(byp_project_fkey_command, KeyCode_F8);
// 	Bind(byp_project_fkey_command, KeyCode_F9);
// 	Bind(byp_project_fkey_command, KeyCode_F10);
// 	Bind(byp_project_fkey_command, KeyCode_F11);
// 	Bind(byp_project_fkey_command, KeyCode_F12);
// 	Bind(byp_project_fkey_command, KeyCode_F13);
// 	Bind(byp_project_fkey_command, KeyCode_F14);
// 	Bind(byp_project_fkey_command, KeyCode_F15);
// 	Bind(byp_project_fkey_command, KeyCode_F16);
// 	Bind(toggle_fullscreen, KeyCode_F11);

// 	Bind(vim_interactive_open_or_new,                   KeyCode_O, KeyCode_Control);
// 	Bind(byp_test,                                      KeyCode_BackwardSlash, KeyCode_Control);
// 	Bind(increase_face_size,                            KeyCode_Equal, KeyCode_Control);
// 	Bind(decrease_face_size,                            KeyCode_Minus, KeyCode_Control);
// 	Bind(byp_reset_face_size,                           KeyCode_0, KeyCode_Control);
// 	Bind(vim_proj_cmd_lister,                           KeyCode_X, KeyCode_Alt, KeyCode_Shift);
// 	Bind(byp_toggle_set_col_ruler,                      KeyCode_BackwardSlash, KeyCode_Control, KeyCode_Shift);


// 	SelectMap(file_id);
// 	ParentMap(global_id);

// 	Bind(delete_char,                                   KeyCode_Delete);
// 	Bind(backspace_char,                                KeyCode_Backspace);
// 	Bind(move_up,                                       KeyCode_Up);
// 	Bind(move_down,                                     KeyCode_Down);
// 	Bind(move_left,                                     KeyCode_Left);
// 	Bind(move_right,                                    KeyCode_Right);
// 	Bind(seek_end_of_line,                              KeyCode_End);
// 	Bind(right_adjust_view,                             KeyCode_Home);
// 	Bind(move_up_to_blank_line_end,                     KeyCode_Up, KeyCode_Control);
// 	Bind(move_down_to_blank_line_end,                   KeyCode_Down, KeyCode_Control);
// 	Bind(backspace_alpha_numeric_boundary,              KeyCode_Backspace, KeyCode_Control);
// 	Bind(delete_alpha_numeric_boundary,                 KeyCode_Delete, KeyCode_Control);
// 	Bind(snipe_backward_whitespace_or_token_boundary,   KeyCode_Backspace, KeyCode_Alt);
// 	Bind(snipe_forward_whitespace_or_token_boundary,    KeyCode_Delete, KeyCode_Alt);
// 	Bind(set_mark,                                      KeyCode_Space, KeyCode_Control);
// 	Bind(delete_range,                                  KeyCode_D, KeyCode_Control);
// 	Bind(delete_line,                                   KeyCode_D, KeyCode_Control, KeyCode_Shift);
// 	Bind(search,                                        KeyCode_F, KeyCode_Control);
// 	Bind(list_all_locations,                            KeyCode_F, KeyCode_Control, KeyCode_Shift);
// 	Bind(list_all_substring_locations_case_insensitive, KeyCode_F, KeyCode_Alt);
// 	Bind(list_all_locations_of_selection,               KeyCode_G, KeyCode_Control, KeyCode_Shift);
// 	Bind(snippet_lister,                                KeyCode_J, KeyCode_Control);
// 	Bind(kill_buffer,                                   KeyCode_K, KeyCode_Control, KeyCode_Shift);
// 	Bind(duplicate_line,                                KeyCode_L, KeyCode_Control);
// 	Bind(cursor_mark_swap,                              KeyCode_M, KeyCode_Control);
// 	Bind(query_replace,                                 KeyCode_Q, KeyCode_Control);
// 	Bind(query_replace_identifier,                      KeyCode_Q, KeyCode_Control, KeyCode_Shift);
// 	Bind(query_replace_selection,                       KeyCode_Q, KeyCode_Alt);
// 	Bind(save,                                          KeyCode_S, KeyCode_Control);
// 	Bind(save_all_dirty_buffers,                        KeyCode_S, KeyCode_Control, KeyCode_Shift);
// 	Bind(search_identifier,                             KeyCode_T, KeyCode_Control);
// 	Bind(list_all_locations_of_identifier,              KeyCode_T, KeyCode_Control, KeyCode_Shift);
// 	Bind(paste_and_indent,                              KeyCode_V, KeyCode_Control);
// 	Bind(cut,                                           KeyCode_X, KeyCode_Control);
// 	Bind(redo,                                          KeyCode_Y, KeyCode_Control);
// 	Bind(undo,                                          KeyCode_Z, KeyCode_Control);
// 	Bind(goto_jump_at_cursor,                           KeyCode_Return);
// 	Bind(goto_jump_at_cursor_same_panel,                KeyCode_Return, KeyCode_Shift);
// 	Bind(view_jump_list_with_lister,                    KeyCode_Period, KeyCode_Control, KeyCode_Shift);

// 	SelectMap(code_id);
// 	ParentMap(file_id);

// 	Bind(comment_line_toggle,                           KeyCode_Semicolon, KeyCode_Control);
// 	Bind(word_complete,                                 KeyCode_Tab);
// 	Bind(if0_off,                                       KeyCode_I, KeyCode_Alt);
// 	Bind(open_matching_file_cpp,                        KeyCode_2, KeyCode_Alt);
// }
