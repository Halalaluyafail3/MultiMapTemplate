# MultiMapTemplate
A header only multimap library using macros in C.

The header defines the two macros `MAP_DECLARATION` and `MAP_DEFINITION` and includes the headers `stdint.h` and `stddef.h`.

This library assumes `typeof` exists, specifically it relies on `typeof(type)`. If `typeof` is not provided by your compiler, it should be defined as a macro which calls your compiler's version of `typeof` (e.g. `#define typeof(...)__typeof__(__VA_ARGS__)`). If `typeof` is not a feature supported by your compiler, it should be defined as `#define typeof(...)__VA_ARGS__` or something equivalent, and the type provided as the type of the keys to `MAP_DECLARATION` shall be constrained: a pointer to this type may be obtained by adding `*` after the type.

The notation `##Name` means that the name provided to `MAP_DECLARATION` or `MAP_DEFINITION` is used as a suffix.

# The MAP_DECLARATION macro

## Synopsis

```c
#define MAP_DECLARATION(Name, Qualifiers, Key, Value, Extra) /* ... */
```

## Description

Declares several structures to represent a map, several functions to manipulate a map, and several typedefs for convenience:
```c
typedef struct MapEntry_##Name MapEntry_##Name;
typedef struct Map_##Name Map_##Name;
typedef typeof(Key) MapKey_##Name; // typeof(Key) is used so types such as int(*)(int) can be used without a typedef

struct MapEntry_##Name {
  MapEntry_##Name *MapNext;
  size_t MapHash;
  MapKey_##Name MapKey;
  Value
};
struct Map_##Name {
  Extra
  size_t MapEntryCount;
  size_t MapBucketsSize;
  MapEntry_##Name **MapBuckets;
};

// Qualifiers
Qualifiers MapEntry_##Name **MapFind_##Name(Map_##Name *, MapKey_##Name);
Qualifiers MapEntry_##Name **MapFindNext_##Name(Map_##Name *, MapEntry_##Name *);
Qualifiers MapEntry_##Name **MapAdd_##Name(Map_##Name *, MapKey_##Name);
Qualifiers MapEntry_##Name **MapLocate_##Name(Map_##Name *, MapEntry_##Name *);
Qualifiers void MapRemove_##Name(Map_##Name *, MapEntry_##Name **);
Qualifiers void MapClear_##Name(Map_##Name *)
```

# The MAP_DEFINITION macro

## Synopsis

```c
#define MAP_DEFINITION(Name, Qualifiers, Hash, IsEqual, Allocate, Free)
```

## Description

`MAP_DECLARTION` shall be invoked with `Name` prior to invoking `MAP_DEFINITION` in the same translation unit. `Qualifiers` shall be equivalent to `Qualifiers` argument provided to `MAP_DECLARATION`. `Hash`, `IsEqual`, `Allocate`, and `Free` shall be names of functions or function like macros that exapnd to expressions which evaluate all of their arguments except the Map argument exactly once, properly parenthesize the arguments and operators, and convert the arguments except the Map argument to expected input type by implicit or explicit conversion (e.g. `Free` should accept any pointer to non-constant type).

Synopses for `Hash`, `IsEqual`, `Allocate`, and `Free`:
```c
size_t Hash(MapKey_##Name Key, Map_##Name *Map);
bool IsEqual(MapKey_##Name Key1, MapKey_##Name Key2, Map_##Name *Map);
// a return type that is convertible to bool is acceptable, using bool is not required
void *Allocate(size_t Size, Map_##Name *Map);
void Free(void *Block, Map_##Name *Map);
// the return value of Free is ignored, if a value is returned it'll be ignored
```

`Hash` and `IsEqual` shall be pure functions. Keys that compare equal by a test with `IsEqual` shall have equivalent results from `Hash`. `IsEqual` shall work like a comparison function and follow the basic rules of comparison functions, such as being reflexive (`X==X` is always true), symmetric (`X==Y` implies `Y==X`), and transitive (`X==Y&&Y==Z` implies `X==Z`).

`Allocate` and `Free` shall work like the functions `malloc` and `free` with an extra `Map` argument.

# Example
```c
#include "Library/MultiMapTemplate.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static size_t HashString(const char *String) { // basic string hasher
  size_t Result = strlen(String);
  for (size_t Index = 0; String[Index]; ++Index) {
    Result ^= (Result << 5) + (Result >> 2) + String[Index];
  }
  return Result;
}
// static because we only need it in this file
// keys are pointers to null terminated strings
// each entry gets an associated Value (of type int)
// and no extra data needs to be stored in the map itself
MAP_DECLARATION(I, static, const char *, int Value;, )
// the map parameter doesn't need to be evaluated
#define Hash(String, Map) HashString(String)
#define IsEqual(String1, String2, Map) (!strcmp(String1, String2))
#define Allocate(Size, Map) malloc(Size)
#define Free(Pointer, Map) free(Pointer)
MAP_DEFINITION(I, static, Hash, IsEqual, Allocate, Free)
#undef Hash // undefine the names afterward since they aren't needed
#undef IsEqual
#undef Allocate
#undef Free
static MapEntry_I **Insert(Map_I *Map, MapKey_I Key, int Value) {
  MapEntry_I **Entry = MapAdd_I(Map, Key); // add the entry
  if (!Entry) { // for this example handling an error isn't important
    exit(EXIT_FAILURE);
  }
  (*Entry)->Value = Value; // and set the value
  return Entry;
}
int main(void) {
  Map_I Map = {0}; // create the map and initialize it to be empty
  MapEntry_I *Entry = *Insert(&Map, "a", 1); // a=1, save pointer to it
  Insert(&Map, "b", 2);                      // b=2
  Insert(&Map, "c", 3);                      // c=3
  Insert(&Map, "d", 4);                      // d=4
  MapRemove_I(&Map,
              MapLocate_I(&Map, Entry));   // since the map changed, find the
                                           // entry again, then remove it
  MapRemove_I(&Map, MapFind_I(&Map, "c")); // remove c
  Insert(&Map, "b", 3); // b=3, multiple values for the same key may be used
  Entry = *MapFind_I(&Map, "b"); // find first b
  printf("%i %i\n", Entry->Value,
         (*MapFindNext_I(&Map, Entry))->Value); // prints 2 and 3 in any order
  MapClear_I(&Map);                             // clear the map
  return 0;
}
```
