
CUSTOM_COMMAND_SIG(kv_auto_indent_line_at_cursor)
CUSTOM_DOC("kv testing: Auto-indents the line on which the cursor sits.")
{
  View_ID view = get_active_view(app, Access_ReadWriteVisible);
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
  Range_i64 pos = Ii64(view_get_cursor_pos(app, view));

  i32 indent_width = 2;
  i32 tab_width    = 2;

  {
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    Token_Array *tokens = &token_array;

    // for_increment(token_index, 0, tokens->count)
    // {
    //   Token *token = tokens->tokens + token_index;
    //   if (0 <= token->kind < arrayCount(token_base_kind_names))
    //   {
    //     char *token_name = token_base_kind_names[token->kind];
    //     print_message_cstr(app, token_name);
    //     print_message_cstr(app, "\n");
    //   }
    // }

    if ( tokens->tokens )
    {
      Scratch_Block scratch(app);
      i32 safety_counter = 0;
      for (;;)
      {
        Range_i64 expanded = enclose_tokens(app, buffer, pos);
        expanded = enclose_whole_lines(app, buffer, expanded);
        if (expanded == pos)
        {
          break;
        }
        pos = expanded;
        safety_counter += 1;
        if (safety_counter == 20) {pos = buffer_range(app, buffer); break;}
      }
      Range_i64  lines = get_line_range_from_pos_range(app, buffer, pos);

      Indent_Flag flags = 0;
      i64 *indentations = 0;
      {
        Arena *arena = scratch;
        i64 count = lines.max - lines.min + 1;
        indentations = push_array(arena, i64, count);
        i64 *shifted_indentations = indentations - lines.first;
        block_fill_u64(indentations, sizeof(*indentations)*count, (u64)(-1));

        i64 anchor_line = clamp_bot(1, lines.first - 1);
        Token *anchor_token = find_anchor_token(app, buffer, tokens, anchor_line);
        if (anchor_token != 0 &&
            anchor_token >= tokens->tokens &&
            anchor_token < tokens->tokens + tokens->count)
        {
          i64 line = get_line_number_from_pos(app, buffer, anchor_token->pos);
          line = clamp_top(line, lines.first);

          Token_Iterator_Array token_it = token_iterator(0, tokens, anchor_token);

          Scratch_Block scratch(app, arena);
          Nest *nest = 0;
          Nest_Alloc nest_alloc = {};

          i64 line_last_indented = line - 1;
          i64 last_indent = 0;
          i64 actual_indent = 0;
          b32 in_unfinished_statement = false;

          Indent_Line_Cache line_cache = {};

          for (;;)
          {
            Token *token = token_it_read(&token_it);

            if (line_cache.where_token_starts == 0 ||
                token->pos >= line_cache.one_past_last_pos)
            {
              ProfileScope(app, "get line number");
              line_cache.where_token_starts = get_line_number_from_pos(app, buffer, token->pos);
              line_cache.one_past_last_pos = get_line_end_pos(app, buffer, line_cache.where_token_starts);
            }

            i64 current_indent = 0;
            if (nest != 0){
              current_indent = nest->indent;
            }
            i64 this_indent = current_indent;
            i64 following_indent = current_indent;

            b32 shift_by_actual_indent = false;
            b32 ignore_unfinished_statement = false;
            if (HasFlag(token->flags, TokenBaseFlag_PreprocessorBody))
            {
              this_indent = 0;
            }
            else
            {
              switch (token->kind)
              {
                case TokenBaseKind_ScopeOpen:
                {
                  Nest *new_nest = indent__new_nest(arena, &nest_alloc);
                  sll_stack_push(nest, new_nest);
                  nest->kind = TokenBaseKind_ScopeOpen;
                  nest->indent = current_indent + indent_width;
                  following_indent = nest->indent;
                  ignore_unfinished_statement = true;
                }break;

                case TokenBaseKind_ScopeClose:
                {
                  for (;nest != 0 && nest->kind != TokenBaseKind_ScopeOpen;){
                    Nest *n = nest;
                    sll_stack_pop(nest);
                    indent__free_nest(&nest_alloc, n);
                  }
                  if (nest != 0 && nest->kind == TokenBaseKind_ScopeOpen){
                    Nest *n = nest;
                    sll_stack_pop(nest);
                    indent__free_nest(&nest_alloc, n);
                  }
                  this_indent = 0;
                  if (nest != 0){
                    this_indent = nest->indent;
                  }
                  following_indent = this_indent;
                  ignore_unfinished_statement = true;
                }break;

                case TokenBaseKind_ParentheticalOpen:
                {
                  Nest *new_nest = indent__new_nest(arena, &nest_alloc);
                  sll_stack_push(nest, new_nest);
                  nest->kind = TokenBaseKind_ParentheticalOpen;
                  line_indent_cache_update(app, buffer, tab_width, &line_cache);
                  nest->indent = (token->pos - line_cache.indent_info.first_char_pos) + 1;
                  following_indent = nest->indent;
                  shift_by_actual_indent = true;
                }break;

                case TokenBaseKind_ParentheticalClose:
                {
                  if (nest != 0 && nest->kind == TokenBaseKind_ParentheticalOpen){
                    Nest *n = nest;
                    sll_stack_pop(nest);
                    indent__free_nest(&nest_alloc, n);
                  }
                  following_indent = 0;
                  if (nest != 0){
                    following_indent = nest->indent;
                  }
//ignore_unfinished_statement = true;
                }break;
              }

              if (token->sub_kind == TokenCppKind_BlockComment ||
                  token->sub_kind == TokenCppKind_LiteralStringRaw){
                ignore_unfinished_statement = true;
              }

              if (in_unfinished_statement && !ignore_unfinished_statement){
                this_indent += indent_width;
              }
            }

#define EMIT(N)                                                         \
//             Stmnt(if (lines.first <= line_it){shifted_indentations[line_it]=N;} \
//                   if (line_it == lines.end){goto finished;}             \
//                   actual_indent = N; )

            i64 line_it = line_last_indented;
            if (lines.first <= line_cache.where_token_starts){
              for (;line_it < line_cache.where_token_starts;){
                line_it += 1;
                if (line_it == line_cache.where_token_starts){
                  EMIT(this_indent);
                }
                else{
                  EMIT(last_indent);
                }
              }
            }
            else{
              actual_indent = this_indent;
              line_it = line_cache.where_token_starts;
            }

            i64 line_where_token_ends = get_line_number_from_pos(app, buffer, token->pos + token->size);
            if (lines.first <= line_where_token_ends){
              line_indent_cache_update(app, buffer, tab_width, &line_cache);
              i64 line_where_token_starts_shift = this_indent - line_cache.indent_info.indent_pos;
              for (;line_it < line_where_token_ends;){
                line_it += 1;
                i64 line_it_start_pos = get_line_start_pos(app, buffer, line_it);
                Indent_Info line_it_indent_info = get_indent_info_line_number_and_start(app, buffer, line_it, line_it_start_pos, tab_width);
                i64 new_indent = line_it_indent_info.indent_pos + line_where_token_starts_shift;
                new_indent = clamp_bot(0, new_indent);
                EMIT(new_indent);
              }
            }
            else{
              line_it = line_where_token_ends;
            }
#undef EMIT

            if (shift_by_actual_indent)
            {
              nest->indent += actual_indent;
              following_indent += actual_indent;
            }

            if (token->kind != TokenBaseKind_Comment)
            {
              in_unfinished_statement = indent__unfinished_statement(token, nest);
              if (in_unfinished_statement)
              {
                following_indent += indent_width;
              }
            }

            last_indent = following_indent;
            line_last_indented = line_it;

            if ( !token_it_inc_non_whitespace(&token_it) )
            {
              break;
            }
          }
        }

    finished:;
        indentations = indentations;
      }
      set_line_indents(app, scratch, buffer, lines, indentations, flags, tab_width);
    }
  }
}
