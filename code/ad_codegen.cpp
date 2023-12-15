/*
  IMPORTANT: libclang's node visitor doesn't work with extern "C",
  so this code is effectively garbage :<
  It's like you shouldn't ever use somebody else's code, or something.

  TODO: "string.chars" should be banned!
 */

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "clang-c/Index.h"

#include "kv-utils.h"
// #include "kv_stdlib.h"

String forward_declare_macro  = toString("forward_declare");
String function_typedef_macro = toString("function_typedef");

enum AnnotationType {
  AnnotationTypeNull            = 0,
  AnnotationTypeForwardDeclare  = 1,
  AnnotationTypeFunctionTypedef = 2,
};

struct Location
{
  AnnotationType annotation_type;
  unsigned  line;
  String    src_filename;
  String    dst_filename;
  FILE     *dst_filestream;
  Location *next;
};

struct FILEWithName {
  String name;
  FILE  *filestream;
};

struct State
{
  // "fd" = "forward declare"
  // TODO: maybe rename to "dst_files"
  FILEWithName       opened_fd_files[64];
  i32                opened_fd_files_count;
  Location          *fd_locations;
  CXTranslationUnit  translation_unit;
};

global_variable Arena perm_arena;
global_variable Arena temp_arena;
global_variable String code_dir;
global_variable State state;

internal void
generateCode(char *templated_code, char *output_name, char *type)
{
  FILE *output = fopen(output_name, "w");
  if (output == 0)
  {
    char *at = templated_code;
    while (*at)
    {
      if ((*at == '.') && (*(at+1) == '.') && (*(at+2) == '.'))
      {
        at += 2;
        fprintf(output, "%s", type);
      }
      else
      {
        fprintf(output, "%c", *at);
      }
      at++;
    }

    fclose(output);
  }
  else
  {
    fprintf(stderr, "Cannot open file %s! \n", output_name);
  }
}

inline String
print(Arena &buffer, CXString string)
{
  String out = {};
  const char *cstring = clang_getCString(string);
  if (cstring)
  {
    out = print(buffer, (char *)cstring);
    clang_disposeString(string);
  }
  return out;
}

inline String toString(Arena &buffer, CXString string) {
  return print(buffer, string);
}

internal char *
printFunctionSignature(Arena &buffer, CXCursor cursor)
{
  char *out = (char *)getNext(buffer);

  CXType type = clang_getCursorType(cursor);
  print(buffer, clang_getTypeSpelling(clang_getResultType(type)));
  print(buffer, " ");

  print(buffer, clang_getCursorSpelling(cursor));

  print(buffer, "(");
  int num_args = clang_Cursor_getNumArguments(cursor);
  for (int i = 0; i < num_args; ++i)
  {
    CXCursor arg_cursor = clang_Cursor_getArgument(cursor, i);
    print(buffer, clang_getTypeSpelling(clang_getArgType(type, i)));
    print(buffer, " ");
    print(buffer, clang_getCursorSpelling(arg_cursor));
    if (i != num_args-1)
    {
      print(buffer, ", ");
    }
  }
  print(buffer, ");");

  return out;
}

// #cutnpaste from printFunctionSignature but such is the nature of C++
internal char *
printFunctionTypedef(Arena &buffer, CXCursor cursor)
{
  // TODO: cleanup use format string please!
  char *out = (char *)getNext(buffer);

  CXType type = clang_getCursorType(cursor);
  auto return_type = clang_getTypeSpelling(clang_getResultType(type));

  print(buffer, "typedef %s ", return_type);

  // function pointer type is the same but with the first char capitalized
  auto function_name = (char *)clang_getCString(clang_getCursorSpelling(cursor));
  print(buffer, toupper(function_name[0]));
  print(buffer, (char *)(function_name+1));

  print(buffer, "(");
  int num_args = clang_Cursor_getNumArguments(cursor);
  for (int i = 0; i < num_args; ++i)
  {
    CXCursor arg_cursor = clang_Cursor_getArgument(cursor, i);
    print(buffer, clang_getTypeSpelling(clang_getArgType(type, i)));
    print(buffer, " ");
    print(buffer, clang_getCursorSpelling(arg_cursor));
    if (i != num_args-1)
    {
      print(buffer, ", ");
    }
  }
  print(buffer, ");");

  return out;
}

internal CXChildVisitResult
topLevelVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  (void)parent; (void)client_data;
  auto temp_mem = beginTemporaryMemory(temp_arena);
  CXSourceLocation location = clang_getCursorLocation(cursor);

  unsigned cursor_line;
  CXFile file;
  clang_getExpansionLocation(location, &file, &cursor_line, 0, 0);

  CXChildVisitResult child_visit_result = CXChildVisit_Continue;
  String filename = toString(temp_arena, clang_getFileName(file));

  if (filename.chars)
  {
    temp_arena.used++;
    CXString path_name0 = clang_File_tryGetRealPathName(file);
    String path = print(temp_arena, path_name0);
    if (path.chars)
    {
      if (isSubstring(path, code_dir, false))
      {
        temp_arena.used++;
        CXString cursor_spelling = clang_getCursorSpelling(cursor);
        CXCursorKind cursor_kind = clang_getCursorKind(cursor);

        // TODO: we need a "print+1" function
        String cursor_string = print(temp_arena, cursor_spelling);
        temp_arena.used++;

        switch (cursor_kind)
        {
          case CXCursor_MacroExpansion:
          {// check for annotations
            String cursor_string = print(temp_arena, "%s\0", cursor_spelling);

            AnnotationType annotation_type = AnnotationTypeNull;
            if (equal(cursor_string, forward_declare_macro)) {
              annotation_type = AnnotationTypeForwardDeclare;
            } else if (equal(cursor_string, function_typedef_macro)) {
              annotation_type = AnnotationTypeFunctionTypedef;
            }
            if (annotation_type)
            {
              // printf("cursor_string: %s, file: %s, line: %d\n", cursor_string.chars, path.chars, cursor_line);
              CXSourceRange source_range = clang_getCursorExtent(cursor);

              String dst_filename = {};
              // bookmark get the real path!
              {// Get the macro arguments b/c libclang won't pass it to me (thanks).
                CXToken  *tokens     = 0;
                unsigned  num_tokens = 0;
                clang_tokenize(state.translation_unit, source_range, &tokens, &num_tokens);
                assert(num_tokens == 4);
                CXToken dst_filename_token = tokens[2];
                CXString dst_filename_cxstring = clang_getTokenSpelling(state.translation_unit, dst_filename_token);
                const char *const_dst_filename_cstring = clang_getCString(dst_filename_cxstring);
                char dst_filename_cstring_[1024];
                strcpy(dst_filename_cstring_, const_dst_filename_cstring);
                char *dst_filename_cstring = dst_filename_cstring_;
                {// get rid of the double quotes
                  i32 len = strlen(dst_filename_cstring);
                  dst_filename_cstring[len-1] = 0;
                  dst_filename_cstring = dst_filename_cstring+1;
                }
                dst_filename = print(perm_arena, "%s/%s", code_dir.chars, dst_filename_cstring);
                perm_arena.used++;
              }
              char *after_dst_filename = dst_filename.chars + dst_filename.length;
              assert(*after_dst_filename == 0);

              Location *loc = pushStruct(perm_arena, Location, true);
              loc->annotation_type = annotation_type;
              loc->line            = cursor_line;
              loc->next            = state.fd_locations;
              loc->src_filename    = copyString(perm_arena, filename);
              loc->dst_filename    = dst_filename;
              state.fd_locations   = loc;

              for (i32 file_index=0;
                   file_index < arrayCount(state.opened_fd_files);
                   file_index++) {
                // see if the header file is already opened
                auto &fdfile = state.opened_fd_files[file_index];
                if (equal(loc->dst_filename, fdfile.name)) {
                  loc->dst_filestream = fdfile.filestream;
                  assert(loc->dst_filestream);
                  break;
                }
              }

              if (!loc->dst_filestream) {
                // create new header file handle
                FILEWithName *f = state.opened_fd_files + state.opened_fd_files_count++;
                assert(state.opened_fd_files_count < arrayCount(state.opened_fd_files));
                f->name       = loc->dst_filename;
                f->filestream = fopen(loc->dst_filename.chars, "w+");
                if (!f->filestream) {
                  printf("cannot open file %s\n", f->name.chars);
                  exit(1);
                }
                loc->dst_filestream = f->filestream;
                assert(loc->dst_filestream);
              }
              assert(*after_dst_filename == 0);
            }
          } break;

          case CXCursor_FunctionDecl:
          {
            for (Location *loc = state.fd_locations;
                 loc;
                 loc = loc->next)
            {
              assert(loc);
              b32 match_line = (cursor_line >= loc->line) && (cursor_line < loc->line+3);
              b32 match_file = equal(filename, loc->src_filename);
              if (match_line && match_file)
              {// function was asked to be forward-declared
                printf("forward declare: %s\n", cursor_string.chars);
                char *signature = printFunctionSignature(temp_arena, cursor);
                temp_arena.used++;
                char *function_pointer_typedef = printFunctionTypedef(temp_arena, cursor);

                if (loc->annotation_type == AnnotationTypeForwardDeclare) {
                  i32 printed = fprintf(loc->dst_filestream, "%s\n", signature);
                  assert(printed > 0);
                } else {
                  assert(loc->annotation_type == AnnotationTypeFunctionTypedef);
                  i32 printed = fprintf(loc->dst_filestream, "%s\n\n", function_pointer_typedef);
                  assert(printed > 0);
                }
                break;
              }
            }
          } break;

          // NOTE: UnexposedDecl could contain extern "C" because of a decade-old bug
          case CXCursor_LinkageSpec:
          case CXCursor_UnexposedDecl: {
            clang_visitChildren(cursor, topLevelVisitor, 0);
            break;
          }

          default:
          {
          } break;
        }
      }
    }
  }

  endTemporaryMemory(temp_mem);
  return child_visit_result;
}

/* This function will scan your code in current dir,
   output to generated/... */
int main()
{
  size_t temp_arena_size = megaBytes(8);
  temp_arena = newArena(malloc(temp_arena_size), temp_arena_size);

  size_t perm_arena_size = megaBytes(128);
  perm_arena = newArena(malloc(perm_arena_size), perm_arena_size);

  CXIndex index = clang_createIndex(0, 0);

  char code_dir_buffer[1024];
  realpath("../code", code_dir_buffer);
  code_dir = toString(code_dir_buffer);

  auto tu_name = print(perm_arena, "%s/%s\0", code_dir.chars, "game.cpp");
  CXTranslationUnit tu = clang_parseTranslationUnit(index, tu_name.chars, nullptr, 0, nullptr, 0, CXTranslationUnit_DetailedPreprocessingRecord);
  if (!tu) {
    printf("failed to parse translation unit %s\n", tu_name.chars);
  }
  state.translation_unit = tu;
  CXCursor cursor = clang_getTranslationUnitCursor(tu);

  clang_visitChildren(cursor, topLevelVisitor, &state);
  state.translation_unit = tu;

  for (i32 i=0; i < state.opened_fd_files_count; i++) {
    fclose(state.opened_fd_files[i].filestream);
  }

  return 0;
}
