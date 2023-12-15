#include "ad_typer.h"
#include "ad_parser.cpp"
#include "ad_debug_config.h"

global_variable EngineState global_state;

Term  hole_ = {.kind = Term_Hole};
Term *hole = &hole_;

inline i32 slotCount(GlobalSlot *slot) { return (i32)bufLength(slot->terms); }

inline void indent(Arena &buffer, i32 indentation)
{
  for (int id=0; id < indentation; id++)
    print(buffer, " ");
}

inline void newlineAndIndent(Arena &buffer, i32 indentation)
{
  print(buffer, "\n");
  indent(buffer, indentation);
}

// TODO this should be in parser
void print(Arena &buffer, Ast *in0, PrintOptions opt);
inline char *
printComposite(Arena &buffer, CompositeAst *in, PrintOptions opt)
{
  char *out = (char *)getNext(buffer);
  int    precedence = 0;        // todo: no idea what the default should be
  Ast   *op         = 0;
  i32    arg_count  = 0;

  Arrow *op_signature = 0;
  Constructor *op_ctor = 0;
  b32 no_print_as_binop = false;
  op       = in->op;
  arg_count = in->arg_count;

  precedence = precedenceOf(in->op->token.string);

  Ast **printed_args = in->args;
  i32 printed_arg_count = arg_count;
  if (op_signature)
  {// print out explicit args only
    if (!print_all_args)
    {
      printed_args = pushArray(top_level_arena, op_signature->param_count, Ast *);
      printed_arg_count = 0;
      for (i32 param_i = 0; param_i < op_signature->param_count; param_i++)
      {
        printed_args[printed_arg_count++] = in->args[param_i];
      }
    }
  }

  if (printed_arg_count == 2 && !no_print_as_binop)
  {// special path for infix binary operator
    if (precedence < opt.no_paren_precedence)
      print(buffer, "(");

    PrintOptions arg_opt = opt;
    // #hack to force printing parentheses when the precedence is the same (a+b)+c.
    arg_opt.no_paren_precedence = precedence+1;
    print(buffer, printed_args[0], arg_opt);

    print(buffer, " ");
    print(buffer, op, opt);
    print(buffer, " ");

    arg_opt.no_paren_precedence = precedence;
    print(buffer, printed_args[1], arg_opt);
    if (precedence < opt.no_paren_precedence)
      print(buffer, ")");
  }
  else
  {// normal prefix path
    print(buffer, op, opt);
    if (!(op_ctor && printed_arg_count == 0))
    {
      print(buffer, "(");
      PrintOptions arg_opt        = opt;
      arg_opt.no_paren_precedence = 0;
      for (i32 arg_i = 0; arg_i < printed_arg_count; arg_i++)
      {
        print(buffer, printed_args[arg_i], arg_opt);
        if (arg_i < printed_arg_count-1)
          print(buffer, ", ");
      }
      print(buffer, ")");
    }
  }
  return out;
}

void print(Arena &buffer, Ast *in0, PrintOptions opt)
{// printAst
  if (in0)
  {
    PrintOptions new_opt = opt;
    unsetFlag(&new_opt.flags, PrintFlag_Detailed);
    new_opt.indentation += 1;

    switch (in0->kind)
    {
      case Ast_Hole:
      {print(buffer, "_");} break;

      case Ast_Identifier:
      {print(buffer, in0->token.string);} break;

      case Ast_RewriteAst:
      {
        RewriteAst *in = castAst(in0, RewriteAst);
        print(buffer, "rewrite");
        print(buffer, " ");
        if (in->right_to_left) print(buffer, "<- ");
        print(buffer, in->eq_or_proof, new_opt);
      } break;

      case Ast_CompositeAst:
      {
        auto *in = castAst(in0, CompositeAst);
        printComposite(buffer, in, new_opt);
      } break;

      case Ast_ForkAst:
      {
        ForkAst *in = castAst(in0, ForkAst);
        print(buffer, "fork ");
        print(buffer, in->subject, new_opt);
        newlineAndIndent(buffer, opt.indentation);
        print(buffer, "{");
        i32 case_count = in->case_count;
        for (i32 ctor_id = 0;
             ctor_id < case_count;
             ctor_id++)
        {
          Token *ctor = in->ctors + ctor_id;
          print(buffer, ctor->string);
          print(buffer, ": ");
          print(buffer, in->cases[ctor_id], new_opt);
          if (ctor_id != case_count)
          {
            print(buffer, ", ");
            newlineAndIndent(buffer, opt.indentation+1);
          }
        }
        print(buffer, "}");
      } break;

      case Ast_ArrowAst:
      {
        ArrowAst *in = castAst(in0, ArrowAst);
        print(buffer, "(");
        for (int param_i = 0;
             param_i < in->param_count;
             param_i++)
        {
          print(buffer, in->param_names[param_i]);
          print(buffer, ": ");
          print(buffer, in->param_types[param_i], new_opt);
          if (param_i < in->param_count-1)
            print(buffer, ", ");
        }
        print(buffer, ") -> ");

        print(buffer, in->output_type, new_opt);
      } break;

      case Ast_AccessorAst:
      {
        AccessorAst *in = castAst(in0, AccessorAst);
        print(buffer, in->record, new_opt);
        print(buffer, ".");
        print(buffer, in->field_name.string);
      } break;

      case Ast_FunctionDecl: {print(buffer, "function decl");} break;

      case Ast_FunctionAst: {print(buffer, "lambda");} break;

      case Ast_LetAst:
      {
        LetAst *in = castAst(in0, LetAst);
        print(buffer, in->lhs);
        if (in->type)
        {
          print(buffer, " : ");
          print(buffer, in->type, new_opt);
        }
        print(buffer, " := ");
        print(buffer, in->rhs, new_opt);
        print(buffer, "; ");
        print(buffer, in->body, new_opt);
      } break;

      case Ast_UnionAst:
      {
        print(buffer, "<some union>");
      } break;

      case Ast_CtorAst:
      {
        print(buffer, "<some ctor>");
      } break;

      case Ast_SeekAst:
      {
        print(buffer, "<some seek>");
      } break;
    }
  }
  else
    print(buffer, "<NULL>");
}

inline void print(Arena &buffer, Ast *in0)
{
  return print(buffer, in0, {});
}

void print(Arena &buffer, Term *in0, PrintOptions opt);

inline char *
printComposite(Arena &buffer, Composite *in, PrintOptions opt)
{
  char *out = (char *)getNext(buffer);

  Arrow *op_signature = castTerm(in->op->type, Arrow);

  String op_name = {};
  if (Token *global_name = in->op->global_name)
  {
    op_name = global_name->string;
  }
  else if (Variable *var = castTerm(in->op, Variable))
  {
    op_name = var->name;
  }
  int precedence = precedenceOf(op_name);

  Term **printed_args = in->args;
  i32 printed_arg_count = in->arg_count;
  if (op_signature)
  {// print out explicit args only
    if (!print_all_args)
    {
      printed_args = pushArray(top_level_arena, op_signature->param_count, Term*);
      printed_arg_count = 0;
      for (i32 param_i = 0; param_i < op_signature->param_count; param_i++)
      {
        printed_args[printed_arg_count++] = in->args[param_i];
      }
    }
  }

  if (printed_arg_count == 2 && precedence)
  {// special path for infix binary operator
    if (precedence < opt.no_paren_precedence)
      print(buffer, "(");

    PrintOptions arg_opt = opt;
    // #hack to force printing parentheses when the precedence is the same (a+b)+c.
    arg_opt.no_paren_precedence = precedence+1;
    print(buffer, printed_args[0], arg_opt);

    print(buffer, " ");
    print(buffer, in->op, opt);
    print(buffer, " ");

    arg_opt.no_paren_precedence = precedence;
    print(buffer, printed_args[1], arg_opt);
    if (precedence < opt.no_paren_precedence)
      print(buffer, ")");
  }
  else
  {// normal prefix path
    print(buffer, in->op, opt);
    if (!(printed_arg_count == 0))
    {
      print(buffer, "(");
      PrintOptions arg_opt        = opt;
      arg_opt.no_paren_precedence = 0;
      for (i32 arg_i = 0; arg_i < printed_arg_count; arg_i++)
      {
        print(buffer, printed_args[arg_i], arg_opt);
        if (arg_i < printed_arg_count-1)
          print(buffer, ", ");
      }
      print(buffer, ")");
    }
  }
  return out;
}

internal void
printArrow(StringBuffer &buffer, Arrow *in, b32 skip_output, PrintOptions opt)
{
  print(buffer, "(");
  for (int param_i = 0;
       param_i < in->param_count;
       param_i++)
  {
    String param_name = in->param_names[param_i];
    if (param_name.chars)
    {
      print(buffer, param_name);
      print(buffer, ": ");
    }
    print(buffer, in->param_types[param_i], opt);
    if (param_i < in->param_count-1)
    {
      print(buffer, ", ");
    }
  }
  print(buffer, ")");

  if (in->output_type && !skip_output)
  {
    print(buffer, " -> ");
    print(buffer, in->output_type, opt);
  }
}

void print(Arena &buffer, Term *in0, PrintOptions opt)
{// mark: printTerm
  if (in0)
  {
    b32 skip_print_type = false;
    PrintOptions new_opt = opt;
    if (in0->global_name &&
        !checkFlag(opt.flags, PrintFlag_Detailed))
    {
      print(buffer, in0->global_name->string);
    }
    else
    {
      if (!checkFlag(opt.flags, PrintFlag_LockDetailed))
      {
        unsetFlag(&new_opt.flags, PrintFlag_Detailed);
      }
      if (new_opt.print_type_depth)
      {
        new_opt.print_type_depth--;
      }
      new_opt.indentation = opt.indentation + 1;

      switch (in0->kind)
      {
        case Term_Variable:
        {
          Variable *in = castTerm(in0, Variable);
          bool has_name = in->name.chars;
          if (has_name)
            print(buffer, in->name);
          else
            print(buffer, "anon");

          if (!has_name || print_var_delta || print_var_index)
          {
            print(buffer, "[");
            if (!has_name || print_var_delta)
            {
              print(buffer, "%d", in->delta);
            }
            if (!has_name || print_var_index)
            {
              print(buffer, ",%d", in->index);
            }
            print(buffer, "]");
          }
        } break;

        case Term_Accessor:
        {
          Accessor *in = castTerm(in0, Accessor);
          print(buffer, in->record, new_opt);
          print(buffer, ".");
          print(buffer, in->debug_field_name);
        } break;

#if 0
        case Term_Pointer:
        {
          Pointer *in = (Pointer *)(in0);
          if (shouldPrintExpanded(in))
          {
            print(buffer, in->ref, opt);
          }
          else
          {
            printPointerName(buffer, in);
          }
        } break;
#endif

        case Term_Hole:
        {print(buffer, "_");} break;

        case Term_Composite:
        {
          auto in = (Composite *)(in0);
          printComposite(buffer, in, new_opt);
        } break;

        case Term_Function:
        {
          Function *in = castTerm(in0, Function);
          print(buffer, "fn");
          if (in->type)
          {
            print(buffer, in->type, new_opt);
            skip_print_type = true;
          }
          newlineAndIndent(buffer, opt.indentation);
          print(buffer, "{ return ");
          print(buffer, in->body, new_opt);
          print(buffer, " }");
        } break;

        case Term_Arrow:
        {
          Arrow *in = castTerm(in0, Arrow);
          printArrow(buffer, in, false, new_opt);
        } break;

        default:
        {
          todoIncomplete;
        } break;
      }
    }

    if (opt.print_type_depth && !skip_print_type)
    {
      print(buffer, ": ");
      new_opt.print_type_depth = 0;
      print(buffer, (in0)->type, new_opt);
    }
  }
  else
    print(buffer, "<NULL>");
}

inline void print(Arena &buffer, Term *in0)
{
  return print(buffer, in0, {});
}

inline void attach(char *key, String value)
{
  InterpError *error = TK->error;
  assert(error->attachment_count < arrayCount(error->attachments));
  error->attachments[error->attachment_count++] = {key, value};
}

inline void
attach(char *key, i32 count, Term **terms, PrintOptions print_options={})
{
  StartString start = startString(error_buffer);
  for (i32 id=0; id < count; id++)
  {
    print(error_buffer, "\n");
    print(error_buffer, terms[id], print_options);
  }
  attach(key, endString(start));
}

inline void
attach(char *key, Token *token)
{
  attach(key, token->string);
}

inline void
attach(char *key, Ast *ast)
{
  StartString start = startString(error_buffer);
  print(error_buffer, ast);
  attach(key, endString(start));
}

inline void
attach(char *key, Term *value)
{
  StartString start = startString(error_buffer);
  print(error_buffer, value);
  attach(key, endString(start));
}

inline void
attach(char *key, i32 n)
{
  StartString start = startString(error_buffer);
  print(error_buffer, "%d", n);
  attach(key, endString(start));
}

struct LookupCurrentFrame { LocalBinding* slot; b32 found; };

internal LookupCurrentFrame
lookupCurrentFrame(LocalBindings *bindings, String key, b32 add_if_missing)
{
  LocalBinding *slot = 0;
  b32 found = false;
  u32 hash = stringHash(key) % arrayCount(bindings->table);
  slot = bindings->table + hash;
  b32 first_slot_valid = slot->key.length;
  if (first_slot_valid)
  {
    b32 stop = false;
    while (!stop)
    {
      if (equal(slot->key, key))
      {
        stop = true;
        found = true;
      }
      else if (slot->tail)
        slot = slot->tail;
      else
      {
        stop = true;
        if (add_if_missing)
        {
          allocate(top_level_arena, slot->tail);
          slot = slot->tail;
          slot->key  = key;
          slot->tail = 0;
        }
      }
    }
  }
  else if (add_if_missing)
  {
    slot->key = key;
  }

  LookupCurrentFrame out = { slot, found };
  return out;
}

inline Term *
lookupLocalName(Typer &typer, Token &token)
{
  String name = token.string;
  LocalBindings *bindings = typer.bindings;
  Term *out = {};
  Scope *scope = typer.scope;
  for (i32 stack_delta = 0; bindings; stack_delta++)
  {
    LookupCurrentFrame lookup = lookupCurrentFrame(bindings, name, false);
    if (lookup.found)
    {
      assert(lookup.slot->var_index < scope->param_count);
      // out = scope->pointers[lookup.slot->var_index];  todo wtf is pointer
      break;
    }
    else
    {
      bindings = bindings->tail;
      scope    = scope->outer;
    }
  }
  return out;
}

internal GlobalSlot *
lookupGlobalNameSlot(String key, b32 add_new)
{
  // :global-bindings-zero-at-startup
  GlobalSlot *slot = 0;
  u32 hash = stringHash(key) % arrayCount(global_state.bindings->table);
  slot = global_state.bindings->table + hash;
  b32 first_slot_valid = slot->key.length == 0;
  if (first_slot_valid)
  {
    while (true)
    {
      if (equal(slot->key, key))
        break;
      else if (slot->hash_tail)
        slot = slot->hash_tail;
      else
      {
        if (add_new)
        {
          slot->hash_tail = pushStruct(*global_state.top_level_arena, GlobalSlot, true);
          slot = slot->hash_tail;
          slot->key = key;
        }
        else
          slot = 0;
        break;
      }
    }
  }
  else if (add_new)
    slot->key = key;
  else
    slot = 0;

  if (slot && !add_new) {
    assert(bufLength(slot->terms) != 0);
  }

  return slot;
}

internal GlobalSlot *
lookupGlobalNameSlot(Identifier *ident, b32 add_new)
{
  return lookupGlobalNameSlot(ident->token.string, add_new);
}

inline GlobalSlot *
lookupGlobalName(Token &token)
{
  if (GlobalSlot *slot = lookupGlobalNameSlot(token.string, false))
    return slot;
  else
  {
    // note: assume that if the code gets here, then the identifier isn't in
    // local scope either.
    tokenError(token, "identifier not found");
    attach("identifier", token);
    return 0;
  }
}

inline Arrow *
getSignature(Term *op)
{
  return castTerm(op->type, Arrow);
}

b32 equal(Term *l0, Term *r0)
{
  b32 out = false;
  if (l0 == r0) {
    out = true;
  }
  else if (l0->kind == r0->kind) {
    switch (l0->kind)
    {
      case Term_Variable:
      {
        Variable *lhs = castTerm(l0, Variable);
        Variable *rhs = castTerm(r0, Variable);
        if ((lhs->delta == rhs->delta) && (lhs->index == rhs->index))
          out = true;
      } break;

      case Term_Arrow:
      {
        Arrow* lhs = castTerm(l0, Arrow);
        Arrow* rhs = castTerm(r0, Arrow);
        i32 param_count = lhs->param_count;
        if (rhs->param_count == param_count)
        {
          b32 type_mismatch = false;
          for (i32 id = 0; id < param_count; id++)
          {
            if (!equal(lhs->param_types[id], rhs->param_types[id]))
            {
              type_mismatch = true;
              break;
            }
          }
          if (!type_mismatch)
          {
            out = equal(lhs->output_type, rhs->output_type);
          }
        }
        else {
          out = false;
        }
      } break;

      case Term_Composite:
      {
        Composite *l = castTerm(l0, Composite);
        Composite *r = castTerm(r0, Composite);

        b32 op_equal = equal(l->op, r->op);
        if (op_equal)
        {
          Arrow *arrow = getSignature(l->op);

          i32 count = l->arg_count;
          assert(l->arg_count == r->arg_count);

          i32 mismatch_count = 0;
          i32 false_count    = 0;
          i32       unique_diff_id   = 0;
          TreePath *unique_diff_path = 0;
          out = true;
          for (i32 arg_i=0; arg_i < count; arg_i++)
          {
            b32 recurse = equal(l->args[arg_i], r->args[arg_i]);
            if (!recurse) {
              out = false;
              break;
            }
          }
        }
      } break;

      case Term_Accessor:
      {
        Accessor *lhs = castTerm(l0, Accessor);
        Accessor *rhs = castTerm(r0, Accessor);
        if (lhs->index == rhs->index)
        {
          if (equal(lhs->record, rhs->record))
          {
            out = true;
          }
        }
      } break;

      default:
        todoIncomplete;
    }
  }

  return out;
}

inline b32
expectedWrongType(Typer &typer)
{
  return checkFlag(typer.expected_errors, ExpectError_WrongType);
}

inline b32
expectedAmbiguous(Typer &typer)
{
  return checkFlag(typer.expected_errors, ExpectError_Ambiguous);
}

inline b32
reportAmbiguousError(Typer &typer, Ast &ast, char *message)
{
  if (expectedAmbiguous(typer))
  {
    silentError();
    return false;
  }
  else
  {
    reportError(ast, message);
    return true;
  }
}

inline b32
reportWrongTypeError(Typer &typer, Ast &ast, char *message)
{
  if (expectedWrongType(typer))
  {
    silentError();
    return false;
  }
  else
  {
    reportError(ast, message);
    return true;
  }
}

BuildTerm buildTerm(Typer &typer, Ast &in0, Term *goal0)
{
  // beware: Usually we mutate in-place, but we may also allocate anew.
  i32 UNUSED_VAR serial = DEBUG_SERIAL++;

  assert(goal0);
  Arena &arena = top_level_arena;
  Term *value = 0;
  b32 should_check_type = true;
  b32 recursed = false;

  switch (in0.kind)
  {
    case Ast_Identifier:
    {
      Identifier &in = *castAst(&in0, Identifier);
      Token &name = in.token;
      if (Term *v = lookupLocalName(typer, name)) {
        value = v;
      } else {
        if (GlobalSlot *slot = lookupGlobalName(name))
        {
          for (i32 value_id = 0; value_id < slotCount(slot); value_id++)
          {
            Term *slot_value = slot->terms[value_id];
            if (equal((slot_value)->type, goal0))
            {
              if (value)
              {
                reportAmbiguousError(typer, in, "not enough type information to disambiguate global name");
                break;
              }
              else
              {
                value = slot_value;
              }
            }
          }

          if (!value)
          {
            if (reportWrongTypeError(typer, in, "global name does not match expected type"))
            {
              attach("name", name);
              attach("expected_type", goal0);
              if (slotCount(slot) == 1)
              {
                attach("actual_type", slot->terms[0]->type);
              }
            }
          }
        }

        if (value)
        {
          should_check_type = false;
        }
      }
    } break;

#if 0
    case Ast_ArrowAst:
    {
      ArrowAst *in = (ArrowAst *)(in0);
      Arrow *out = newTerm(arena, Arrow, rea.Type);
      i32 param_count = in->param_count;

      // :arrow-copy-done-later
      out->param_count = param_count;
      out->param_names = in->param_names;
      out->param_flags = in->param_flags;

      if (noError())
      {
        Scope *scope = typer->scope = newScope(typer->scope, param_count);
        extendBindings(typer);
        out->param_types = pushArray(arena, param_count, Term*);
        for (i32 i=0; i < param_count && noError(); i++)
        {
          if (Term *param_type = buildTerm(typer, in->param_types[i], hole))
          {
            out->param_types[i] = toAbstractTerm(arena, param_type, getScopeDepth(scope->outer));
            String name = in->param_names[i];
            scope->pointers[i] = newStackPointer(name, scope->depth, i, param_type);
            addLocalBinding(typer, name, i);
          }
        }

        if (noError())
        {
          value = out;
          // :no-output-type
          if (in->output_type)
          {
            if (Term *output_type = buildTerm(typer, in->output_type, hole))
            {
              out->output_type = toAbstractTerm(arena, output_type, getScopeDepth(scope->outer));
            }
          }
          else
          {
            out->output_type = hole;
          }
        }
        unwindBindingsAndScope(typer);
      }
    } break;
#endif

    case Ast_AccessorAst:
    {
      todoIncomplete;
#if 0
      AccessorAst &in = *castAst(&in0, AccessorAst);
      if (Term *record0 = buildTerm(typer, *in.record, hole))
      {
        if (Record *record = castRecord(record0))
        {
          // is a record, or a pointer to a record
          i32 ctor_i = record->ctor->index;
          Term **members = record->members;
          Union *uni = getUnionOrPolyUnion((record0)->type);
          Arrow *struc = getConstructorSignature(uni, ctor_i);
          i32 param_count = struc->param_count;

          i32 field_i = -1;
          for (i32 param_i=0; param_i < param_count; param_i++)
          {// figure out the param id
            if (equal(in->field_name.string, struc->param_names[param_i]))
            {
              field_i = param_i;
              break;
            }
          }

          if (field_i != -1)
          {
            value = members[field_i];
          }
          else
          {
            tokenError(&in->field_name, "accessor has invalid member");
            attach("expected a member of constructor", getConstructorName(uni, ctor_i));
          }
        }
        else
        {
          if (Union *uni = getUnionOrPolyUnion(record0->type))
          {
            if (uni->ctor_count == 1)
            {
              Arrow *struc = getConstructorSignature(uni, 0);
              i32 param_count = struc->param_count;

              i32 field_i = -1;
              for (i32 param_i=0; param_i < param_count; param_i++)
              {// figure out the param id
                if (equal(in->field_name.string, struc->param_names[param_i]))
                {
                  field_i = param_i;
                  break;
                }
              }

              if (field_i != -1)
              {
                value = newAccessorToStruct(record0, field_i);
              }
            }
          }
          reportError(in->record, "cannot access a non-record");
        }
      }
#endif
    } break;

    case Ast_FunctionAst:
    {
      todoIncomplete;
#if 0
      FunctionAst *in = castAst(in0, FunctionAst);
      Term *type = goal0;
      if (in->signature)
      {
        if (isNameOnlyArrowType(in->signature))
        {
          type = buildNameOnlyArrowType(typer, in->signature, goal0);
        }
        else
        {
          type = buildTerm(typer, in->signature, hole);
        }
      }

      if (noError())
      {
        if (Arrow *signature = castTerm(type, Arrow))
        {
          value = buildFunctionGivenSignature(typer, signature, in->body, 0, in);
        }
        else
        {
          reportError(in0, "function signature is required to be an arrow type");
          attach("type", type);
        }
      }
#endif
    } break;

    case Ast_TypedExpression:
    {
      TypedExpression &in = *(TypedExpression *)(&in0);
      if (Term *type = buildTerm(typer, *in.type, hole).value)
      {
        value = buildTerm(typer, *in.expression, type).value;
      }
    } break;
  }

  if (noError())
  {// typecheck if needed
    Term *actual = value->type;
    if (should_check_type && !recursed)
    {
      if (!equal(actual, goal0))
      {
        if (expectedWrongType(typer)) silentError();
        else
        {
          reportError(in0, "actual type differs from expected type");
          attach("got", actual);

#if 0
          DEBUG_LOG_compare = 1;
          matchType(actual, goal0);
          DEBUG_LOG_compare = 0;
#endif
        }
      }
    }
  }

  if (!getError()) {
    assert(value);
  }

  return BuildTerm{.value=value};
}

inline b32
isGlobalValue(Term *in0)
{
  // :atomic-constructors-dont-have-global-names
  return in0->global_name != NULL;
}

inline Term *
_copyTerm(Arena &arena, void *src, size_t size)
{
  Term *out = (Term *)copySize(arena, src, size);
  out->serial = DEBUG_SERIAL++;
  return out;
}

#define copyTerm(arena, src) \
  (mytypeof(src)) _copyTerm(arena, (src), sizeof(*(src)))

inline Term *
copyToGlobalArena(Term *in0)
{
  Term *out0 = 0;
  Arena &arena = *global_state.top_level_arena;
  if (isGlobalValue(in0))
  {
    out0 = in0;
  }
  else
  {
    switch (in0->kind)
    {
      case Term_Variable:
      {
        Variable *in  = castTerm(in0, Variable);
        Variable *out = copyTerm(arena, in);
        out0 = out;
      } break;

      case Term_Composite:
      {
        Composite *in  = castTerm(in0, Composite);
        Composite *out = copyTerm(arena, in);
        out->op = copyToGlobalArena(in->op);
        allocateArray(arena, in->arg_count, out->args);
        for (i32 i=0; i < in->arg_count; i++)
        {
          out->args[i] = copyToGlobalArena(in->args[i]);
        }
        out0 = out;
      } break;

      case Term_Arrow:
      {
        Arrow *in  = castTerm(in0, Arrow);
        Arrow *out = copyTerm(arena, in);
        i32 param_count = in->param_count;
        allocateArray(arena, param_count, out->param_types);
        for (i32 i=0; i < param_count; i++)
        {
          out->param_types[i] = copyToGlobalArena(in->param_types[i]);
        }
        out->output_type = copyToGlobalArena(in->output_type);
        // :arrow-copy-done-later
        out->param_names = copyArray(arena, param_count, in->param_names);
        out->param_flags = copyArray(arena, param_count, in->param_flags);
        out0 = out;
      } break;

      case Term_Function:
      {
        Function *in  = castTerm(in0, Function);
        Function *out = copyTerm(arena, in);
        out->body = copyToGlobalArena(in->body);
        out0 = out;
      } break;

      case Term_Accessor:
      {
        Accessor *in = (Accessor *)(in0);
        Accessor *out = copyTerm(arena, in);
        out->record = copyToGlobalArena(in->record);
        out0 = out;
      } break;

      invalidDefaultCase;
    }

    out0->type = copyToGlobalArena(in0->type);
  }
  return out0;
}

inline Term *
parseAndBuildGlobal(Typer typer, Term *expected_type=hole)
{
  Term *out = 0;
  if (Ast *ast = parseExpression())
  {
    if ((out = buildTerm(typer, *ast, expected_type)))
    {
      out = copyToGlobalArena(out);
    }
  }
  return out;
}

inline void
addGlobalBinding(Token &name, Term *value, String *tags=0)
{
  Arena &arena = *global_state.top_level_arena;
  assert(inArena(arena, value));
  GlobalSlot *slot = lookupGlobalNameSlot(name.string, true);
  // todo incomplete check for type conflict

  bufPush(slot->terms, value);
  bufPush(slot->tags, tags);

  value->global_name = pushCopy(arena, &name);
}
