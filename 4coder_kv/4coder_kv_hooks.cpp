#include "4coder_kv_core.cpp"

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

function Tick_Function kv_tick;
function void kv_tick(Application_Links *app, Frame_Info frame_info)
{
  code_index_update_tick(app);
  if(tick_all_fade_ranges(app, frame_info.animation_dt)){
    animate_in_n_milliseconds(app, 0);
  }

  vim_animate_filebar(app, frame_info);
  vim_animate_cursor(app, frame_info);

  vim_cursor_blink++;

  f32 AUTOSAVE_PERIOD_SECONDS = 5.0f;
  seconds_since_last_keystroke += frame_info.literal_dt;
  if (seconds_since_last_keystroke > AUTOSAVE_PERIOD_SECONDS)
  {
    seconds_since_last_keystroke = 0;
    save_all_dirty_buffers(app);
    print_message_cstr(app, "auto-saved all dirty buffers\n");
  }
}

// (Application_Links *app, Frame_Info frame_info, View_ID view_id)
function Render_Caller_Function byp_render_caller;
function void byp_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view)
{
	ProfileScope(app, "default render caller");

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

