/*
  Model definition for an skm lexer.
  todo(kv): I still don't understand it, like wth is "fallback"?
*/

#define LANG_NAME_LOWER skm
#define LANG_NAME_CAMEL Skm

#include "lexer_generator/4coder_lex_gen_main.cpp"
#define KV_IMPLEMENTATION
#include "kv.h"

internal void
build_language_model(void)
{
    u8 utf8[129];
    smh_utf8_fill(utf8);
    
    smh_set_base_character_names();
    smh_typical_tokens();
    
    sm_select_base_kind(TokenBaseKind_Comment);
    sm_direct_token_kind("Text");
    
    // skm Operators
    Operator_Set *main_ops = sm_begin_op_set();
    //
    sm_select_base_kind(TokenBaseKind_ParentheticalOpen);
    sm_op("(");
    sm_op("[");
    sm_op("{");
    sm_select_base_kind(TokenBaseKind_ParentheticalClose);
    sm_op(")");
    sm_op("]");
    sm_op("}");
    
    // State Machine
    State *root = sm_begin_state_machine();
    
#define AddState(N) State *N = sm_add_state(#N)
    
    AddState(text);
    AddState(whitespace);
    
    ////
    
    sm_select_state(root);
    
    {// EOF
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("EOF");
        sm_case_eof(emit);
    }
    
    // Text
    u8 *text_chars = 0;
    for (i32 character=0; character < 128; character++) {
      if (isprint(character)) {
        switch (character) {
          case 0:
          case '(':
          case '[':
          case '{':
          case ')':
          case ']':
          case '}':
          case ' ':
          case '\r':
          case '\t':
          case '\f':
          case '\v':
          case '\n':
          {}break;

          default:
            arrput(text_chars, character);
        }
      }
    }
    arrput(text_chars, 0);
    // printf("text_chars: %s\n", text_chars);

    sm_case(text_chars, text);
    sm_case(utf8, text);
    
    // Whitespace
    sm_case(" \r\t\f\v\n", whitespace);

    {// Operators
        Character_Set *op_char_set = smo_new_char_set();
        smo_char_set_union_ops_firsts(op_char_set, main_ops);
        char *char_set_array = smo_char_set_get_array(op_char_set);
        State *operator_state = smo_op_set_lexer_root(main_ops, root, "LexError");
        sm_case_peek(char_set_array, operator_state);
    }
    
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Text");
        sm_fallback(emit);
    }
    
    ////
    
    sm_select_state(text);
    sm_case(text_chars, text);
    sm_case(utf8, text);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Text");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(whitespace);
    sm_case(" \t\r\f\v\n", whitespace);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Whitespace");
        sm_fallback_peek(emit);
    }
}
