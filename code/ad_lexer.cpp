#include "ad_lexer.h"
#include "ad_globals.h"

// todo: I don't love that this is a pointer
global_variable Tokenizer *TK;
global_variable String *interned_strings;

inline Token
newToken(String text)
{
  Token out;
  out.string = text;
  out.line   = 0;
  out.column = 0;
  out.kind   = Token_Identifier;
  return out;
}

inline Token
newToken(const char *text)
{
  return newToken(toString(text));
}

inline char
getMatchingPair(Token *opening)
{
  switch (opening->string.chars[0])
  {
    case '(':
        return ')';
    case '[':
        return ']';
    case '{':
        return '}';
    default:
        return 0;
  }
}

inline b32
equal(Token *token, const char *string)
{
    return equal(token->string, string);
}

inline b32
equal(Token *token, char c)
{
    return ((token->string.length == 1) &&
            (token->string.chars[0] == c));
}

inline b32
isAlphaNumeric(char c)
{
  return ((('a' <= c) && (c <= 'z'))
          || (('A' <= c) && (c <= 'Z'))
          || (('0' <= c) && (c <= '9'))
          || (c == '\'') || (c == '_'));
}

inline b32
isSpecial(char c)
{
  switch (c)
  {
    case '`':
    case '/':
    case '\\':
    case '?':
    case '<':
    case '>':
    case '!':
    case '~':
    case '@':
    case '#':
    case '^':
    case '&':
    case '|':
    case '*':
    case '-':
    case '+':
    case '=':
        return true;

    default:
        return false;
  }
}

inline void
printCharToBufferRepeat(char *buffer, char c, i32 repeat)
{
  for (i32 index = 0 ;
       index < repeat;
       index++)
  {
    buffer[index] = c;
  }
  buffer[repeat] = 0;
}

inline void
pushContext(String string, b32 is_important)
{
  InterpContext *context = pushStruct(top_level_arena, InterpContext);
  context->first        = string;
  context->is_important = is_important;
  context->next         = TK->context;
  TK->context = context;
}

inline void
pushContext(char *string, b32 is_important=false)
{
  pushContext(toString(string), is_important);
}

inline void
pushContext(const char *string, b32 is_important=false)
{
  pushContext(toString(string), is_important);
}

inline void
popContext()
{
  TK->context = TK->context->next;
}

inline b32
hasMore()
{
  return (*TK->at != 0 && !TK->error);
}

InterpError SILENT_ERROR;

inline void
wipeError()
{
  resetArena(error_buffer);
  TK->error = 0;
}

inline b32
noError()
{
  return !TK->error;
}

inline InterpError *
getError()
{
  return TK->error;
}

inline InterpError *
hasError()
{
  return TK->error;
}

inline void
silentError()
{
  TK->error = &SILENT_ERROR;
}

inline b32
hasSilentError()
{
  return TK->error == &SILENT_ERROR;
}

inline char
nextChar()
{
  char out;
  if (*TK->at) {
    out = *TK->at++;
    if (out == '\n')
    {
      TK->line++;
      TK->column = 1;
    }
    else
      TK->column++;
  }
  else {
    out = 0;
  }
  return out;
}

internal void
eatAllSpaces(Tokenizer &tk = *TK)
{
  b32 stop = false;
  while ((*tk.at) && (!stop))
  {
    switch (*tk.at)
    {
      case '\n':
      case '\t':
      case ' ':
      {
        nextChar();
      } break;

      case ';':
      {
        if (*(tk.at+1) == ';')
        {
          nextChar();
          nextChar();
          while ((*tk.at) && (*tk.at != '\n'))
            nextChar();
        }
        else
        {
          stop = true;
        }
      } break;

      default:
          stop = true;
    }
  }
}

inline void
initTokenizer(Tokenizer &tk, char *input)
{
  tk.line      = 1;
  tk.column    = 1;
  tk.at        = input;
  if (input) {
    eatAllSpaces(tk);
  }
}

internal void
reportErrorVA(i32 line, i32 column, char *format, va_list arg_list)
{
  auto tk = TK;
  assert(!tk->error);  // NOTE: check that the parser doesn't do useless work after failure.

  InterpContext *context = 0;
  for (InterpContext *it = tk->context; it; it=it->next)
  {
    // note: we reverse the context list here, which is convenient for printing.
    // todo #speed don't do this in here! Since we sometimes recover from error.
    InterpContext *new_context = pushCopy(top_level_arena, it);
    new_context->next = context;
    context = new_context;
  }

  tk->error = pushStruct(top_level_arena, InterpError, true);
  tk->error->message = printVA(top_level_arena, format, arg_list);
  tk->error->line    = line;
  tk->error->column  = column;
  tk->error->context = context;
}

internal void
reportError(Token &token, char *format, ...)
{
  va_list arg_list;
  va_start(arg_list, format);
  reportErrorVA(token.line, token.column, format, arg_list);
  va_end(arg_list);
}

// todo cleanup always use the global tokenizer, so we can get rid of this function
internal void
tokenError(Token &token, char *message)
{
  reportError(token, "%s", message);
}

internal void
tokenError(char *message)
{
  tokenError(TK->last_token, message);
}

internal void
reportError(char *message)
{
  reportError(TK->last_token, message);
}

b32
isDigit(char c)
{
  return (('0' <= c) && (c <= '9'));
}

String
internString(String input)
{
  for (i32 i=0; i < bufLength(interned_strings); i++) {
    if (equal(input, interned_strings[i])) {
      return interned_strings[i];
    }
  }

  String new_string = {};
  new_string.length = input.length;
  new_string.chars  = (char *)xmalloc(input.length+1);
  copyMemory_(input.chars, new_string.chars, input.length);
  new_string.chars[input.length] = 0;  // null-terminated
  bufPush(interned_strings, new_string);
  return new_string;
}

inline String
internString(char *cstring)
{
  String string = toString(cstring);
  return internString(string);
}

inline String
internStringRange(char *start, char *end)
{
  i32 length = end - start;
  String string = String{start, length};
  return internString(string);
}

// todo #speed calling nextChar() a lot
forward_declare("generated_ad_lexer.h") Token *
eatToken()
{
  Tokenizer &tk = *TK;
  Token token = {};
  token.string.chars = tk.at;
  token.line         = tk.line;
  token.column       = tk.column;

  char first_char = nextChar();
  char second_char = *tk.at;
  switch (first_char)
  {
    case '/': {
      // C++ single-line comment
      if (second_char == '/') {
        while (nextChar() != '\n') {
          // Skip comment line
        }
      } else {
        
      }
      break;
    }

    case '#':
    {
      token.string.chars++; // advance past the hash
      token.kind = Token_Directive;
      while (isAlphaNumeric(*tk.at))
        nextChar();
    } break;

    case '"':
    {
      token.string.chars++; // advance past the opening double quote
      token.kind = Token_StringLiteral;
      while (*tk.at != '"')
        nextChar();
      // handle the closing double quote
      nextChar();
      token.string.length = (i32)(tk.at - token.string.chars - 1);
    } break;

    case '.':
    {
      if ((*tk.at == '.') && (*(tk.at+1) == '.'))
      {
        nextChar();
        nextChar();
        token.kind = Token_Ellipsis;
      }
      else if (*tk.at == '.')
      {
        nextChar();
        token.kind = Token_DoubleDot;
      }
      else
        token.kind = (TokenKind)'.';
    } break;

    case '=':
    {
      switch (*tk.at)
      {
        case '>':
        {
          token.kind = Token_StrongArrow;
          nextChar();
        } break;

        default:
        {
          token.kind = Token_Special;
          while (isSpecial(*tk.at))
            nextChar();
        } break;
      }
    } break;

    case '-':
    {
      switch (*tk.at)
      {
        case '-':
        {
          token.kind = Token_DoubleDash;
          nextChar();
        } break;

        case '>':
        {
          token.kind = Token_Arrow;
          nextChar();
        } break;

        default:
        {
          token.kind = Token_Special;
          while (isSpecial(*tk.at))
            nextChar();
        } break;
      }
    } break;

    case ':':
    {
      switch (*tk.at)
      {
        case ':':
        {
          token.kind = Token_DoubleColon;
          nextChar();
        } break;

        case '=':
        {
          token.kind = Token_ColonEqual;
          nextChar();
        } break;

        default:
        {
          token.kind = (TokenKind)':';
        } break;
      }
    } break;

    case '0'...'9':
    {
      token.val = first_char - '0';
      while (isDigit(*tk.at)) {
        token.val = token.val*10 + (*tk.at - '0');
        nextChar();
      }
      token.kind = Token_Int;
      break;
    }

    // for char in range(ord('a'), ord('z')+1):
    //     print(f"case '{chr(char)}':")
    case 'A'...'Z':
    case 'a'...'z':
    case '_':
    {
      while (isAlphaNumeric(*tk.at)) {
        nextChar();
      }
      token.kind = Token_Identifier;
      token.string = internStringRange(token.string.chars, tk.at);
      break;
    }

    default:
    {
      if (isSpecial(first_char))
      {
        token.kind = Token_Special;
        while (isSpecial(*tk.at)) {
          nextChar();
        }
      }
      else {
        token.kind = (TokenKind)first_char;
      }
      break;
    }
  }

  if (token.kind)
  {
    if (!token.string.length)
      token.string.length = (i32)(tk.at - token.string.chars);
    assert(token.string.length > 0);

    switch (token.kind)
    {
      case Token_Identifier:
      {
        // todo: lookup keywords with hash table
        for (int id = 1; id < arrayCount(language_keywords); id++)
        {
          if (equal(token.string, language_keywords[id]))
          {
            token.kind = (TokenKind)((int)Token_Keyword_START + id);
            break;
          }
        }
      } break;

      default: {};
    }
  }

  tk.last_token = token;
  // NOTE: :always-eat-spaces We eat spaces afterward, so that we can always
  // check *tk->at to see if there's anything left to parse.
  eatAllSpaces();
  return &tk.last_token;
}

// todo just return the token pointer!
forward_declare("generated_ad_lexer.h") inline Token
nextToken()
{
  eatToken();
  return TK->last_token;
}

forward_declare("generated_ad_lexer.h") inline Token
peekToken()
{
    auto tk_save = *TK;
    Token token = nextToken();
    *TK = tk_save;
    return token;
}

inline char
peekChar()
{
  return *TK->at;  // :always-eat-spaces
}

inline b32
eatUntil(char c)
{
  b32 found = false;
  while (*TK->at && !found)
  {
    if (*TK->at == c)
      found = true;
    nextToken();
  }
  return found;
}

inline b32
inString(char *string, Token &token)
{
    if (token.string.length == 1)
    {
        char character = token.string.chars[0];
        for (char *c = string; *c; c++)
        {
            if (*c == character)
                return true;
        }
    }
    return false;
}

// todo: Better hash function!
internal u32
stringHash(String string)
{
    u32 out = 0;
    for (int i = 0; i < string.length; i++)
    {
        out = 65599*out + string.chars[i];
    }
    return out;
}

inline b32
isIdentifier(Token &token)
{
    return ((token.kind == Token_Identifier)
            || (token.kind == Token_Special));
}

inline b32
eatUntilMatchingPair()
{
  auto tk = TK;
  b32 found = false;
  Token opening = tk->last_token;
  char  closing = getMatchingPair(&opening);
  assert(closing);
  for (; !found && hasMore(); )
  {
    Token token = nextToken();
    if (getMatchingPair(&token))
      eatUntilMatchingPair();

    else if (equal(&token, closing))
      found = true;
  }

  if (noError() && !found)
  {
    reportError(opening, "could not find matching pair for");
  }

  return found;
}

inline Token &
lastToken()
{
  return TK->last_token;
}

inline String
lastString()
{
  return TK->last_token.string;
}

inline i32
toInt32(Token &token)
{
  i32 out = 0;
  String string = token.string;
  i32 length = string.length;
  for (int char_i=0;
       noError() && char_i < length;
       char_i++)
  {
    char c = string.chars[char_i];
    if ('0' <= c && c <= '9')
    {
      out = out*10 + (c - '0');
    }
    else
    {
      tokenError("expected a 32-bit integer");
    }
  }
  return out;
}

inline i32
parseInt32()
{
  eatToken();
  i32 out = toInt32(lastToken());
  return out;
}
