#include "4coder_byp_token.cpp"

global b32 byp_relative_numbers;

CUSTOM_COMMAND_SIG(byp_toggle_relative_numbers)
CUSTOM_DOC("Toggles value for `relative_numbers`")
{ byp_relative_numbers ^= 1; }

function void
byp_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id, Buffer_ID buffer, Text_Layout_ID text_layout_id, Rect_f32 rect) {
	ProfileScope(app, "render buffer");
	b32 is_active_view = view_id == get_active_view(app, Access_Always);
	Rect_f32 prev_clip = draw_set_clip(app, rect);

	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);

	Face_Metrics metrics = get_face_metrics(app, face_id);
	u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
	f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
	f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));

	i64 cursor_pos = view_correct_cursor(app, view_id);
	view_correct_mark(app, view_id);

	b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
	if(use_scope_highlight){
		Color_Array colors = finalize_color_array(defcolor_back_cycle);
		draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
	}

	b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
	if(highlight_line_at_cursor && is_active_view){
		i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
		draw_line_highlight(app, text_layout_id, line_number, fcolor_id(defcolor_highlight_cursor_line));
	}

	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	if(token_array.tokens == 0){
		paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
	}else{
		byp_draw_token_colors(app, view_id, buffer, text_layout_id);
	}

	b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
	b32 use_jump_highlight  = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
	if(use_error_highlight || use_jump_highlight){
		Buffer_ID comp_buffer = get_buffer_by_name(app, string_u8_litexpr("*compilation*"), Access_Always);
		if(use_error_highlight){
			draw_jump_highlights(app, buffer, text_layout_id, comp_buffer, fcolor_id(defcolor_highlight_junk));
			// TODO(BYP): Draw error messsage annotations
		}

		if(use_jump_highlight){
			Buffer_ID jump_buffer = get_locked_jump_buffer(app);
			if(jump_buffer != comp_buffer){
				draw_jump_highlights(app, buffer, text_layout_id, jump_buffer, fcolor_id(defcolor_highlight_white));
			}
		}
	}

	b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
	if(use_paren_helper){
		Color_Array colors = finalize_color_array(defcolor_text_cycle);
		draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
	}

	b64 show_whitespace = false;
	view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
	if(show_whitespace){
		if(token_array.tokens == 0){
			draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
		}else{
			draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
		}
	}

	if(is_active_view && vim_state.mode == VIM_Visual){
		vim_draw_visual_mode(app, view_id, buffer, face_id, text_layout_id);
	}

	fold_draw(app, buffer, text_layout_id);

	vim_draw_search_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);

    vim_draw_cursor(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);

	paint_fade_ranges(app, text_layout_id, buffer);
	draw_text_layout_default(app, text_layout_id);

	vim_draw_after_text(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);

	draw_set_clip(app, prev_clip);
}

function Render_Caller_Function kv_render_caller;
function void
kv_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view)
{
	ProfileScope(app, "render caller");

	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);

	Face_ID face_id = get_face_id(app, 0);
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 line_height = face_metrics.line_height;
	f32 digit_advance = face_metrics.decimal_digit_advance;

	Rect_f32 region = view_get_screen_rect(app, view);
	Rect_f32 prev_clip = draw_set_clip(app, region);

	Rect_f32 global_rect = global_get_screen_rectangle(app);
	f32 filebar_y = global_rect.y1 - 2.f*line_height - vim_cur_filebar_offset;
	if(region.y1 >= filebar_y){ region.y1 = filebar_y; }

	draw_rectangle_fcolor(app, region, 0.f, fcolor_id(defcolor_back));

	region = vim_draw_query_bars(app, region, view, face_id);

	{
		Rect_f32_Pair pair = layout_file_bar_on_bot(region, line_height);
		pair.b = rect_split_top_bottom(pair.b, line_height).a;
		vim_draw_filebar(app, view, buffer, frame_info, face_id, pair.b);
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

	Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
	Buffer_Point_Delta_Result delta = delta_apply(app, view, frame_info.animation_dt, scroll);
	if(!block_match_struct(&scroll.position, &delta.point)){
		block_copy_struct(&scroll.position, &delta.point);
		view_set_buffer_scroll(app, view, scroll, SetBufferScroll_NoCursorChange);
	}
	if(delta.still_animating){ animate_in_n_milliseconds(app, 0); }
	Buffer_Point buffer_point = scroll.position;
	Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);

	if(show_line_number_margins){
		if(byp_relative_numbers)
			vim_draw_rel_line_number_margin(app, view, buffer, face_id, text_layout_id, line_number_rect);
		else
			vim_draw_line_number_margin(app, view, buffer, face_id, text_layout_id, line_number_rect);
	}else{
		draw_rectangle_fcolor(app, line_number_rect, 0.f, fcolor_id(defcolor_back));
	}

	byp_render_buffer(app, view, face_id, buffer, text_layout_id, region);

	text_layout_free(app, text_layout_id);
	draw_set_clip(app, prev_clip);
}
