/*
  Usage: Define "KV_UTILS_IMPLEMENTATION" before including this file to get the
  implementation for your compilation unit.

  --------------- Printing -----------------

  After printing a string to an arena, there is nil-termination, but that might
  be overwritten. You can lock the termination by simply incrementing the arena
  base pointer.

  When converting from C string to our string (using e.g "toString"), always put
  a +1 nil terminator (and keep it there as long as the string can be referenced
  ), because we feel like it's just a conversion, and can be converted back.

  All non-nil-terminated strings must be marked explicitly with "non_nil"
*/

#pragma once

#include <stdlib.h> // malloc, free
#include <stdio.h>  // printf
#include <stdarg.h>
#include <stddef.h>
#include "kv-intrinsics.h"

/*
  Compilers
*/

#if __llvm__
#    undef COMPILER_LLVM
#    define COMPILER_LLVM 1
#else
#    undef COMPILER_MSVC
#    define COMPILER_MSVC 1
#endif

#define UNUSED_VAR __attribute__((unused))
#define unused_var __attribute__((unused))

#define internal        static
#define global_variable UNUSED_VAR static
#define global_constant UNUSED_VAR static
#define local_persist   static

#define kiloBytes(value) ((value)*1024LL)
#define megaBytes(value) (kiloBytes(value)*1024LL)
#define gigaBytes(value) (megaBytes(value)*1024LL)
#define teraBytes(value) (gigaBytes(value)*1024LL)

#define arrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define arrayLength arrayCount

#if COMPILER_MSVC
#  define debugbreak __debugbreak()
#else
#  define debugbreak __builtin_trap()
#endif

#if AUTO_INTERNAL
#  define assert(claim) if (!(claim)) { printf("%s:%d: assertion fired!", __FILE__, __LINE__); fflush(stdout); debugbreak; }
#else
#  define assert(claim)
#endif

#define invalidCodePath assert(false)
#define todoErrorReport assert(false)
#define todoIncomplete  assert(false)
#define todoTestMe      assert(false)
#define todoOutlaw      assert(false)
#define todoUnknown     assert(false)
#define invalidDefaultCase default: { assert(false); };
#define breakhere  { int x = 5; (void)x; }

// source: https://groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s
#define PP_NARG(...) PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N
#define PP_RSEQ_N() 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define DUMP_1(x) dump(x)
#define DUMP_2(x, ...) dump(x); DUMP_1(__VA_ARGS__)
#define DUMP_3(x, ...) dump(x); DUMP_2(__VA_ARGS__)
#define DUMP_4(x, ...) dump(x); DUMP_3(__VA_ARGS__)
#define DUMP_5(x, ...) dump(x); DUMP_4(__VA_ARGS__)
#define DUMP_6(x, ...) dump(x); DUMP_5(__VA_ARGS__)
#define DUMP_7(x, ...) dump(x); DUMP_6(__VA_ARGS__)
#define DUMP_8(x, ...) dump(x); DUMP_7(__VA_ARGS__)
#define DUMP_9(x, ...) dump(x); DUMP_8(__VA_ARGS__)
#define DUMP_N(N) CONCATENATE(DUMP_, N)
#define DUMP_NO_NEWLINE(...) DUMP_N(PP_NARG(__VA_ARGS__))(__VA_ARGS__)
#define DUMP(...) DUMP_NO_NEWLINE(__VA_ARGS__, "\n")
// DUMP(a,b) -> DUMP_N(2,a,b)(a,b) -> DUMP_2()

inline void
zeroSize(void *base, size_t size)
{
    u8 *ptr = (u8 *) base;
    while(size--)
        *ptr++ = 0;
}

#define zeroStruct(base, type) zeroSize(base, sizeof(type));
#define zeroOut(base) zeroSize(base, sizeof(base))

struct Arena
{
  u8     *base;
  size_t  used;
  size_t  cap;

  // support backward push
  size_t original_cap;

  i32 temp_count;
};

inline Arena
newArena(void *base, size_t cap)
{
    Arena arena = {};
    arena.cap          = cap;
    arena.base         = (u8 *)base;
    arena.original_cap = cap;
    return arena;
}

inline size_t
getArenaFree(Arena &arena)
{
    size_t out = arena.cap - arena.used;
    return out;
}

inline void *
pushSize(Arena &arena, size_t size, b32 zero = false)
{
  void *out = arena.base + arena.used;
  arena.used += size;
  assert(arena.used <= arena.cap);
  if (zero) zeroSize(out, size);
  return(out);
}

inline void *
pushSizeBackward(Arena &arena, size_t size)
{
  arena.cap -= size;
  assert(arena.used <= arena.cap);
  void *out = arena.base + arena.cap;
  return(out);
}

// NOTE: Apparently ##__VA_ARGS__ is the thing
#define pushStruct(arena, type, ...)    (type *) pushSize(arena, sizeof(type), ##__VA_ARGS__)
#define pushStructBackward(arena, type) (type *) pushSizeBackward(arena, sizeof(type))
#define pushArray(arena, count, type)   (type *) pushSize(arena, (count)*sizeof(type))
#define allocate(arena, x, ...) x = (mytypeof(x)) pushSize(arena, sizeof(*x), ##__VA_ARGS__)
#define allocateArray(arena, count, x, ...) x = (mytypeof(x)) pushSize(arena, (count)*sizeof(*x), ##__VA_ARGS__)

#define pushItems_1(array, index, item) array[index] = item;
#define pushItems_2(array, index, item, ...) array[index] = item; pushItems_1(array, index+1, ##__VA_ARGS__);
#define pushItems_3(array, index, item, ...) array[index] = item; pushItems_2(array, index+1, ##__VA_ARGS__);
#define pushItems_N(N, ...) CONCATENATE(pushItems_, N)

#define pushItems(arena, array, item, ...)     \
  array = (mytypeof(item) *) pushArray(arena, PP_NARG(item, ##__VA_ARGS__), mytypeof(item)); \
  pushItems_N(PP_NARG(item, ##__VA_ARGS__), ##__VA_ARGS__)(array, 0, item, ##__VA_ARGS__)

#define pushItemsAs(...) \
  auto pushItems(##__VA_ARGS__)

inline Arena
subArena(Arena &parent, size_t size)
{
  u8 *base = (u8 *)pushSize(parent, size);
  Arena result = newArena(base, size);
  return result;
}

inline Arena
subArenaWithRemainingMemory(Arena &parent)
{
    Arena result = {};
    auto size = parent.cap - parent.used;
    result.base = (u8 *)pushSize(parent, size);
    result.cap  = size;
    return result;
}

struct TempMemoryMarker
{
    Arena  &arena;
    size_t  original_used;
};


inline TempMemoryMarker
beginTemporaryMemory(Arena &arena)
{
  TempMemoryMarker out = {arena, arena.used};
  arena.temp_count++;
  return out;
}

inline void
endTemporaryMemory(TempMemoryMarker temp)
{
  temp.arena.temp_count--;
  assert(temp.arena.used >= temp.original_used);
  temp.arena.used = temp.original_used;
}

inline void
commitTemporaryMemory(TempMemoryMarker temp)
{
  temp.arena.temp_count--;
}

inline void
checkArena(Arena *arena)
{
    assert(arena->temp_count == 0);
}

inline void
resetArena(Arena &arena, b32 zero=false)
{
  arena.used = 0;
  if (zero) {
    zeroMemory(arena.base, arena.cap);
  }
}

inline void *
pushCopySize(Arena &arena, void *src, size_t size)
{
  void *dst = pushSize(arena, size);
  copyMemory_(src, dst, size);
  return dst;
}

#if COMPILER_MSVC
#    define mytypeof decltype
#else
#    define mytypeof __typeof__
#endif

#define pushCopy(arena, src) (mytypeof(src)) pushCopySize(arena, (src), sizeof(*(src)))
/* #define copyStructNoCast(arena, src) copySize(arena, src, sizeof(*(src))) */
#define pushCopyArray(arena, count, src) (mytypeof(src)) pushCopySize(arena, (src), count*sizeof(*(src)))

inline u8 *getNext(Arena &buffer)
{
  return (buffer.base + buffer.used);
}

/* MARK: String */

inline i32
stringLength(char *string)
{
    i32 out = 0;
    char *c = string;
    while (*c)
    {
        out++;
        c++;
    }
    return out;
}

struct String
{
  char *chars;
  i32   length;                 // note: does not include the nil terminator
  operator bool() {return chars;}
};

typedef Arena StringBuffer;

struct StartString {
  StringBuffer &arena;
  char         *chars;
};

inline StartString
startString(Arena &arena)
{
  char *start = (char *)getNext(arena);
  return {.arena=arena, .chars=start};
};

inline String
endString(StartString start)
{
  String out = {};
  out.chars = start.chars;
  out.length = (i32)((char*)getNext(start.arena) - start.chars);
  int zero = 0;
  pushCopy(start.arena, &zero);
  return out;
}

inline b32
equal(String s, const char *cstring)
{
  if (!s.chars)
  {
    return false;
  }
  else
  {
    i32 at = 0;
    for (;
         at < s.length;
         at++)
    {
      if ((cstring[at] == 0) || (s.chars[at] != cstring[at]))
      {
        return false;
      }
    }
    return (cstring[at] == 0);
  }
}

inline b32
equal(const char *cstring, String s)
{
  return equal(s, cstring);
}

inline b32
equal(String a, String b)
{
    b32 out = true;
    if (a.length != b.length)
        out = false;
    else
    {
        for (int i = 0; i < a.length; i++)
        {
            if (a.chars[i] != b.chars[i])
            {
                out = false;
                break;
            }
        }
    }
    return out;
}

inline b32
equal(String a, char c)
{
  return a.length == 1 && a.chars[0] == c;
}

inline String
toString(const char *c)
{
  String out;
  out.chars  = (char*)c;
  out.length = 0;
  while (*c++) {
    out.length++;
  }
  return out;
}

inline String
toString(Arena &arena, const char *c)
{
  String out = {};
  out.chars = (char *)getNext(arena);
  char *dst = out.chars;
  while ((*dst++ = *c++)) {
    out.length++;
  }
  *dst = '\0';
  pushSize(arena, out.length+1);
  return out;
}

inline char *
toCString(Arena &arena, String string)
{
  char *out = (char *)pushCopySize(arena, string.chars, string.length+1);
  u8 *next = getNext(arena) - 1;
  *next = 0;
  return out;
}

// NOTE: "temporary" means that your string nil terminator might be stomped on in the future
// Useful for passing the string to fopen or something.
inline char *
toCStringTemporary(String string)
{
  assert(*(string.chars + string.length) == 0);
  return string.chars;
}

inline b32
equal(char *s1, char *s2)
{
  todoTestMe;
  b32 out = true;
  char *c1 = s1;
  char *c2 = s2;
  while (*c1++ == *c2++)
  {
    if (*c1 == 0) {
      out = (*c2 == 0);
      break;
    } else if (*c2 == 0) {
      out = (*c1 == 0);
      break;
    }
  }
  return out;
}

internal String
printVA(Arena &buffer, char *format, va_list arg_list)
{
  char *at = (char *)getNext(buffer);
  int printed = vsnprintf(at, (buffer.cap - buffer.used), format, arg_list);
  buffer.used += printed;
  return String{at, printed};
}

internal String
print(Arena &buffer, char *format, ...)
{
  String out = {};

  va_list arg_list;
  va_start(arg_list, format);

  out.chars = (char *)getNext(buffer);
  auto printed = vsnprintf(out.chars, (buffer.cap-1 - buffer.used), format, arg_list);
  buffer.used += printed;
  out.length   = printed;
  buffer.base[buffer.used] = 0; // nil-termination

  va_end(arg_list);

  return out;
}

inline String
print(Arena &buffer, String s)
{
  String out = {};
  {
    out.chars = (char *)getNext(buffer);
    char *at = out.chars;
    const char *c = s.chars;
    for (i32 index = 0; index < s.length; index++)
      *at++ = *c++;
    *at = 0;
    out.length = (i32)(at - out.chars);
    buffer.used += out.length;
    assert(buffer.used <= buffer.cap);
  }

  return out;
}

// inline void  todo: we don't ever wanna use this.
// print(String s)
// {
//   printf("%.*s", s.length, s.chars);
// }

// internal void  todo: just use the "%c" format in printf
// print(Arena &buffer, char character)
// {
//   char *string = (char *)pushSize(buffer, 1);
//   *string = character;
// }

// todo #cleanup same as "inArena"?
inline b32
belongsToArena(Arena *arena, u8 *memory)
{
  return ((memory >= arena->base) && (memory < arena->base + arena->cap));
}

#define maximum(a, b) ((a < b) ? b : a)
#define minimum(a, b) ((a < b) ? a : b)

// Metaprogramming tags
#define forward_declare(FILE_NAME)
#define function_typedef(FILE_NAME)

inline char
toLowerCase(char c)
{
  if (('a' <= c) && (c <= 'z'))
    return c - 32;
  return c;
}

inline b32
isSubstring(String full, String sub, b32 case_sensitive=true)
{
  b32 out = true;
  if (sub.length > full.length)
  {
    out = false;
  }
  else
  {
    for (i32 id = 0;
         id < sub.length;
         id++)
    {
      char s = sub.chars[id];
      char f = full.chars[id];
      b32 mismatch = case_sensitive ? (s != f) : (toLowerCase(s) != toLowerCase(f));
      if (mismatch)
      {
        out = false;
        break;
      }
    }
  }
  return out;
}

inline String
copyString(Arena &buffer, String src)
{
  String out;
  out.chars  = pushCopyArray(buffer, src.length, src.chars);
  out.length = src.length;
  return out;
}

inline void dump() {printf("\n");}
inline void dump(int d) {printf("%d", d);}
inline void dump(char *c) {printf("%s", c);}
inline void dump(String s) {printf("%.*s", s.length, s.chars);}

inline void
concat(String *a, String b)
{
  a->length += b.length;
}

/* MARK: End of String */

inline b32
inArena(Arena &arena, void *p)
{
  return ((u64)p >= (u64)arena.base && (u64)p < (u64)arena.base+arena.cap);
}

inline b32
checkFlag(u32 flags, u32 flag)
{
  return flags & flag;
}

inline void
setFlag(u32 *flags, u32 flag)
{
  *flags |= flag;
}

inline void
unsetFlag(u32 *flags, u32 flag)
{
  *flags &= ~flag;
}

#define SWAP(a, b) { \
    auto temp = a; \
    a = b; \
    b = temp; \
}

#define llPush(arena, member, list)            \
  mytypeof(list) new_list = pushStruct(arena, mytypeof(*list)); \
  new_list->head          = member;             \
  new_list->tail          = list;               \
  list                    = new_list;

// defer macro from https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})
// end defer macro //////////////////

#define EAT_TYPE(POINTER, TYPE) (TYPE *)(POINTER += sizeof(TYPE), POINTER - sizeof(TYPE))

#define DLL_EXPORT extern "C" __attribute__((visibility("default")))


void *xmalloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    perror("xmalloc failed");
    exit(1);
  }
  return ptr;
}

/* 
   Stretchy buffer by Sean Barrett
 */

struct BufferHeader {
  size_t len;
  size_t cap;
  char items[0];
};

#define bufHeader_(buffer) ((BufferHeader *)((char *)(buffer) - offsetof(BufferHeader, items)))
#define doesBufFit_(buffer, new_len) (new_len <= bufCap(buffer))
#define bufFit_(buffer, new_len) doesBufFit_(buffer, new_len) ? 0 : (buffer = (mytypeof(buffer))bufGrow_(buffer, new_len, sizeof(*buffer)))
#define bufLength(buffer) (buffer ? bufHeader_(buffer)->len : 0)
#define bufCap(buffer) (buffer ? bufHeader_(buffer)->cap : 0)
#define bufPush(buffer, item) (bufFit_(buffer, bufLength(buffer)+1)), buffer[bufHeader_(buffer)->len++] = item
#define bufFree(buffer) free(bufHeader_(buffer))

void * bufGrow_(void *buffer, size_t new_len, size_t item_size);

#ifdef KV_UTILS_IMPLEMENTATION
void *
bufGrow_(void *buffer, size_t new_len, size_t item_size)
{
  size_t new_cap = maximum(bufCap(buffer)*2, new_len);
  size_t new_size = sizeof(BufferHeader)+new_cap*item_size;
  BufferHeader *new_header = 0;
  if (buffer) {
    new_header = (BufferHeader *)realloc(bufHeader_(buffer), new_size);
  } else {
    new_header = (BufferHeader *)xmalloc(new_size);
    new_header->len = 0;
  }
  new_header->cap = new_cap;
  buffer = new_header->items;
  assert(bufCap(buffer) >= new_len);
  return buffer;
}
#endif // KV_UTILS_IMPLEMENTATION

/* End of stretchy buffer */
