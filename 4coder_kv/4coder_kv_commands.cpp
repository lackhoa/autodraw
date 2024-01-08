/* NOTE(kv): This file is for miscellaneous commands */

#include "4coder_kv_utils.cpp"
#include "4coder_kv_input.cpp"

Table_u64_u64 shifted_version_of_characters;

VIM_COMMAND_SIG(kv_shift_character)
{
  GET_VIEW_AND_BUFFER;
  i64 pos = view_get_cursor_pos(app, view);

  u8 current_character = 0;
  buffer_read_range(app, buffer, Ii64(pos, pos+1), &current_character);

  u64 replacement_char = 0;
  if (character_is_upper(current_character))
  {
    replacement_char = character_to_lower(current_character);
  }
  else if (character_is_lower(current_character))
  {
    replacement_char = character_to_upper(current_character);
  }
  else
  {
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


inline void byp_make_vim_request(Application_Links *app, BYP_Vim_Request request){
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

inline u8 kv_is_group_opener(u8 c)
{
  switch (c) {
    case '(':
      return ')';
    case '[':
      return ']';
    case '{':
      return '}';
    case '\"':
      return '\"';
    case '\'':
      return '\'';
    default:   
      return 0;
  }
}

inline u8 kv_is_group_closer(u8 c)
{
  switch (c) {
    case ')':
      return '(';
    case ']':
      return '[';
    case '}':
      return '{';
    case '\"':
      return '\"';
    case '\'':
      return '\'';
    default:   
      return 0;
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

// todo(kv): support fallback path that works on characters
function b32
kv_find_surrounding_nest(Application_Links *app, Buffer_ID buffer, i64 pos, Range_i64 *out)
{
    b32 result = false;
    Range_i64 range = {};
    Find_Nest_Flag flags = FindNest_Scope | FindNest_Paren | FindNest_Balanced;
    if (find_nest_side(app, buffer, pos-1, flags,
                       Scan_Backward, NestDelim_Open, &range.start) &&
        find_nest_side(app, buffer, pos, flags|FindNest_EndOfToken,
                       Scan_Forward, NestDelim_Close, &range.end))
    {
        *out = range;
        result = true;
    }
    return(result);
}

VIM_COMMAND_SIG(kv_sexpr_up)
{
  GET_VIEW_AND_BUFFER;
  vim_push_jump(app, view);
  Vim_Motion_Block vim_motion_block(app);
  Token_Array tokens = get_token_array_from_buffer(app, buffer);

  i64 pos = view_correct_cursor(app, view);

  Range_i64 nest = {};
  if ( kv_find_surrounding_nest(app, buffer, pos, &nest) )
  {
    view_set_cursor_and_preferred_x(app, view, seek_pos(nest.min));
  }
}

VIM_COMMAND_SIG(kv_sexpr_down)
{
  GET_VIEW_AND_BUFFER;
  vim_push_jump(app, view);
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
  Token *token = token_it_read(&token_it);
  if (!token) return;
  do
  {
    Token *token = token_it_read(&token_it);
    if (kv_is_group_opener(token))
    {
      kv_move_to_token(app, token);
      // todo(kv): we can do "find_nest_side" here
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
  Token *token = token_it_read(&token_it);
  if (!token) return;
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

void kv_surround_with(Application_Links *app, char *opener, char *closer)
{
  GET_VIEW_AND_BUFFER;
  HISTORY_MERGE_SCOPE;

  i64 min = view_get_cursor_pos(app, view);
  i64 max = view_get_mark_pos(app, view);
  if (max < min) SWAP(min, max);
  max += 1;

  buffer_replace_range(app, buffer, Ii64(max), SCu8(closer));
  buffer_replace_range(app, buffer, Ii64(min), SCu8(opener));

  vim_normal_mode(app);
}

CUSTOM_COMMAND_SIG(kv_reopen_with_confirmation)
CUSTOM_DOC("Like reopen, but asks for confirmation")
{
    Query_Bar_Group group(app);
    Query_Bar bar = {};
    bar.prompt = SCu8("Reload current buffer from disk?");
    if (start_query_bar(app, & bar, 0))
    {
      b32 cancelled = false;
      for (;!cancelled;){
        User_Input in = get_next_input(app, EventProperty_AnyKey, 0);
        if (in.abort){
          cancelled = true;
        }
        else{
          switch (in.event.key.code){
            case KeyCode_Y:
            {
              View_ID view = get_active_view(app, Access_ReadVisible);
              Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
              buffer_reopen(app, buffer, 0);
              cancelled = true;
            }break;
                        
            case KeyCode_Shift:
            case KeyCode_Control:
            case KeyCode_Alt:
            case KeyCode_Command:
            case KeyCode_CapsLock:
            {}break;
                        
            default:
            {
              cancelled = true;
            }break;
          }
        }
      }
    }
}

VIM_COMMAND_SIG(kv_surround_paren)          {kv_surround_with(app, "(", ")");}
VIM_COMMAND_SIG(kv_surround_paren_spaced)   {kv_surround_with(app, "( ", " )");}
VIM_COMMAND_SIG(kv_surround_bracket)        {kv_surround_with(app, "[", "]");}
VIM_COMMAND_SIG(kv_surround_bracket_spaced) {kv_surround_with(app, "[ ", " ]");}
VIM_COMMAND_SIG(kv_surround_brace)          {kv_surround_with(app, "{", "}");}
VIM_COMMAND_SIG(kv_surround_brace_spaced)   {kv_surround_with(app, "{ ", " }");}
VIM_COMMAND_SIG(kv_surround_double_quote)   {kv_surround_with(app, "\"", "\"");}

VIM_COMMAND_SIG(kv_void_command) { return; }

VIM_COMMAND_SIG(kv_vim_normal_mode)
{
  vim_normal_mode(app);
  arrsetlen(kv_quail_keystroke_buffer, 0);
}

VIM_COMMAND_SIG(kv_sexpr_select_whole)
{
  GET_VIEW_AND_BUFFER;

  Range_i64 nest = {};
  b32 result = false;

  i64 pos = view_get_cursor_pos(app, view);
  u8 current_char = buffer_get_char(app, buffer, pos);
  if ( kv_is_group_opener(current_char) )
    pos++;

  result = kv_find_surrounding_nest(app, buffer, pos, &nest);
  nest.max--;
  if (result)
  {
    view_set_cursor_and_preferred_x(app, view, seek_pos(nest.min));
    view_set_mark(app, view, seek_pos(nest.max));
    vim_state.mode = VIM_Visual;
    vim_state.params.edit_type = EDIT_CharWise;
  }
}

inline b32
character_is_path(char character)
{
  switch (character)
  {
    case '/': case '~': case '.': case '\\':  case '-':
      return true;
    default:
      return character_is_alpha_numeric(character);
  }
}

// todo: support relative path maybe
CUSTOM_COMMAND_SIG(kv_open_file_ultimate)
CUSTOM_DOC("The one-stop-shop for all your file-opening need")
{
  GET_VIEW_AND_BUFFER;

  Scratch_Block scratch(app);

  i64 curpos = view_get_cursor_pos(app, view);

  i64 buffer_size = buffer_get_size(app, buffer);
  i64 min = curpos;
  i64 max = curpos;
  //
  for (; max < buffer_size; max++)
  {
    u8 character = 0;
    buffer_read_range(app, buffer, Ii64(max, max+1), &character);
    if (!character_is_path(character)) { break; }
  }
  for (; min >= 0; min--)
  {
    u8 character = 0;
    buffer_read_range(app, buffer, Ii64(min, min+1), &character);
    if (!character_is_path(character)) { min++; break; }
  }

  b32 looking_at_path = false;
  if (max > min)
  {
    String_Const_u8 path = push_buffer_range(app, scratch, buffer, Range_i64{min, max});
    printf_message(app, scratch, "the path is: %.*s\n", string_expand(path));
    if (view_open_file(app, view, path, true))
    {
      looking_at_path = true;
    }
  }

  if (!looking_at_path)
  {
    vim_interactive_open_or_new(app);
  }
}

VIM_COMMAND_SIG(kv_delete_surrounding_groupers)
{
  GET_VIEW_AND_BUFFER;
  HISTORY_MERGE_SCOPE;

  i64 pos = view_get_cursor_pos(app, view);
  Range_i64 range = {};
  if (kv_find_surrounding_nest(app, buffer, pos, &range))
  {
    kv_buffer_delete_pos(app, buffer, range.max-1);
    kv_buffer_delete_pos(app, buffer, range.max-1);
  }
}

function void kv_do_t_internal(Application_Links *app, b32 shiftp)
{
  GET_VIEW_AND_BUFFER;
  HISTORY_MERGE_SCOPE;

  i64 pos = view_get_cursor_pos(app, view);
  u8 current_char = buffer_get_char(app, buffer, pos);
  // 1. optionally delete space
  if (current_char == ' ')
  {
    kv_buffer_delete_pos(app, buffer, pos);
    current_char = buffer_get_char(app, buffer, pos);
  }
  else if (current_char == '_')
  {
    pos++;
    current_char = buffer_get_char(app, buffer, pos);
  }

  // 2. upcase character/word
  if (shiftp)
  {
    Scratch_Block temp(app);

    i64 max = scan_any_boundary(app, boundary_alpha_numeric, buffer, Scan_Forward, pos);
    String_Const_u8 string = push_buffer_range(app, temp, buffer, Range_i64{pos, max});
    string_mod_upper(string);

    kv_buffer_replace_range(app, buffer, pos, max, string);
  }
  else
  {
    u8 next_char = buffer_get_char(app, buffer, pos);
    u8 upper = character_to_upper(current_char);  // todo(kv): I hope the string is copied
    kv_buffer_replace_range(app, buffer, pos, pos+1, SCu8(&upper, 1));
  }

  // 3. move
  move_right_alpha_numeric_boundary(app);
}

VIM_COMMAND_SIG(kv_do_t) {kv_do_t_internal(app, false);}
VIM_COMMAND_SIG(kv_do_T) {kv_do_t_internal(app, true);}

CUSTOM_COMMAND_SIG(kv_run)
CUSTOM_DOC("run the current script")
{
  GET_VIEW_AND_BUFFER;
  Scratch_Block temp(app);

  String_Const_u8 dir = push_hot_directory(app, temp);
  String_Const_u8 cmd = push_buffer_file_name(app, temp, buffer);
  standard_build_exec_command(app, view, dir, cmd);
}

CUSTOM_COMMAND_SIG(note)
CUSTOM_DOC("run the current script")
{
  View_ID   view = get_active_view(app, Access_ReadVisible);
  Buffer_ID buffer = create_buffer(app, SCu8("~/notes/note.skm"), 0);
  view_set_buffer(app, view, buffer, 0);
}
