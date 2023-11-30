#pragma once

#include <cstdint>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
// #include "stdlib.h"

struct BufferHeader {
  size_t len;
  size_t cap;
  void *items[0];
};

#define bufHeader_(buffer) ((BufferHeader *)((char *)(buffer) - offsetof(BufferHeader, items)))
#define doesBufFit_(buffer, new_len) (new_len <= bufCap(buffer))
#define bufFit_(buffer, new_len) doesBufFit_(buffer, new_len) ? 0 : (buffer = (mytypeof(buffer))bufGrow_(buffer, new_len, sizeof(*buffer)))
#define bufLen(buffer) (buffer ? bufHeader_(buffer)->len : 0)
#define bufCap(buffer) (buffer ? bufHeader_(buffer)->cap : 0)
#define bufPush(buffer, item) (bufFit_(buffer, bufLen(buffer)+1)), buffer[bufHeader_(buffer)->len++] = item
#define bufFree(buffer) free(bufHeader_(buffer))

void * bufGrow_(void *buffer, size_t new_len, size_t item_size);

//
// Compilers
//

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

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  i32;
typedef long     i64;
typedef int8_t   b8;
typedef int32_t  b32;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    r32;
typedef float    f32;  // todo Not sure why we don't just use this?

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

#if REA_INTERNAL
#  define assert(claim) if (!(claim)) { printf("%s:%d: assertion fired!", __FILE__, __LINE__); fflush(stdout); crashTheProgram; }
#else
#  define assert(claim)
#endif

#define invalidCodePath assert(false)
#define todoErrorReport assert(false)
#define todoIncomplete  assert(false)
#define todoOutlaw      assert(false)
#define todoUnknown     assert(false)
#define invalidDefaultCase default: { assert(false); };
#define breakhere  { int x = 5; (void)x; }

#include "kv-intrinsics.h"

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

    i32 temp_count;
};

typedef Arena StringBuffer;

inline Arena
newArena(size_t cap, void *base)
{
    Arena arena = {};
    arena.cap = cap;
    arena.base = (u8 *)base;
    return arena;
}

inline size_t
getArenaFree(Arena *arena)
{
    size_t out = arena->cap - arena->used;
    return out;
}

inline void *
pushSize(Arena &arena, size_t size, b32 zero = false)
{
    void *out = (arena.base + arena.used);
    arena.used += size;
    assert(arena->used <= arena->cap);
    if (zero) zeroSize(out, size);
    return(out);
}

// todo: clang broke __VA_ARGS__!
#define pushStruct(arena, type) (type *) pushSize(arena, sizeof(type))
#define pushArray(arena, count, type) (type *) pushSize(arena, (count)*sizeof(type))
#define allocate(arena, x, ...) x = (mytypeof(x)) pushSize(arena, sizeof(*x), __VA_ARGS__)
#define allocateArray(arena, count, x, ...) x = (mytypeof(x)) pushSize(arena, (count)*sizeof(*x), __VA_ARGS__)

#define pushItems_1(array, index, item) array[index] = item;
#define pushItems_2(array, index, item, ...) array[index] = item; pushItems_1(array, index+1, __VA_ARGS__);
#define pushItems_3(array, index, item, ...) array[index] = item; pushItems_2(array, index+1, __VA_ARGS__);
#define pushItems_N(N, ...) CONCATENATE(pushItems_, N)

#define pushItems(arena, array, item, ...)     \
  array = (mytypeof(item) *) pushArray(arena, PP_NARG(item, __VA_ARGS__), mytypeof(item)); \
  pushItems_N(PP_NARG(item, __VA_ARGS__), __VA_ARGS__)(array, 0, item, __VA_ARGS__)

#define pushItemsAs(...) \
  auto pushItems(__VA_ARGS__)

inline Arena
subArena(Arena &parent, size_t size)
{
    Arena result = {};
    result.base = (u8 *)pushSize(parent, size);
    result.cap  = size;
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
  assert(temp.arena->used >= temp.original_used);
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

// todo re-implement these
// inline void
// resetArena(Arena *arena, b32 zero=false)
// {
//   arena->used = 0;
//   if (zero)
//     zeroMemory(arena->base, arena->cap);
// }

// inline void *
// copySize(Arena *arena, void *src, size_t size)
// {
//   void *dst = pushSize(arena, size);
//   copyMemory(dst, src, size);
//   return dst;
// }

#if COMPILER_MSVC
#    define mytypeof decltype
#else
#    define mytypeof __typeof__
#endif

#define pushCopy(arena, src) (mytypeof(src)) copySize(arena, (src), sizeof(*(src)))
/* #define copyStructNoCast(arena, src) copySize(arena, src, sizeof(*(src))) */
#define copyArray(arena, count, src) (mytypeof(src)) copySize(arena, (src), count*sizeof(*(src)))

// inline b32
// inRange(i32 min, i32 val, i32 max)
// {
//     return (min <= val) && (val <= max);
// }

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

inline void *
getNext(Arena &buffer)
{
  return (buffer.base + buffer.used);
}

struct StartString {
  StringBuffer &buffer;
  char         *chars;
};

inline StartString
startString(StringBuffer &buffer)
{
  char *start = (char *)(buffer.base + buffer.used);
  return {.buffer=buffer, .chars=start};
};

inline String
endString(StartString start)
{
  String out = {};
  out.chars = start.chars;
  out.length = (i32)((char*)getNext(start.buffer) - start.chars);
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
  while (*c) {
    out.length++;
    c++;
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
  pushSize(arena, out.length);
  return out;
}

inline b32
equal(char *s1, char *s2)
{
  b32 out = true;
  char *c1 = s1;
  char *c2 = s2;
  while (true)
  {
    if (*c1 != *c2)
    {
      out = false;
      break;
    }
    else
    {
      if (*c1 == 0)
        break;
      else
      {
        c1++;
        c2++;
      }
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
    assert(buffer->used <= buffer->cap);
  }

  return out;
}

inline void
print(String s)
{
  printf("%.*s", s.length, s.chars);
}

// todo #cleanup same as "inArena"?
inline b32
belongsToArena(Arena *arena, u8 *memory)
{
  return ((memory >= arena->base) && (memory < arena->base + arena->cap));
}

#define maximum(a, b) ((a < b) ? b : a)
#define minimum(a, b) ((a < b) ? a : b)

// Metaprogramming tags
#define forward_declare
#define embed_struct

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

inline void dump() {printf("\n");}
inline void dump(int d) {printf("%d", d);}
inline void dump(char *c) {printf("%s", c);}
#if 0
inline void dump(String s) {print(0, s);}
#endif


inline b32
inArena(Arena *arena, void *p)
{
  return ((u64)p >= (u64)arena->base && (u64)p < (u64)arena->base+arena->cap);
}

// todo Reimplement
// inline String
// copyString(Arena *buffer, String src)
// {
//   String out;
//   out.chars  = copyArray(buffer, src.length, src.chars);
//   out.length = src.length;
//   return out;
// }

inline void
concat(String *a, String b)
{
  a->length += b.length;
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

// defer macro ////////////////////
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
