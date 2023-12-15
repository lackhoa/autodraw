#include "ad_parser.h"
#include "ad_lexer.cpp"

#define NULL_WHEN_ERROR(name) if (noError()) {assert(name);} else {name = {};}

void reportError(Ast &in, char *format, ...)
{
  va_list arg_list;
  va_start(arg_list, format);
  Token *token = &in.token;
  reportErrorVA(token->line, token->column, format, arg_list);
  va_end(arg_list);
}

// builtin expession end markers for now
inline b32
isExpressionEndMarker(Token &token)
{
  if (inString(")]}{,;:", token))
  {
    return true;
  }

  // Halt at all directives
  if (token.kind == Token_Directive)
  {
    return true;
  }

  switch (token.kind)
  {
    case Token_DoubleColon:
    case Token_ColonEqual:
    case Token_DoubleDash:
    case Token_StrongArrow:
    case Token_DoubleDot:
    {
      return true;
    }
    default: {}
  }

  return false;
}

inline b32
seesExpressionEndMarker()
{
  Token token = peekToken();  // todo: #speed I don't think we should have to peek anything here.
  return isExpressionEndMarker(token);
}

inline b32
requireChar(char c, char *reason=0)
{
  auto out = false;
  if (!reason)
    reason = "";
  if (hasMore())
  {
    Token token = nextToken();
    if (token.string.length == 1 && token.string.chars[0] == c)
      out = true;
    else
      reportError(token, "expected character '%c' (%s)", c, reason);
  }
  return out;
}

inline b32
requireKind(TokenKind tc, char *message=0)
{
  b32 out = false;
  if (hasMore())
  {
    if (!message) message = "token was of a different kind than expected";
    if (nextToken().kind == tc) out = true;
    else tokenError(message);
  }
  return out;
}

inline b32
requireIdentifier(char *message=0)
{
  b32 out = false;
  if (hasMore())
  {
    if (!message) message = "expected identifier";
    Token *token = eatToken();
    if (isIdentifier(*token))
      out = true;
    else
      tokenError(message);
  }
  return out;
}

inline Identifier *
parseIdentifier()
{
  Identifier *out = 0;
  if (requireIdentifier())
  {
    out = newAst(top_level_arena, Identifier, lastToken());
  }
  return out;
}


inline b32
optionalIdentifier()
{
  b32 out = false;
  if (hasMore())
  {
    Token token = peekToken();
    if (isIdentifier(token))
    {
      eatToken();
      out = true;
    }
  }
  return out;
}

inline b32
optionalKind(TokenKind tc)
{
  b32 out = false;
  if (hasMore())
    if (peekToken().kind == tc)
    {
      out = true;
      eatToken();
    }
  return out;
}

inline b32
optionalDirective(char *string)
{
  b32 out = false;
  if (hasMore())
  {
    Token token = peekToken();
    if (token.kind == Token_Directive)
    {
      if (equal(token.string, string))
      {
        out = true;
        eatToken();
      }
    }
  }
  return out;
}

inline b32
optionalChar(char c)
{
  b32 out = false;
  if (hasMore())
  {
    Token token = peekToken();
    if (equal(&token, c))
    {
      out = true;
      eatToken();
    }
  }
  return out;
}

inline void
eatCharRepeatedly(char c)
{
  while (hasMore())
  {
    if (!optionalChar(c)) break;
  }
}

inline b32
requireString(char *str)
{
  b32 out = false;
  if (hasMore())
  {
    eatToken();
    if (equal(lastToken(), str))
    {
      out = true;
    }
    else
    {
      reportError(lastToken(), "expected string %s", str);
    }
  }
  return out;
}

inline b32
optionalString(char *str)
{
  b32 out = false;
  if (hasMore())
  {
    Token token = peekToken();
    if (equal(&token, str))
    {
      out = true;
      nextToken();
    }
  }
  return out;
}

inline Ast **
getAstBody(Ast *item0)
{
  // todo #speed at least use a switch here!
  if (LetAst *let = castAst(item0, LetAst))
  {
    return &let->body;
  }
  if (RewriteAst *rewrite = castAst(item0, RewriteAst))
  {
    return &rewrite->body;
  }
  if (GoalTransform *item = castAst(item0, GoalTransform))
  {
    return &item->body;
  }
  if (Invert *item = castAst(item0, Invert))
  {
    return &item->body;
  }
  if (SubstAst *item = castAst(item0, SubstAst))
  {
    return &item->body;
  }
  if (AlgebraNormAst *item = castAst(item0, AlgebraNormAst))
  {
    return &item->body;
  }
  if (AliasAst *item = castAst(item0, AliasAst))
  {
    return &item->body;
  }
  if (LetEqAst *item = castAst(item0, LetEqAst))
  {
    return &item->body;
  }
  invalidCodePath;
  return 0;
}

inline ArrowAst *
parseNameOnlyArrowType()
{
  Arena &arena = top_level_arena;
  ArrowAst *out = 0;
  if (seesExpressionEndMarker())
  {
    breakhere; // return null explicitly
  }
  else
  {
    out = newAst(arena, ArrowAst, lastToken());
    i32 cap = 16;  // todo #grow
    allocateArray(arena, cap, out->param_names, true);
    for (;;)
    {
      i32 param_i = out->param_count++;
      assert(param_i < cap);
      if (optionalChar('_'))
      {
        breakhere; // anonymous parameter
      }
      else if (requireIdentifier())
      {
        out->param_names[param_i] = lastString();
      }
      if (!optionalChar(',')) break;
    }
    NULL_WHEN_ERROR(out);
  }
  return out;
}

internal ArrowAst *
parseArrowType(b32 skip_output_type)
{
  Arena &arena = top_level_arena;
  ArrowAst *out = 0;
  i32      param_count = 0;
  String  *param_names;
  Ast    **param_types;
  u32     *param_flags;
  Token marking_token = peekToken();
  char begin_arg_char = '(';
  char end_arg_char   = ')';
  if (requireChar(begin_arg_char))
  {
    i32 cap = 16;  // todo grow
    // :arrow-copy-done-later
    allocateArray(arena, cap, param_names, true);
    allocateArray(arena, cap, param_flags, true);
    allocateArray(arena, cap, param_types, true);

    i32 typeless_run = 0;
    Token typeless_token;
    for (b32 stop = false;
         !stop && hasMore();
         )
    {
      if (optionalChar(end_arg_char)) stop = true;
      else
      {
        i32 param_i = param_count++;
        assert(param_i < cap);

        if (optionalDirective("unused"))
        {
          setFlag(&param_flags[param_i], ParameterFlag_Unused);
        }

        for (;;)
        {
          if (optionalChar('$'))
          {
            setFlag(&param_flags[param_i], ParameterFlag_Inferred);
          }
          else if (optionalChar('^'))
          {
            setFlag(&param_flags[param_i], ParameterFlag_Poly);
          }
          else break;
        }

        Tokenizer tk_save = *TK;
        String param_name = {};
        Token maybe_param_name_token = nextToken();
        if (isIdentifier(maybe_param_name_token))
        {
          Token after_name = peekToken();
          if (equal(&after_name, ':'))
          {
            eatToken();
            param_name = maybe_param_name_token.string;
            pushContext("parameter type");
            if (Ast *param_type = parseExpression())
            {
              param_types[param_i] = param_type;
              if (typeless_run)
              {
                for (i32 offset = 1; offset <= typeless_run; offset++)
                {
                  param_types[param_i - offset] = param_type;
                }
                typeless_run = 0;
              }
            }
            popContext();
          }
          else if (equal(&after_name, ','))
          {
            param_name = maybe_param_name_token.string;
            typeless_run++;
            typeless_token = maybe_param_name_token;
          }
        }

        if (param_name.chars)
        {
          param_names[param_i] = param_name;
        }
        else
        {
          if (maybe_param_name_token.kind != ':')
          {
            *TK = tk_save;
          }
          pushContext("anonymous parameter");
          Token anonymous_parameter_token = TK->last_token;
          if (Ast *param_type = parseExpression())
          {
            param_types[param_i] = param_type;
            if (typeless_run)
            {
              tokenError(anonymous_parameter_token, "cannot follow a typeless parameter with an anonymous parameter");
            }
          }
          popContext();
        }

        if (hasMore())
        {
          Token delimiter = nextToken();
          if (equal(&delimiter, end_arg_char))
            stop = true;
          else if (!equal(&delimiter, ','))
            tokenError("unexpected token");
        }
      }
    }

    if (noError())
    {
      if (typeless_run)
      {
        tokenError(typeless_token, "please provide types for all parameters");
      }
    }
  }

  if (noError())
  {
    out = newAst(arena, ArrowAst, marking_token);
    out->param_count = param_count;
    out->param_names = param_names;
    out->param_types = param_types;
    out->param_flags = param_flags;

    if (optionalKind(Token_Arrow))
    {
      if (Ast *return_type = parseExpression())
        out->output_type = return_type;
    }
    else if (!skip_output_type)  // structs don't need return type
    {
      reportError("non-struct arrow types require an output type");
    }
  }

  NULL_WHEN_ERROR(out);
  return out;
}

internal Ast *
parseList(Arena &arena)
{
  ListAst *out = 0;
  // :list-opening-brace-eaten
  Token &first_token = lastToken();
  i32 count = 0;
  i32 cap = 16;  // todo #grow
  Ast **items = pushArray(arena, cap, Ast*);
  char closing = ']';
  Ast *tail = 0;
  for (; noError(); )
  {
    if (optionalChar(closing))
      break;

    items[count++] = parseExpression();

    if (!optionalChar(','))
    {
      if (optionalKind(Token_DoubleDot))
      {
        tail = parseExpression();
      }
      requireChar(closing);
      break;
    }
  }
  if (noError())
  {
    out = newAst(arena, ListAst, first_token);
    out->count = count;
    out->items = items;
    out->tail  = tail;
  }
  return out;
}

internal Ast *
parseFunctionExpression(Arena &arena)
{
  // cutnpaste from "parseGlobalFunction"
  FunctionAst *out = newAst(arena, FunctionAst, lastToken());

  ArrowAst *signature = 0;
  if (peekChar() == '{') {
    // inferred signature.
  }
  else if (peekChar() == '(') {
    signature = parseArrowType(false);
  }
  else {
    signature = parseNameOnlyArrowType();
  }

  NULL_WHEN_ERROR(out);
  return out;
}

internal Ast *
parseOperand()
{
  Ast *operand = 0;
  Token token = nextToken();
  Arena &arena = top_level_arena;
  switch (token.kind)
  {
    case '(':
    {
      operand = parseExpression();
      requireChar(')');
    } break;

    case '[':
    {// :list-opening-brace-eaten
      operand = parseList(arena);
    } break;

    case Token_Keyword_func:
    {
      operand = parseFunctionExpression(arena);
    } break;

    case Token_Special:
    {
      if (equal(token, '_'))
        operand = newAst(arena, Hole, token);
      else
        operand = newAst(arena, Identifier, token);
    } break;

    default:
    {
      tokenError("expected start of expression");
    }
  }

  while (hasMore())
  {
    if (optionalChar('('))
    {// function call syntax, let's keep going
      Ast *op = operand;
      i32 cap = 16;  // todo #grow
      Ast **args = pushArray(arena, cap, Ast*);
      CompositeAst *new_operand = newAst(arena, CompositeAst, op->token);
      new_operand->op        = op;
      new_operand->arg_count = 0;
      new_operand->args      = args;
      operand = new_operand;
      while (hasMore())
      {
        if (optionalChar(')'))
          break;
        else
        {
          i32 arg_i = new_operand->arg_count++;
          assert(arg_i < cap);
          if ((args[arg_i] = parseExpression()))
          {
            if (!optionalChar(','))
            {
              requireChar(')', "expected ',' or ')'");
              break;
            }
          }
        }
      }
    }
    else if (optionalChar('.'))
    {// member accessor
      AccessorAst *accessor = newAst(arena, AccessorAst, TK->last_token);
      accessor->record      = operand;
      if (requireIdentifier("expected identifier as field name"))
      {
        accessor->field_name = lastToken();
        operand              = accessor;
      }
    }
    else break;
  }
  NULL_WHEN_ERROR(operand);
  return operand;
}

inline b32
eitherOrChar(char optional, char require)
{
  b32 out = false;
  if (!optionalChar(optional))
  {
    out = requireChar(require);
  }
  return out;
}

inline b32
seesArrowExpression()
{
  b32 out = false;
  Tokenizer tk_save = *TK;
  if (equal(nextToken(), '('))
  {
    if (eatUntilMatchingPair())
    {
      out = (nextToken().kind == Token_Arrow);
    }
  }
  *TK = tk_save;
  return out;
}

inline i32
precedenceOf(String op)
{
  int out = 0;  // NOTE: 0 means it's literally not a binop
  const i32 eq_precedence = 50;

  // TODO: implement for real
  if (equal(op, "->"))
  {
    out = eq_precedence - 20;
  }
  else if (equal(op, "/\\"))
  {
    out = eq_precedence - 10;
  }
  else if (equal(op, "\\/"))
  {
    out = eq_precedence - 10;
  }
  else if (equal(op, "=") || equal(op, "!="))
  {
    out = eq_precedence;
  }
  else if (equal(op, "<") || equal(op, ">") ||
           equal(op, "<?") || equal(op, ">?") ||
           equal(op, "<=") || equal(op, ">=") ||
           equal(op, "<=?") || equal(op, ">=?") ||
           equal(op, "=?") || equal(op, "=="))
  {
    out = eq_precedence + 5;
  }
  else if (equal(op, "+") || equal(op, "-") || equal(op, "+/"))
  {
    out = eq_precedence + 10;
  }
  else if (equal(op, "|"))
  {
    out = eq_precedence + 15;
  }
  else if (equal(op, "&") || equal(op, "*") || equal(op, "*/") || equal(op, "/"))
  {
    out = eq_precedence + 20;
  }
  else if (equal(op, "cons"))
  {
    // NOTE: temporary precedence, until the day we figure a better convention.
    out = eq_precedence + 2;
  }

  return out;
}

internal b32
seesCast()
{
  b32 out = false;
  Tokenizer tk_save = *TK;
  if (peekChar() == '(')
  {
    eatToken();
    if (peekChar() == ':')
    {
      out = true;
    }
  }
  *TK = tk_save;
  return out;
}

internal TypedExpression *
parseCast()
{
  TypedExpression *out = 0;

  Token token = lastToken();
  requireChar('(');
  requireChar(':');
  assert(noError());

  if (Ast *type = parseExpression())
  {
    if (requireChar(')'))
    {
      if (Ast *expression = parseExpression())
      {
        out = newAst(top_level_arena, TypedExpression, token);
        out->type       = type;
        out->expression = expression;
      }
    }
  }
  return out;
}

struct ParseExpressionOptions {
  i32 min_precedence = -9999;
};

internal Ast *
parseExpression_(ParseExpressionOptions opt)
{
  Arena &arena = top_level_arena;
  Ast *out = 0;
  if (seesArrowExpression())
  {
    // NOTE: Doing it like this means that all operators bind stronger than "->".
    out = parseArrowType(false);
  }
  else if (seesCast())
  {
    out = parseCast();
  }
  else if (Ast *operand = parseOperand())
  {
    // (a+b) * c
    //     ^
    for (b32 stop = false; !stop && hasMore();)
    {
      Token op_token = peekToken();
      if (isIdentifier(op_token) || op_token.kind == Token_Arrow)
      {// infix operator syntax
        // (a+b) * c
        //        ^
        Identifier *op = newAst(arena, Identifier, op_token);
        i32 precedence = precedenceOf(op_token.string);
        if (precedence >= opt.min_precedence)
        {
          // recurse
          eatToken();
          // a + b * c
          //      ^
          ParseExpressionOptions opt1 = opt;
          opt1.min_precedence = precedence;

          if (Ast *recurse = parseExpression_(opt1))
          {
            if (op_token.kind == Token_Arrow)
            {
              Ast *param_type = operand;
              ArrowAst *arrow = newAst(arena, ArrowAst, op_token);
              arrow->param_count = 1;
              pushItems(arena, arrow->param_types, param_type);
              pushItems(arena, arrow->param_names, String{});
              pushItems(arena, arrow->param_flags, (u32)0);
              arrow->output_type = recurse;
              operand = arrow;
            }
            else
            {
              // bookmark cleanup: use pushItems
              i32 arg_count = 2;
              Ast **args    = pushArray(arena, arg_count, Ast*);
              args[0] = operand;
              args[1] = recurse;

              CompositeAst *new_operand = newAst(arena, CompositeAst, op_token);
              new_operand->op        = op;
              new_operand->arg_count = arg_count;
              new_operand->args      = args;
              operand = new_operand;
            }
          }
        }
        else
        {
          // we are pulled to the left
          // a * b + c
          //      ^
          stop = true;
        }
      }
      else if (isExpressionEndMarker(op_token)) {
        stop = true;
      }
      else {
        tokenError(op_token, "expected operator token");
      }
    }
    if (noError())
      out = operand;
  }

  NULL_WHEN_ERROR(out);
  return out;
}


#if 0  // bookmark

forward_declare inline Ast *
parseExpression()
{
  return parseExpression_(ParseExpressionOptions{});
}

struct NormList {
  i32          count;
  Identifier **items;
};

// todo No need to normalize a fork if that fork contains other forks inside.
inline void
insertAutoNormalizations(Arena *arena, NormList norm_list, Ast *in0)
{
  switch (in0->kind)
  {
    case Ast_ForkAst:
    {
      ForkAst *in = castAst(in0, ForkAst);
      for (i32 case_id=0; case_id < in->case_count; case_id++)
      {
        Ast *body = in->cases[case_id];
        insertAutoNormalizations(arena, norm_list, body);
        for (i32 norm_id=0; norm_id < norm_list.count; norm_id++)
        {
          Identifier *item = norm_list.items[norm_id];
          // todo cleanup: we'll need to clean useless let-normalization too!
          LetAst *new_body = newAst(arena, LetAst, &item->token);
          new_body->lhs   = item->token.string;
          new_body->rhs   = item;
          new_body->type  = newAst(arena, NormalizeMeAst, &item->token);
          new_body->body  = body;
          setFlag(&new_body->flags, AstFlag_Generated);
          body = new_body;
        }
        GoalTransform *new_body = newAst(arena, GoalTransform, &body->token);
        new_body->new_goal = newAst(arena, NormalizeMeAst, &body->token);
        setFlag(&new_body->flags, AstFlag_Generated);
        new_body->body = body;
        in->cases[case_id] = new_body;
      }
    } break;

    case Ast_RewriteAst:
    {
      RewriteAst *in = castAst(in0, RewriteAst);
      assert(in->body);
      insertAutoNormalizations(arena, norm_list, in->body);
    } break;

    case Ast_LetAst:
    {
      LetAst *in = castAst(in0, LetAst);
      insertAutoNormalizations(arena, norm_list, in->body);
    } break;

    default:
    {} break;
  }
}

internal FunctionAst *
parseGlobalFunction(Arena *arena, Token *name, b32 is_theorem)
{
  FunctionAst *out = newAst(arena, FunctionAst, name);
  assert(isIdentifier(name));

  if (Ast *signature0 = parseExpression())
  {
    NormList norm_list = {};
    if (ArrowAst *signature = castAst(signature0, ArrowAst))
    {
      while (true)
      {
        // todo #speed elseif
        if (optionalDirective("norm"))
        {
          pushContext("auto normalization: #norm(IDENTIFIER...)");
          if (requireChar('('))
          {
            i32 cap = 16;  // todo #grow
            norm_list.items = pushArray(temp_arena, cap, Identifier*);
            for (; noError(); )
            {
              if (optionalChar(')'))
                break;
              else if (requireIdentifier("expect auto-normalized parameter"))
              {
                // todo handle unbound identifier: all names in the norm list
                // should be in the function signature.
                Token *name = lastToken();
                i32 norm_i = norm_list.count++;
                assert(norm_i < cap);
                norm_list.items[norm_i] = newAst(arena, Identifier, name);
                if (!optionalChar(','))
                {
                  requireChar(')');
                  break;
                }
              }
            }
          }
          popContext();
        }
        else if (optionalDirective("hint"))
        {
          setFlag(&out->function_flags, FunctionFlag_is_global_hint);
        }
        else if (optionalDirective("no_expand"))
        {
          // todo: we can automatically infer this!
          setFlag(&out->function_flags, FunctionFlag_no_expand);
        }
        else if (optionalDirective("expand"))
        {
          setFlag(&out->function_flags, FunctionFlag_expand);
        }
        else if (optionalDirective("builtin"))
        {
          setFlag(&out->flags, AstFlag_IsBuiltin);
          if (optionalChar('('))
          {
            if (requireIdentifier())
              out->builtin_name = lastString();
            requireChar(')');
          }
        }
        else if (optionalDirective("tag"))
        {
          if (requireChar('('))
          {
            for (; hasMore();)
            {
              if (requireIdentifier())
              {
                bufPush(out->tags, lastString());
              }
              if (eitherOrChar(',', ')')) break;
            }
          }
        }
        else if (optionalDirective("measure"))
        {
          if (requireChar('('))
          {
            if (Ast *measure_function_body = parseExpression())
            {
              if (requireChar(','))
              {
                if (Ast *well_founded_proof = parseExpression())
                {
                  FunctionAst *measure_function = newAst(arena, FunctionAst, &measure_function_body->token);
                  measure_function->signature = pushCopy(arena, signature);
                  measure_function->signature->output_type = 0;  // :no-output-type
                  measure_function->body      = measure_function_body;
                  out->measure_function   = measure_function;
                  out->well_founded_proof = well_founded_proof;
                }
              }
            }
            requireChar(')');
          }
        }
        else
          break;
      }

      if (Ast *body = parseSequence())
      {
        if (is_theorem)
        {
          insertAutoNormalizations(arena, norm_list, body);
          setFlag(&out->function_flags, FunctionFlag_never_expand);
        }
        out->body      = body;
        out->signature = signature;
      }
    }
    else
      reportError(signature0, "function definition requires an arrow type");
  }

  NULL_WHEN_ERROR(out);
  return out;
}

#endif

/*
  expr3 = INT | '(' expr ')'
  expr2 = -expr3
  expr1 = expr2 ([/ *] expr2)*
  expr0 = expr1 ([+-] expr1)*
  expr  = expr0
*/
