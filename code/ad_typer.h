#pragma once

#include "ad_parser.h"

struct Function;
struct Union;
struct Arrow;
struct Composite;
// NOTE: I almost think records are different cases from composites, just a
// coincidence that they are represented the same way.
typedef Composite Record;
struct Constructor;
struct Term;
struct LocalBindings;
struct DataMap;

enum TermKind {
  Term_Hole = 1,

  Term_Pointer,

  Term_Function,
  Term_Variable,
  Term_Accessor,
  Term_Composite,
  Term_Arrow,
};

struct Trinary {i32 v;};
Trinary Trinary_False   = Trinary{0};
Trinary Trinary_True    = Trinary{1};
Trinary Trinary_Unknown = Trinary{2};
Trinary toTrinary(b32 v)
{
  if (v == 0) return Trinary_False;
  else return Trinary_True;
}

b32 operator==(Trinary a, Trinary b)
{
  return a.v == b.v;
}

b32 operator!=(Trinary a, Trinary b)
{
  return a.v != b.v;
}

struct Pointer;

struct Scope {
  Scope    *outer;
  i32       depth;
  i32       param_count;
  Pointer **pointers;
};

const u32 ExpectError_Ambiguous = 1 << 0;  // NOTE: Maybe a better name would be "missing type info".
const u32 ExpectError_WrongType = 1 << 1;

struct Typer {
  LocalBindings *bindings;
  Scope         *scope;
  u32            expected_errors;  // ExpectError
};

struct TermList {Term *head; TermList *tail;};

struct Term {
  TermKind  kind;
  i32       serial;
  Term     *type;
  Token    *global_name;
};

struct SyntheticAst : Ast {
  Term *term;
};

struct TermArray {
  i32    count;
  Term **items;
};

struct Function : Term {
  Term *body;
  u32   function_flags;
};

struct LocalBinding
{
  i32           hash;
  String        key;
  i32           var_index;
  LocalBinding *tail;
};

struct LookupLocalName {
  b32   found;
  i32   stack_delta;
  i32   var_index;
  operator bool() {return found;}
};

struct LocalBindings {
  LocalBinding   table[128];
  LocalBindings *tail;
};

struct Variable : Term {
  String name;
  i32    delta;
  i32    index;
};

struct TreePath {
  i32       head;  // -1 for op (todo: change to u8 so we can hack this on the stack)
  TreePath *tail;
};

struct Accessor : Term {
  Term   *record;
  i32     index;
  String  debug_field_name;
};

struct Composite : Term {
  union {
    struct {
      Term  *op;
      i32    arg_count;
      Term **args;
    };
    struct {
      Constructor  *ctor;
      i32           member_count;
      Term        **members;
    };
  };
};

struct Arrow : Term {
  i32     param_count;
  String *param_names;
  Term  **param_types;
  u32    *param_flags;
  Term   *output_type;
};

struct GlobalSlot {
  String key;

  Term   **terms;
  String **tags;

  GlobalSlot *hash_tail;
};

struct GlobalBindings  // :global-bindings-zero-at-startup
{
    GlobalSlot table[1024];
};

struct BuildTerm
{
  Term *value;
  operator bool()  { return value; }
  operator Term*() { return value; }
};

struct FileList {
  char     *head_path;
  char     *head_content;
  FileList *tail;
};

inline Term ** toTerms(Pointer **pointers) {return (Term**)pointers;}

// :global_state_cleared_at_startup
struct EngineState {
  Arena           *top_level_arena;
  FileList        *file_list;
  GlobalBindings  *bindings;
  b32              top_level_debug_mode;
};

struct TreePathList {
  TreePath     *head;
  TreePathList *tail;
};

String number_to_string[] = {
  toString("0"), toString("1"), toString("2"), toString("3"),
  toString("4"), toString("5"), toString("6"), toString("7"),
  toString("8"), toString("9"), toString("10"), toString("11"),
  toString("12"), toString("13"), toString("14"), toString("15"),
};
