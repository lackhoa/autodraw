#include "4coder_vim/4coder_vim_include.h"
#include "4coder_kv_core.cpp"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#if !defined(META_PASS)
#  include "generated/managed_id_metadata.cpp"
#endif

extern "C" b32 adMainFcoder(char *autodraw_path_chars);
  
function void kvInitShiftedTable()
{
  Base_Allocator *base = get_base_allocator_system();
  shifted_version_of_characters = make_table_u64_u64(base, 128);
#define INSERT(CHAR1, CHAR2) table_insert(&shifted_version_of_characters, CHAR1, CHAR2)
  {
    INSERT('a', 'A');
    INSERT('1', '!');
    INSERT('2', '@');
    INSERT('3', '#');
    INSERT('4', '$');
    INSERT('5', '%');
    INSERT('6', '^');
    INSERT('7', '&');
    INSERT('8', '*');
    INSERT('`', '~');
    INSERT('-', '_');
    INSERT(',', '<');
    INSERT('.', '>');
    INSERT(';', ':');
    INSERT('=', '+');
    INSERT('/', '?');
    INSERT('\\', '|');
  }
#undef INSERT
}

function void kvInitVimQuailTable()
{
  arrsetcap(vim_quail_table, 64);
  arrpush(vim_quail_table, (VimQuailEntry{SCu8(",."), SCu8("->")}));
  arrpush(vim_quail_table, (VimQuailEntry{SCu8(",,"), SCu8("_")}));
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
	BIND(N|  MAP,  write_space,               KeyCode_Space);
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

void custom_layer_init(Application_Links *app)
{
  default_framework_init(app);
  set_all_default_hooks(app);

  vim_buffer_peek_list[ArrayCount(vim_default_peek_list) + 0] = { buffer_identifier(string_u8_litexpr("*scratch*")), 1.f, 1.f };
  vim_buffer_peek_list[ArrayCount(vim_default_peek_list) + 1] = { buffer_identifier(string_u8_litexpr("todo.txt")),  1.f, 1.f };
  vim_request_vtable[VIM_REQUEST_COUNT + BYP_REQUEST_Title]     = byp_apply_title;
  vim_request_vtable[VIM_REQUEST_COUNT + BYP_REQUEST_Comment]   = byp_apply_comment;
  vim_request_vtable[VIM_REQUEST_COUNT + BYP_REQUEST_UnComment] = byp_apply_uncomment;

  vim_text_object_vtable[VIM_TEXT_OBJECT_COUNT + BYP_OBJECT_param0] = {',', (Vim_Text_Object_Func *)byp_object_param};
  vim_text_object_vtable[VIM_TEXT_OBJECT_COUNT + BYP_OBJECT_param1] = {';', (Vim_Text_Object_Func *)byp_object_param};
  vim_text_object_vtable[VIM_TEXT_OBJECT_COUNT + BYP_OBJECT_camel0] = {'_', (Vim_Text_Object_Func *)byp_object_camel};
  vim_text_object_vtable[VIM_TEXT_OBJECT_COUNT + BYP_OBJECT_camel1] = {'-', (Vim_Text_Object_Func *)byp_object_camel};
  vim_init(app);

  set_custom_hook(app, HookID_SaveFile,                kv_file_save);
  // set_custom_hook(app, HookID_BufferRegion,            byp_buffer_region);
  set_custom_hook(app, HookID_RenderCaller,            byp_render_caller);
  set_custom_hook(app, HookID_WholeScreenRenderCaller, vim_draw_whole_screen);

  set_custom_hook(app, HookID_Tick,             kv_tick);
  set_custom_hook(app, HookID_NewFile,          kv_new_file);
  set_custom_hook(app, HookID_BeginBuffer,      vim_begin_buffer);
  set_custom_hook(app, HookID_BufferEditRange,  vim_buffer_edit_range);
  set_custom_hook(app, HookID_ViewChangeBuffer, vim_view_change_buffer);
  set_custom_hook(app, HookID_ViewEventHandler, vim_view_input_handler);

  Thread_Context *tctx = get_thread_context(app);
  mapping_init(tctx, &framework_mapping);
  String_ID global_map_id = vars_save_string_lit("keys_global");
  String_ID file_map_id = vars_save_string_lit("keys_file");
  String_ID code_map_id = vars_save_string_lit("keys_code");
  kv_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);

  kvInitShiftedTable();
  kvInitVimQuailTable();  // TODO: init this table in the vim layer too!
  kv_vim_bindings(app);

  if (false)
  {
    char *todo_autodraw_path = (char *)"/Users/khoa/AutoDraw/build";
    adMainFcoder(todo_autodraw_path);
  }
}
