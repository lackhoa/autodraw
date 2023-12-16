#pragma once

#include "kv-utils.h"

enum TokenKind
{
  Token_Colon   = ':',
  // 0-255 reserved for single-char ASCII types.

  Token_Special = 256,
  Token_Int,
  Token_Identifier,
  Token_DoubleDot,
  Token_Ellipsis,
  Token_DoubleDash,
  Token_StringLiteral,
  Token_DoubleColon,
  Token_ColonEqual,
  Token_Arrow,
  Token_StrongArrow,  // NOTE: strong arrow is used for lambda, might as well use it as a tactic.

  Token_Keyword_START,
  Token_Keyword_func,

  Token_Directive,
};

const char *language_keywords[] = {
  "", "func",
};

struct Token
{
  String    string;
  i32       line;
  i32       column;
  TokenKind kind;
  u64       val;
  operator String() { return string; };
};

struct ErrorAttachment { char *key; String value; };

struct InterpContext { String first; InterpContext *next; b32 is_important; };

struct InterpError
{
  String         message;
  i32            line;
  i32            column;
  InterpContext *context;
  b32            goal_attached;

  i32             attachment_count;
  ErrorAttachment attachments[16];
};

// todo: atm the tokenizer also doubles as our error tracker, which is annoying
// to think about because errors also come from the typechecker.
struct Tokenizer
{
  InterpError   *error;
  InterpContext *context;

  char  *at;
  Token  last_token;
  i32    line;
  i32    column;

  String directory;
};
