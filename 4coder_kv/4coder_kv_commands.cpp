/* NOTE(kv): This file is for miscellaneous commands */

#include "4coder_kv_core.cpp"

Table_u64_u64 shifted_version_of_characters;

VIM_COMMAND_SIG(kv_shift_character)
{
  GET_VIEW_AND_BUFFER;
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

CUSTOM_COMMAND_SIG(vim_goto_definition_other_panel)
{
  vim_push_jump(app, get_active_view(app, Access_ReadVisible));
  view_buffer_other_panel(app);
  jump_to_definition_at_cursor(app);
}

VIM_COMMAND_SIG(kv_newline_above)
{
  vim_newline_above(app);
  vim_normal_mode(app);
}

VIM_COMMAND_SIG(kv_newline_below)
{
  vim_newline_below(app);
  vim_normal_mode(app);
}

CUSTOM_COMMAND_SIG(byp_reset_face_size)
CUSTOM_DOC("Resets face size to default")
{
  GET_VIEW_AND_BUFFER;
  Face_ID face_id = get_face_id(app, buffer);
  Face_Description description = get_face_description(app, face_id);
  description.parameters.pt_size = (i32)def_get_config_u64(app, vars_save_string_lit("default_font_size"));
  try_modify_face(app, face_id, &description);
}

CUSTOM_COMMAND_SIG(kv_profile_disable_and_inspect)
CUSTOM_DOC("disable and inspect profile")
{
  profile_disable(app);
  profile_inspect(app);
 }

inline b32 kv_is_group_opener(Token *token)
{
  return (token->kind == TokenBaseKind_ParentheticalOpen ||
          token->kind == TokenBaseKind_ScopeOpen);
}

inline b32 kv_is_group_closer(Token *token)
{
  return (token->kind == TokenBaseKind_ParentheticalClose ||
          token->kind == TokenBaseKind_ScopeClose);
}

inline b8 kv_is_group_opener(u8 c)
{
  switch (c) {
    case '(': case '[': case '{': return true;
    default:                      return false;
  }
}

inline b8 kv_is_group_closer(u8 c)
{
  switch (c) {
    case ')': case ']': case '}': return true;
    default:                      return false;
  }
}

void kv_vim_bounce(Application_Links *app)
{
  GET_VIEW_AND_BUFFER;
  Vim_Motion_Block vim_motion_block(app);
  i64 pos = view_get_cursor_pos(app, view);
  pos = vim_scan_bounce(app, buffer, pos, Scan_Forward);
  view_set_cursor_and_preferred_x(app, view, seek_pos(pos));
}

inline void kv_move_to_token(Application_Links *app, Token *token)
{
  View_ID   view = get_active_view(app, Access_ReadVisible);
  view_set_cursor_and_preferred_x(app, view, seek_pos(token->pos));
}

internal Token *
kv_token_at_cursor(Application_Links *app, i64 delta=0)
{
  GET_VIEW_AND_BUFFER;
  Token_Array tokens = get_token_array_from_buffer(app, buffer);
  i64 pos = view_correct_cursor(app, view) + delta;
  return token_from_pos(&tokens, pos);
}

internal Token_Iterator_Array
kv_token_it_at_cursor(Application_Links *app, i64 delta=0)
{
  GET_VIEW_AND_BUFFER;
  Token_Array tokens = get_token_array_from_buffer(app, buffer);
  i64 pos = view_correct_cursor(app, view) + delta;
  return token_iterator_pos(0, &tokens, pos);
}

VIM_COMMAND_SIG(kv_sexpr_up)
{
  GET_VIEW_AND_BUFFER;
  vim_push_jump(app, view);
  Vim_Motion_Block vim_motion_block(app);
  Token_Array tokens = get_token_array_from_buffer(app, buffer);

  i64 pos = view_correct_cursor(app, view);
  Token_Iterator_Array token_it = token_iterator_pos(0, &tokens, pos);
  i32 nest_level = 0;
  while (token_it_dec(&token_it))
  {
    Token *token = token_it_read(&token_it);
    if (kv_is_group_opener(token))
    {
      if (nest_level == 0) {
        kv_move_to_token(app, token);
        break;
      }
      else {
        soft_assert(nest_level > 0);
        nest_level--;
      }
    }
    else if (kv_is_group_closer(token))  nest_level++; 
  }
}

VIM_COMMAND_SIG(kv_sexpr_down)
{
  GET_VIEW_AND_BUFFER;
  Token_Iterator_Array token_it = kv_token_it_at_cursor(app);
  do
  {
    Token *token = token_it_read(&token_it);
    if (kv_is_group_opener(token))
    {
      kv_move_to_token(app, token);
      move_right(app);
      break;
    }
  } while (token_it_inc(&token_it));
}

VIM_COMMAND_SIG(kv_sexpr_right)
{
  GET_VIEW_AND_BUFFER;
  Token_Iterator_Array token_it = kv_token_it_at_cursor(app);
  do {
    Token *token = token_it_read(&token_it);
    if (kv_is_group_opener(token))
    {
      kv_move_to_token(app, token);
      kv_vim_bounce(app);
      move_right(app);
      break;
    }
    else if (kv_is_group_closer(token))
    {
      kv_move_to_token(app, token);
      move_left(app);
      break;
    }
  } while (token_it_inc(&token_it));
}

VIM_COMMAND_SIG(kv_sexpr_left)
{
  GET_VIEW_AND_BUFFER;
  Token_Iterator_Array token_it = kv_token_it_at_cursor(app, -1);
  do
  {
    Token *token = token_it_read(&token_it);
    if (kv_is_group_opener(token))
    {
      kv_move_to_token(app, token);
      move_right(app);
      break;
    }
    else if (kv_is_group_closer(token))
    {
      kv_move_to_token(app, token);
      kv_vim_bounce(app);
      break;
    }
  } while (token_it_dec(&token_it));
}

VIM_COMMAND_SIG(kv_sexpr_end)
{
  kv_sexpr_up(app);
  kv_sexpr_right(app);
  move_left(app);
  move_left(app);
}

