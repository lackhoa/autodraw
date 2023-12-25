#include "4coder_vim/4coder_vim_include.h"
#include "4coder_byp_token.h"
#include "4coder_byp_colors.cpp"
#include "4coder_kv_draw.cpp"
#define KV_UTILS_NO_SHORT_NAMES
#include "kv_utils.h"
#undef KV_UTILS_NO_SHORT_NAMES

Table_u64_u64 shifted_version_of_characters;

VIM_COMMAND_SIG(kv_shift_character)
{
  View_ID view = get_active_view(app, Access_ReadVisible);
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
  i64 pos = view_get_cursor_pos(app, view);

  u8 current_character = 0;
  buffer_read_range(app, buffer, Ii64(pos, pos+1), &current_character);

  u64 replacement_char = 0;
  if (character_is_upper(current_character)) {
    replacement_char = character_to_lower(current_character);
  } else if (character_is_lower(current_character)) {
    replacement_char = character_to_upper(current_character);
  } else {
    table_read(&shifted_version_of_characters, (u64)current_character, &replacement_char);
  }
  //
  if (replacement_char) {
    buffer_replace_range(app, buffer, Ii64(pos, pos+1), SCu8((u8 *)&replacement_char, 1));
  }

  move_right(app);
}

void kvInitVimQuailTable()
{
#define QUAIL_CAPACITY 64
  Arena arena = make_arena_system();
  vim_quail_keys   = push_array(&arena, String_Const_u8, QUAIL_CAPACITY);
  vim_quail_values = push_array(&arena, String_Const_u8, QUAIL_CAPACITY);

  i32 index = 0;

  index = vim_quail_count++;
  vim_quail_keys[index]   = SCu8(",.");
  vim_quail_values[index] = SCu8("->");

  index = vim_quail_count++;
  vim_quail_keys[index]   = SCu8(",,");
  vim_quail_values[index] = SCu8("_");

#undef QUAIL_CAPACITY
}

function void
kv_essential_mapping(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id)
{
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


VIM_REQUEST_SIG(byp_apply_comment){
	i64 line0 = get_line_number_from_pos(app, buffer, range.min);
	i64 line1 = get_line_number_from_pos(app, buffer, range.max);
	line1 += (line0 == line1);
	History_Group history_group = history_group_begin(app, buffer);
	for(i64 l=line0; l<line1; l++){
		i64 line_start = get_pos_past_lead_whitespace_from_line_number(app, buffer, l);
		b32 has_comment = c_line_comment_starts_at_position(app, buffer, line_start);
		if(!has_comment){
			buffer_replace_range(app, buffer, Ii64(line_start), string_u8_litexpr("//"));
			buffer_post_fade(app, buffer, 0.667f, Ii64_size(line_start,2), fcolor_resolve(fcolor_id(defcolor_paste)));
		}
	}
	history_group_end(history_group);
}

VIM_REQUEST_SIG(byp_apply_uncomment){
	i64 line0 = get_line_number_from_pos(app, buffer, range.min);
	i64 line1 = get_line_number_from_pos(app, buffer, range.max);
	line1 += (line0 == line1);
	History_Group history_group = history_group_begin(app, buffer);
	for(i64 l=line0; l<line1; l++){
		i64 line_start = get_pos_past_lead_whitespace_from_line_number(app, buffer, l);
		b32 has_comment = c_line_comment_starts_at_position(app, buffer, line_start);
		if(has_comment){
			buffer_replace_range(app, buffer, Ii64_size(line_start,2), string_u8_empty);
		}
	}
	history_group_end(history_group);
}

function void byp_make_vim_request(Application_Links *app, BYP_Vim_Request request){
	vim_make_request(app, Vim_Request_Type(VIM_REQUEST_COUNT + request));
}


VIM_COMMAND_SIG(byp_request_title){ byp_make_vim_request(app, BYP_REQUEST_Title); }
VIM_COMMAND_SIG(byp_request_comment){ byp_make_vim_request(app, BYP_REQUEST_Comment); }
VIM_COMMAND_SIG(byp_request_uncomment){ byp_make_vim_request(app, BYP_REQUEST_UnComment); }
VIM_COMMAND_SIG(byp_visual_comment){
	if(vim_state.mode == VIM_Visual){
		Vim_Edit_Type edit = vim_state.params.edit_type;
		byp_request_comment(app);
		vim_state.mode = VIM_Visual;
		vim_state.params.edit_type = edit;
	}
}
VIM_COMMAND_SIG(byp_visual_uncomment){
	if(vim_state.mode == VIM_Visual){
		Vim_Edit_Type edit = vim_state.params.edit_type;
		byp_request_uncomment(app);
		vim_state.mode = VIM_Visual;
		vim_state.params.edit_type = edit;
	}
}

global b32 byp_relative_numbers;

CUSTOM_COMMAND_SIG(byp_toggle_relative_numbers)
CUSTOM_DOC("Toggles value for `relative_numbers`")
{ byp_relative_numbers ^= 1; }

CUSTOM_COMMAND_SIG(byp_test)
CUSTOM_DOC("Just bound to the key I spam to execute whatever test code I'm working on")
{
	Scratch_Block scratch(app);
	View_ID view = get_active_view(app, Access_ReadVisible);

	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
	i64 pos = view_get_cursor_pos(app, view);
	Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));

	print_message(app, push_stringf(scratch, "Buffer[%d] = '%c'\n", pos, buffer_get_char(app, buffer, pos)));
	print_message(app, push_stringf(scratch, "Line %d Col: %d\n", cursor.line, cursor.col));

	Face_ID face = get_face_id(app, buffer);
	Face_Description desc = get_face_description(app, face);
	printf_message(app, scratch, "Face Size: %d \n", desc.parameters.pt_size);
}

CUSTOM_COMMAND_SIG(byp_reset_face_size)
CUSTOM_DOC("Resets face size to default")
{
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Face_ID face_id = get_face_id(app, buffer);
	Face_Description description = get_face_description(app, face_id);
	description.parameters.pt_size = (i32)def_get_config_u64(app, vars_save_string_lit("default_font_size"));
	try_modify_face(app, face_id, &description);
}


global b32 byp_bracket_opened;

CUSTOM_COMMAND_SIG(kv_text_input)
CUSTOM_DOC("implement quail mode")
{
	User_Input in = get_current_input(app);
	String_Const_u8 insert = to_writable(&in);
	byp_bracket_opened = insert.str[insert.size-1] == '{';
	write_text(app, insert);
}

CUSTOM_COMMAND_SIG(byp_auto_complete_bracket)
CUSTOM_DOC("Sets the right size of the view near the x position of the cursor.")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	i64 pos = view_get_character_legal_pos_from_pos(app, view, view_get_cursor_pos(app, view));
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	do{
		if(token_array.tokens == 0){
			if(byp_bracket_opened){
				write_text(app, string_u8_litexpr("\n\n}"));
				move_up(app);
				byp_bracket_opened = 0;
				return;
			}else{
				break;
			}
		}

		i64 first_index = token_index_from_pos(&token_array, pos);
		Token_Iterator_Array it = token_iterator_index(0, token_array.tokens, token_array.count, first_index);
		if(!token_it_dec(&it)){ break; }

		Token *token = token_it_read(&it);
		if(token && byp_bracket_opened && buffer_get_char(app, buffer, token->pos) == '{'){
			token_it_dec(&it);
			token = token_it_read(&it);
			if(token->kind == TokenBaseKind_Identifier){
				if(!token_it_dec(&it)){ break; }
				token = token_it_read(&it);
			}
			String_Const_u8 insert = string_u8_litexpr("\n\n};");
			insert.size -= (token->kind != byp_TokenKind_Struct);
			write_text(app, insert);
			move_up(app);
			byp_bracket_opened = 0;
			return;
		}
	}while(0);

	write_text(app, string_u8_litexpr("\n"));
	byp_bracket_opened = 0;
}

CUSTOM_COMMAND_SIG(explorer)
CUSTOM_DOC("Opens file explorer in hot directory")
{
	Scratch_Block scratch(app);
	String_Const_u8 hot = push_hot_directory(app, scratch);
	exec_system_command(app, 0, buffer_identifier(0), hot, string_u8_litexpr("explorer ."), 0);
}

CUSTOM_COMMAND_SIG(byp_list_all_locations_selection)
CUSTOM_DOC("Lists locations of selection range")
{
	vim_normal_mode(app);
	View_ID view = get_active_view(app, Access_ReadVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
	Range_i64 range = get_view_range(app, view);
	range.max++;

	Scratch_Block scratch(app);
	String_Const_u8 range_string = push_buffer_range(app, scratch, buffer, range);
	list_all_locations__generic(app, range_string, ListAllLocationsFlag_CaseSensitive|ListAllLocationsFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(byp_open_current_peek)
CUSTOM_DOC("Sets the active view to the current peeked buffer")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = buffer_identifier_to_id(app, vim_buffer_peek_list[vim_buffer_peek_index].buffer_id);
	view_set_buffer(app, view, buffer, SetBuffer_KeepOriginalGUI);
	vim_show_buffer_peek = 1;
	vim_toggle_show_buffer_peek(app);
}


function void
byp_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id) {
	ProfileScope(app, "default render caller");

	Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);

	Face_ID face_id = get_face_id(app, 0);
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 line_height = face_metrics.line_height;
	f32 digit_advance = face_metrics.decimal_digit_advance;

	Rect_f32 region = view_get_screen_rect(app, view_id);
	Rect_f32 prev_clip = draw_set_clip(app, region);

	Rect_f32 global_rect = global_get_screen_rectangle(app);
	f32 filebar_y = global_rect.y1 - 2.f*line_height - vim_cur_filebar_offset;
	if(region.y1 >= filebar_y){ region.y1 = filebar_y; }

	draw_rectangle_fcolor(app, region, 0.f, fcolor_id(defcolor_back));

	region = vim_draw_query_bars(app, region, view_id, face_id);

	{
		Rect_f32_Pair pair = layout_file_bar_on_bot(region, line_height);
		pair.b = rect_split_top_bottom(pair.b, line_height).a;
		vim_draw_filebar(app, view_id, buffer, frame_info, face_id, pair.b);
		region = pair.a;
	}

	// Draw borders
	if(region.x0 > global_rect.x0){
		Rect_f32_Pair border_pair = rect_split_left_right(region, 2.f);
		draw_rectangle_fcolor(app, border_pair.a, 0.f, fcolor_id(defcolor_margin));
		region = border_pair.b;
	}
	if(region.x1 < global_rect.x1){
		Rect_f32_Pair border_pair = rect_split_left_right_neg(region, 2.f);
		draw_rectangle_fcolor(app, border_pair.b, 0.f, fcolor_id(defcolor_margin));
		region = border_pair.a;
	}
	region.y0 += 3.f;


	if(show_fps_hud){
		Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
		draw_fps_hud(app, frame_info, face_id, pair.max);
		region = pair.min;
		animate_in_n_milliseconds(app, 1000);
	}

	// NOTE(allen): layout line numbers
	b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
	Rect_f32_Pair pair = (show_line_number_margins ?
						  (byp_relative_numbers ?
						   vim_line_number_margin(app, buffer, region, digit_advance) :
						   layout_line_number_margin(app, buffer, region, digit_advance)) :
						  rect_split_left_right(region, 1.5f*digit_advance));
	Rect_f32 line_number_rect = pair.min;
	region = pair.max;

	Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
	Buffer_Point_Delta_Result delta = delta_apply(app, view_id, frame_info.animation_dt, scroll);
	if(!block_match_struct(&scroll.position, &delta.point)){
		block_copy_struct(&scroll.position, &delta.point);
		view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
	}
	if(delta.still_animating){ animate_in_n_milliseconds(app, 0); }
	Buffer_Point buffer_point = scroll.position;
	Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);

	if(show_line_number_margins){
		if(byp_relative_numbers)
			vim_draw_rel_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
		else
			vim_draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
	}else{
		draw_rectangle_fcolor(app, line_number_rect, 0.f, fcolor_id(defcolor_back));
	}

	byp_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);

	text_layout_free(app, text_layout_id);
	draw_set_clip(app, prev_clip);
}

function void
kv_tick(Application_Links *app, Frame_Info frame_info) {
	code_index_update_tick(app);
	if(tick_all_fade_ranges(app, frame_info.animation_dt)){
		animate_in_n_milliseconds(app, 0);
	}

	vim_animate_filebar(app, frame_info);
	vim_animate_cursor(app, frame_info);

	vim_cursor_blink++;
}

BUFFER_HOOK_SIG(kv_file_save) {
  default_file_save(app, buffer_id);
  vim_file_save(app, buffer_id);
  return 0;
}

BUFFER_HOOK_SIG(kv_new_file) {
	Scratch_Block scratch(app);
	String_Const_u8 file_name = push_buffer_base_name(app, scratch, buffer_id);
	if(string_match(string_postfix(file_name, 4), string_u8_litexpr(".bat"))){
		Buffer_Insertion insert = begin_buffer_insertion_at_buffered(app, buffer_id, 0, scratch, KB(16));
		insertf(&insert, "@echo off" "\n");
		end_buffer_insertion(&insert);
		return 0;
	}

	return 0;
}

VIM_REQUEST_SIG(byp_apply_title) {
	Scratch_Block scratch(app);
	String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
	u8 prev = buffer_get_char(app, buffer, range.min-1);
	foreach(i, text.size){
		text.str[i] += u8(i32('A' - 'a')*((!character_is_alpha(prev) || prev == '_') &&
										  character_is_lower(text.str[i])));
		prev = text.str[i];
	}
	buffer_replace_range(app, buffer, range, text);
	buffer_post_fade(app, buffer, 0.667f, range, fcolor_resolve(fcolor_id(defcolor_paste)));
}

VIM_TEXT_OBJECT_SIG(byp_object_param) {
	u8 c = buffer_get_char(app, buffer, cursor_pos);
	Range_i64 range = Ii64(cursor_pos + (c == ',' || c == ';'));

	for(; range.min>0; range.min--){
		c = buffer_get_char(app, buffer, range.min);
		if(c == ',' || c == ';'){ break; }
		Scan_Direction bounce = vim_bounce_direction(c);
		if(bounce == Scan_Forward){ break; }
		if(bounce == Scan_Backward){
			range.min = vim_bounce_pair(app, buffer, range.min, c)-1;
			continue;
		}
	}

	i64 buffer_size = buffer_get_size(app, buffer);
	for(; range.max < buffer_size; range.max++){
		c = buffer_get_char(app, buffer, range.max);
		if(c == ',' || c == ';'){ break; }
		Scan_Direction bounce = vim_bounce_direction(c);
		if(bounce == Scan_Backward){ break; }
		if(bounce == Scan_Forward){
			range.max = vim_bounce_pair(app, buffer, range.max, c);
			continue;
		}
	}
	range.min++; range.max--;
	if(range.min >= range.max){ range = {}; }

	return range;
}

VIM_TEXT_OBJECT_SIG(byp_object_camel) {
	Range_i64 range = {};
	Scratch_Block scratch(app);
	Range_i64 line_range = get_line_range_from_pos(app, buffer, cursor_pos);
	i64 s = line_range.min;
	u8 *line_text = push_buffer_range(app, scratch, buffer, line_range).str;
	u8 c = line_text[cursor_pos-s];
	if(!character_is_alpha_numeric(c)){ return {}; }
	cursor_pos += line_text[cursor_pos-s] == '_';
	range.min = range.max = cursor_pos;

	b32 valid=false;
	for(; range.min>0; range.min--){
		c = line_text[range.min-s];
		if(!character_is_alpha_numeric(c) || c == '_'){ valid = true; break; }
	}
	if(!valid){ return {}; }

	valid=false;
	for(; range.max>0; range.max++){
		c = line_text[range.max-s];
		if(!character_is_alpha_numeric(c) || c == '_'){ valid = true; break; }
	}
	if(!valid){ return {}; }

	range.min += (vim_state.params.clusivity != VIM_Inclusive || line_text[range.min-s] != '_');
	range.max -= (vim_state.params.clusivity != VIM_Inclusive || line_text[range.max-s] != '_');
	if(range.min >= range.max){ range = {}; }

	return range;
}
