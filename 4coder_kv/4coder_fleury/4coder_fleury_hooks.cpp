//~ NOTE(rjf): Buffer Render

#pragma once

#include "4coder_fleury_hooks.h"
#include "4coder_fleury_colors.cpp"
#include "4coder_fleury_brace.cpp"
#include "4coder_fleury_error_annotations.cpp"
#include "4coder_fleury_divider_comments.cpp"
#include "4coder_fleury_cursor.cpp"
#include "4coder_fleury_calc.cpp"
#include "4coder_fleury_pos_context_tooltips.cpp"
#include "4coder_fleury_code_peek.cpp"
#include "4coder_fleury_recent_files.cpp"

function void
F4_RenderBuffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                Buffer_ID buffer, Text_Layout_ID text_layout_id,
                Rect_f32 rect, Frame_Info frame_info)
{
    Scratch_Block scratch(app);
    ProfileScope(app, "[Fleury] Render Buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if(token_array.tokens != 0)
    {
        F4_SyntaxHighlight(app, text_layout_id, &token_array);
        
        // NOTE(allen): Scan for TODOs and NOTEs
        b32 use_comment_keywords = def_get_config_b32(vars_save_string_lit("use_comment_keywords"));
        if(use_comment_keywords)
        {
            Comment_Highlight_Pair pairs[] =
            {
                {str8_lit("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {str8_lit("TODO"), finalize_color(defcolor_comment_pop, 1)},
                {def_get_config_string(scratch, vars_save_string_lit("user_name")), finalize_color(fleury_color_comment_user_name, 0)},
            };
            draw_comment_highlights(app, buffer, text_layout_id,
                                    &token_array, pairs, ArrayCount(pairs));
        }
    }
    else
    {
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    {
        ProfileScope(app, "[Fleury] Token Highlight");
        
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, cursor_pos);
        Token *token = token_it_read(&it);
        if(token && token->kind == TokenBaseKind_Identifier)
        {
            F4_RenderRangeHighlight(app, view_id, text_layout_id,
                                    Ii64(token->pos, token->pos + token->size),
                                    F4_RangeHighlightKind_Underline,
                                    fcolor_resolve(fcolor_id(fleury_color_token_highlight)));
        }
    }

    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
    f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
    f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));
    
    // NOTE(rjf): Cursor
    F4_Cursor_RenderEmacsStyle(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness, frame_info);
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    draw_set_clip(app, prev_clip);
}


//~ NOTE(rjf): Render hook

function void
F4_DrawFileBar(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Rect_f32 bar)
{
    Scratch_Block scratch(app);
    
    draw_rectangle_fcolor(app, bar, 0.f, fcolor_id(defcolor_bar));
    
    FColor base_color = fcolor_id(defcolor_base);
    FColor pop2_color = fcolor_id(defcolor_pop2);
    
    i64 cursor_position = view_get_cursor_pos(app, view_id);
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));
    
    Fancy_Line list = {};
    String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer);
    push_fancy_string(scratch, &list, base_color, unique_name);
    push_fancy_stringf(scratch, &list, base_color, " - Row: %3.lld Col: %3.lld -", cursor.line, cursor.col);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    switch (*eol_setting){
        case LineEndingKind_Binary:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" bin"));
        }break;
        
        case LineEndingKind_LF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" lf"));
        }break;
        
        case LineEndingKind_CRLF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" crlf"));
        }break;
    }
    
    u8 space[3];
    {
        Dirty_State dirty = buffer_get_dirty_state(app, buffer);
        String_u8 str = Su8(space, 0, 3);
        if (dirty != 0){
            string_append(&str, string_u8_litexpr(" "));
        }
        if (HasFlag(dirty, DirtyState_UnsavedChanges)){
            string_append(&str, string_u8_litexpr("*"));
        }
        if (HasFlag(dirty, DirtyState_UnloadedChanges)){
            string_append(&str, string_u8_litexpr("!"));
        }
        push_fancy_string(scratch, &list, pop2_color, str.string);
    }
    
    push_fancy_string(scratch, &list, base_color, S8Lit(" Syntax Mode: "));
    push_fancy_string(scratch, &list, base_color, F4_SyntaxOptionString());
    
    Vec2_f32 p = bar.p0 + V2f32(2.f, 2.f);
    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
    
    if(!def_get_config_b32(vars_save_string_lit("f4_disable_progress_bar")))
    {
        f32 progress = (f32)cursor.line / (f32)buffer_get_line_count(app, buffer);
        Rect_f32 progress_bar_rect =
        {
            bar.x0 + (bar.x1 - bar.x0) * progress,
            bar.y0,
            bar.x1,
            bar.y1,
        };
        ARGB_Color progress_bar_color = fcolor_resolve(fcolor_id(fleury_color_file_progress_bar));
        if(F4_ARGBIsValid(progress_bar_color))
        {
            draw_rectangle(app, progress_bar_rect, 0, progress_bar_color);
        }
    }
}

function void
F4_Render(Application_Links *app, Frame_Info frame_info, View_ID view_id)
{
    F4_RecentFiles_RefreshView(app, view_id);
    
    ProfileScope(app, "[Fleury] Render");
    Scratch_Block scratch(app);
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    f32 margin_size = (f32)def_get_config_u64(app, vars_save_string_lit("f4_margin_size"));
    Rect_f32 view_rect = view_get_screen_rect(app, view_id);
    Rect_f32 region = rect_inner(view_rect, margin_size);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer);
    
    //~ NOTE(rjf): Draw background.
    {
        ARGB_Color color = fcolor_resolve(fcolor_id(defcolor_back));
        if(string_match(buffer_name, string_u8_litexpr("*compilation*")))
        {
            color = color_blend(color, 0.5f, 0xff000000);
        }
        // NOTE(rjf): Inactive background color.
        else if(is_active_view == 0)
        {
            ARGB_Color inactive_bg_color = fcolor_resolve(fcolor_id(fleury_color_inactive_pane_background));
            if(F4_ARGBIsValid(inactive_bg_color))
            {
                color = inactive_bg_color;
            }
        }
        draw_rectangle(app, region, 0.f, color);
        draw_margin(app, view_rect, region, color);
    }
    
    //~ NOTE(rjf): Draw margin.
    {
        ARGB_Color color = fcolor_resolve(fcolor_id(defcolor_margin));
        if(def_get_config_b32(vars_save_string_lit("f4_margin_use_mode_color")) &&
           is_active_view)
        {
            color = F4_GetColor(app, ColorCtx_Cursor(power_mode.enabled ? ColorFlag_PowerMode : 0,
                                                     GlobalKeybindingMode));
        }
        draw_margin(app, view_rect, region, color);
    }
    
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if(view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar)
    {
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        F4_DrawFileBar(app, view_id, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id, frame_info.animation_dt, scroll);
    
    if(!block_match_struct(&scroll.position, &delta.point))
    {
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    
    if(delta.still_animating)
    {
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars))
        {
            for (i32 i = 0; i < query_bars.count; i += 1)
            {
                Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, 1);
                draw_query_bar(app, query_bars.ptrs[i], face_id, pair.min);
                region = pair.max;
            }
        }
    }
    
    // NOTE(allen): FPS hud
    if(show_fps_hud)
    {
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    Rect_f32 line_number_rect = {};
    if(def_get_config_b32(vars_save_string_lit("show_line_number_margins")))
    {
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        line_number_rect.x1 += 4;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    if(is_active_view)
    {
        buffer_point.pixel_shift.y += F4_PowerMode_ScreenShake()*1.f;
    }
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if(def_get_config_b32(vars_save_string_lit("show_line_number_margins")))
    {
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    F4_RenderBuffer(app, view_id, face_id, buffer, text_layout_id, region, frame_info);
    
    // NOTE(rjf): Render command server
#if OS_WINDOWS
    CS_render_caller(app, frame_info, view_id);
#endif
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}

//~ NOTE(rjf): Begin buffer hook

function BUFFER_HOOK_SIG(F4_BeginBuffer)
{
    ProfileScope(app, "[Fleury] Begin Buffer");
    
    Scratch_Block scratch(app);
    b32 treat_as_code = false;
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    
    // NOTE(rjf): Treat as code if the config tells us to.
    if(treat_as_code == false)
    {
        if(file_name.size > 0)
        {
            String_Const_u8 treat_as_code_string = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
            String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code_string);
            String_Const_u8 ext = string_file_extension(file_name);
            for(i32 i = 0; i < extensions.count; ++i)
            {
                if(string_match(ext, extensions.strings[i]))
                {
                    treat_as_code = true;
                    break;
                }
            }
        }
    }
    
    // NOTE(rjf): Treat as code for *calc* and *peek* buffers.
    if(treat_as_code == false)
    {
        if(buffer_name.size > 0)
        {
            if(string_match(buffer_name, string_u8_litexpr("*calc*")))
            {
                treat_as_code = true;
            }
            else if(string_match(buffer_name, string_u8_litexpr("*peek*")))
            {
                treat_as_code = true;
            }
        }
    }
    
    // NOTE(rjf): Treat as code if we've identified the language of a file.
    if(treat_as_code == false)
    {
        F4_Language *language = F4_LanguageFromBuffer(app, buffer_id);
        if(language)
        {
            treat_as_code = true;
        }
    }
    
    String_ID file_map_id = vars_save_string_lit("keys_file");
    String_ID code_map_id = vars_save_string_lit("keys_code");
    
    Command_Map_ID map_id = (treat_as_code) ? (code_map_id) : (file_map_id);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_lexer = false;
    if(treat_as_code)
    {
        wrap_lines = def_get_config_b32(vars_save_string_lit("enable_code_wrapping"));
        use_lexer = true;
    }
    
    if(string_match(buffer_name, string_u8_litexpr("*compilation*")))
    {
        wrap_lines = false;
    }
    
    if (use_lexer)
    {
        ProfileBlock(app, "begin buffer kick off lexer");
        Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
        *lex_task_ptr = async_task_no_dep(&global_async_system, F4_DoFullLex_ASYNC, make_data_struct(&buffer_id));
    }
    
    {
        b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
        *wrap_lines_ptr = wrap_lines;
    }
    
    if (use_lexer)
    {
        buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
    }
    else
    {
        if (treat_as_code)
        {
            buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
        }
        else{
            buffer_set_layout(app, buffer_id, layout_generic);
        }
    }
    
    
    // no meaning for return
    return(0);
}


//~ NOTE(rjf): Layout

function Layout_Item_List
F4_LayoutInner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, Layout_Virtual_Indent virt_indent)
{
    Layout_Item_List list = get_empty_item_list(range);
    
    Scratch_Block scratch(app);
    String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
    
    Face_Advance_Map advance_map = get_face_advance_map(app, face);
    Face_Metrics metrics = get_face_metrics(app, face);
    f32 tab_width = (f32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
    tab_width = clamp_bot(1, tab_width);
    LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, tab_width, width);
    
    if (text.size == 0){
        lr_tb_write_blank(&pos_vars, face, arena, &list, range.first);
    }
    else{
        b32 skipping_leading_whitespace = (virt_indent == LayoutVirtualIndent_On);
        Newline_Layout_Vars newline_vars = get_newline_layout_vars();
        
        u8 *ptr = text.str;
        u8 *end_ptr = ptr + text.size;
        for (;ptr < end_ptr;){
            Character_Consume_Result consume = utf8_consume(ptr, (u64)(end_ptr - ptr));
            
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            switch (consume.codepoint){
                case '\t':
                case ' ':
                {
                    newline_layout_consume_default(&newline_vars);
                    f32 advance = lr_tb_advance(&pos_vars, face, consume.codepoint);
                    if (!skipping_leading_whitespace){
                        lr_tb_write_with_advance(&pos_vars, face, advance, arena, &list, index, consume.codepoint);
                    }
                    else{
                        lr_tb_advance_x_without_item(&pos_vars, advance);
                    }
                }break;
                
                default:
                {
                    newline_layout_consume_default(&newline_vars);
                    lr_tb_write(&pos_vars, face, arena, &list, index, consume.codepoint);
                }break;
                
                case '\r':
                {
                    newline_layout_consume_CR(&newline_vars, index);
                }break;
                
                case '\n':
                {
                    i64 newline_index = newline_layout_consume_LF(&newline_vars, index);
                    lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
                    lr_tb_next_line(&pos_vars);
                }break;
                
                case max_u32:
                {
                    newline_layout_consume_default(&newline_vars);
                    lr_tb_write_byte(&pos_vars, face, arena, &list, index, *ptr);
                }break;
            }
            
            ptr += consume.inc;
        }
        
        if (newline_layout_consume_finish(&newline_vars)){
            i64 index = layout_index_from_ptr(ptr, text.str, range.first);
            lr_tb_write_blank(&pos_vars, face, arena, &list, index);
        }
    }
    
    layout_item_list_finish(&list, -pos_vars.line_to_text_shift);
    
    return(list);
}

function Layout_Item_List
F4_Layout(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width)
{
    return(F4_LayoutInner(app, arena, buffer, range, face, width, LayoutVirtualIndent_Off));
}

//~ NOTE(rjf): Tick

function void
F4_Tick(Application_Links *app, Frame_Info frame_info)
{
    linalloc_clear(&global_frame_arena);
    global_tooltip_count = 0;
    
    F4_TickColors(app, frame_info);
    F4_Index_Tick(app);
    F4_CLC_Tick(frame_info);
    F4_PowerMode_Tick(app, frame_info);
    F4_UpdateFlashes(app, frame_info);
    
    // NOTE(rjf): Default tick stuff from the 4th dimension:
    default_tick(app, frame_info);
}

//~ NOTE(rjf): Whole Screen Render Hook

function void
F4_WholeScreenRender(Application_Links *app, Frame_Info frame_info)
{
    F4_PowerMode_RenderWholeScreen(app, frame_info);
}
