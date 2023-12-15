#pragma once

struct Term;

enum TermKind {
  Term_Hole = 1,

  Term_Primitive,

  Term_Pointer,

  Term_Union,
  Term_Constructor,
  Term_Function,
  Term_Fork,
  Term_Variable,
  Term_Computation,
  Term_Accessor,
  Term_Composite,
  Term_Arrow,
  Term_Rewrite,
  Term_Let,
};

struct Term {
  TermKind  kind;
  i32       serial;
  Term     *type;
  Token    *global_name;
};
