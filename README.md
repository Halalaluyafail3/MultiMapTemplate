# MultiMapTemplate
A header only multimap library using macros in C.

The header defines the three macros `MAP_DECLARATION`, `MAP_DEFINITION`, and `MAP_FUNCTION_DECLARATIONS`, and includes the headers `stdint.h` and `stddef.h`.

This library assumes `typeof` exists, specifically it relies on `typeof(type)`. If `typeof` is not provided by your compiler, it should be defined as a macro which calls your compiler's version of `typeof` (e.g. `#define typeof(...)__typeof__(__VA_ARGS__)`). If `typeof` is not a feature supported by your compiler, it should be defined as `#define typeof(...)__VA_ARGS__` or something equivalent, and the type provided as the type of the keys to `MAP_DECLARATION` shall be constrained: a pointer to this type may be obtained by adding `*` after the type.

The notation `##Name` means that the name provided to `MAP_DECLARATION`, `MAP_DEFINITION`, or `MAP_FUNCTION_DECLARATIONS` is used as a suffix. The macros `MAP_DECLARATION` and `MAP_DEFINITION` are each only allowed to be invoked once in a translation unit with a given name. In C23 the rules around struct definitions have been relaxed, so in C23 the restriction that `MAP_DECLARATION` must be invoked only once in a translation unit does not exist. If `MAP_DECLARATION` is invoked multiple times in the same translation unit, all of the arguments except `Qualifiers` need to be the same across the invocations of `MAP_DECLARATION`. The types declared previously may be used in a repeated invocation of `MAP_DECLARATION` to ensure that the types are the same.

`MAP_DEFINITION` shall only be invoked at file scope. `MAP_DECLARATION` and `MAP_FUNCTION_DECLARATIONS` may be invoked in block scope; however, the `Qualifiers` need to be valid for such a case (i.e. no `static`) and prior to C23 `MAP_DECLARATION` shall still only be invoked once per translation unit (because of the type compatibility rules). As with normal functions, declarations of functions from these macros inside of an inner scope will not modify any declarations in an outer scope.

An example is provided at the end for how to use this library.

# The MAP_DECLARATION macro

## Synopsis

```c
#define MAP_DECLARATION(Name, Qualifiers, Key, Value, Extra) /* ... */
```

## Description

Declares several structures to represent a map, several functions to manipulate a map, and several typedefs for convenience:
```c
// types are declared in this order, which guarantees which types are complete in certain places
// the parameters Key, Value, and Extra are guaranteed to only be expanded once
// typeof(Key) is used so types such as int(*)(int) can be used without a typedef
typedef struct MapEntry_##Name MapEntry_##Name;
typedef struct Map_##Name Map_##Name;
typedef typeof(Key) MapKey_##Name;

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

Qualifiers MapEntry_##Name **MapFind_##Name(Map_##Name *, MapKey_##Name);
Qualifiers MapEntry_##Name **MapFindNext_##Name(Map_##Name *, MapEntry_##Name *);
Qualifiers MapEntry_##Name **MapAdd_##Name(Map_##Name *, MapKey_##Name);
Qualifiers MapEntry_##Name **MapLocate_##Name(Map_##Name *, MapEntry_##Name *);
Qualifiers void MapRemove_##Name(Map_##Name *, MapEntry_##Name **);
Qualifiers void MapClear_##Name(Map_##Name *);
```

## Argument descriptions

`Qualifiers` is used to control the scope of the functions declarations. The meaning of the qualifiers are equivalent to what they mean with regular functions declarations.

`Keys` is the type of the keys, it shall be a complete object type and not an array type.

`Extra` specifies zero or more structure members at the start of `Map_##Name`. A flexible array member shall not be used. This is intended to be used to provide context to the functions provided to `MAP_DEFINITION`, and can be left empty if not needed.

`Value` specifies zero or more structure members at the end of `MapEntry_##Name`. A flexible array member shall not be used. This is intended to be used to associate the entry with some values, though it can be empty to create a type that is more like a set.

## Structure member descriptions

The map type represents a closed addressing hash table. `MapEntryCount` is the number of entries in the map. `MapBucketsSize` is a power of two greater than or equal to eight, or is zero iff `MapEntryCount` is zero; it represents the sizes of the buckets array. `MapBuckets` is a pointers to the first element in the buckets array, or null if `MapEntryCount` is zero. Each bucket is a pointer to the first entry of the bucket, or null if there are no entries in the bucket. An empty map will have all three of these members equal to zero, in all other cases these members will be non-zero.

Because of how the built in members of `Map_##Name` work, copying each of the built in members works fine as long as the map is not mutated. When the map is mutated, the map used for mutation is the only one that is guaranteed to be valid. Members in the `Extra` argument do not need to obey this.

The entry type represents an entry in a closed addressing hash table. `MapNext` is the pointer to the next entry in the bucket, or null if there are no more entries in the bucket. `MapHash` is the hash of the key in the entry. `MapKey` is the key of this entry.

## Function descriptions

`MapFind_##Name` searches for an entry with the given key. If a key is found in the table, a pointer to the next pointer of the previous entry in the bucket is returned, or a pointer to the pointer that points at the entry in the buckets array. If no entry is found, null is returned. If multiple entries have keys that compare equal to the provided key to search for, the earliest entry in the bucket is returned. A pointer to pointer is returned so `MapRemove_##Name` has enough information to remove the entry without having to hash the key again or needing a doubly linked list. These pointers to pointers reference things beyond the entry itself, and are only valid until an entry is added or removed.

`MapFindNext_##Name` searches for the next entry with the same key as the provided entry. The entry must be in the provided map. The function is similar to `MapFind_##Name`, except it starts searching after the provided entry and the searched key is obtained from the provided entry. If no entry is found, null is returned. The order of entries within a bucket shouldn't be relied up, but it can only change when an entry is inserted or removed. This function does not need information about what points at the provided entry, so it only takes a pointer to the entry.

`MapAdd_##Name` adds an entry to the table with the provided key. If successful, a pointer similar to the result of `MapFind_##Name` is returned, or null if unsuccessful. Members specified in the `Value` argument are not initialized by this function.

`MapLocate_##Name` takes a pointer to an entry and searches through the bucket that it is in to return a pointer similar to the result of `MapFind_##Name`. The entry must be in the provided map. A pointer to an entry is stable for the life of the entry, so a pointer to an entry can be stored even if elements are added or removed from the map. The purpose of this function is to get the extra information of where an entry is in a map, primarily so it can be removed from the map. Similarly to `MapFind_##Name`, the resulting pointer is only valid until an entry is added or removed from the map.

`MapRemove_##Name` removes an entry from the map. This function will never fail. This function takes a pointer to the pointer to the entry so it can quickly remove the entry without needing to search the map.

`MapClear_##Name` clears all entries from the given map. This function is useful for ensuring all resources are cleaned up when the map should be removed.

# The MAP_DEFINITION macro

## Synopsis

```c
#define MAP_DEFINITION(Name, Qualifiers, Hash, IsEqual, Allocate, Free) /* ... */
```

## Description

Prior to `MAP_DEFINITION` being invoked, `MAP_DECLARATION` shall have been invoked with `Name` in the same translation unit. This macro will provide the definitions to the functions declared in `MAP_DECLARATION`. `Qualifiers` is used to control the scope of the function definitions, and has the same meaning as qualifiers of regular function definitions. `Hash`, `IsEqual`, `Allocate`, and `Free` shall be names of functions or function like macros that expand to expressions which evaluate all of their arguments except the `Map` argument exactly once, properly parenthesize the arguments (except the `Map` argument) and operators, and convert the arguments except the `Map` argument to expected input type by implicit or explicit conversion (e.g. `Free` should accept any pointer to non-constant and non-volatile object type).

Synopses for `Hash`, `IsEqual`, `Allocate`, and `Free`:
```c
size_t Hash(MapKey_##Name Key, Map_##Name *Map);
bool IsEqual(MapKey_##Name Key1, MapKey_##Name Key2, Map_##Name *Map);
// a return type that is convertible to bool is acceptable, using bool is not required
void *Allocate(size_t Size, Map_##Name *Map);
void Free(void *Block, Map_##Name *Map);
// the return value of Free is ignored, if a value is returned it'll be ignored
```

`Hash` and `IsEqual` shall be pure functions. Keys that compare equal by a test with `IsEqual` shall have equivalent results from `Hash`. `IsEqual` shall work like a comparison function and follow the basic rules of comparison functions, such as being reflexive (`X==X` is always true), symmetric (`X==Y` implies `Y==X`), and transitive (`X==Y&&Y==Z` implies `X==Z`). These rules will only matter for keys that are inserted into the table or searched for.

`Allocate` and `Free` shall work like the functions `malloc` and `free` with an extra `Map` argument, except that `Free` need not support being called with a null pointer.

All invocations of the functions have sequence points between them. The state of the map's structure members is unspecified during an invocation of one of the functions (the structure members should not be changed), the purpose of the `Map` argument is to provide access to the fields defined in the `Extra` argument provided to `MAP_DECLARATION`. None of the functions should be invoked during an invocation of one of the functions, i.e. the functions are not re-entrant. If a macro invocation does not return, there is no safe way to deal with the map. For these reasons, using `longjmp` to jump outside of the function call or accessing a global map from a function registered with `atexit`, `signal`, or other similar functions should be avoided.

`MapLocate_##Name` is thread safe and signal safe (none of the provided functions will be invoked). `MapFind_##Name` and `MapFindNext_##Name` are thread safe and signal safe unless the `Hash` and `IsEqual` functions are not thread safe or not signal safe (`Allocate` and `Free` will not be invoked). `MapAdd_##Name`, `MapRemove_##Name`, and `MapClear_##Name` are never thread safe or signal safe and require external synchronization to be used from multiple threads or from inside and outside of a signal handler.

# The MAP_FUNCTION_DECLARATIONS macro

## Synopsis

```c
#define MAP_FUNCTION_DECLARATIONS(Name, Qualifiers) /* ... */
```

# Description

Prior to `MAP_FUNCTION_DECLARATIONS` being invoked, `MAP_DECLARATION` shall have been invoked with `Name` in the same translation unit. This macro will declare the functions declare in `MAP_DECLARATION` again. `Qualifiers` is used to control the scope of the function definitions, and has the same meaning as qualifiers of regular function definitions.

# The Qualifiers argument

The purpose of the `Qualifiers` argument is to provide a way of controlling the storage classes of the functions. For example:
```c
// in a header
MAP_DECLARATION(Name, extern, ...)
// in a source file
#include "header.h"
MAP_DEFINITION(Name, extern, ...)
```
Here, using `extern` (or nothing) will require that one translation unit use `MAP_DEFINITION` similar to defining an `extern` function. `static` or `static inline` will require that `MAP_DEFINITION` is provided in that translation unit and the function definitions will not be visible. `inline` definitions are more complicated due to how C requires one translation unit to provide an external declaration:
```c
// in a header
#ifndef DECLARATION_QUALIFIERS
#define DECLARATION_QUALIFIERS inline
#endif
MAP_DECLARATION(Name, DECLARATION_QUALIFIERS, ...)
MAP_DEFINITION(Name, inline, ...)
// in a source file
#define DECLARATION_QUALIFIERS extern
#include "header.h"
```
The `MAP_DECLARATION` needs to be used with `extern` (or something equivalent) in the translation unit that provides the external definition, and since `MAP_DECLARATION` can only be invoked once with a given name in a translation unit (prior to C2X/C23) there needs to be a macro controlling whether extern or inline is used. For this reason, a macro named `MAP_FUNCTION_DECLARATIONS` is provided to make this simpler:
```c
// in a header
MAP_DECLARATION(Name, inline, ...)
MAP_DEFINITION(Name, inline, ...)
// in a source file
#include "header.h"
MAP_FUNCTION_DECLARATIONS(Name, extern)
```

# Reserved identifiers

This header defines the names `MAP_DECLARATION`, `MAP_DEFINITON`, and `MAP_FUNCTION_DECLARATIONS` as macros, these macros shouldn't be defined prior to including the header file. Names used by `MAP_DECLARATION` shouldn't be used as macro names, except `typeof`. Names starting with `MapRESERVED__` are reserved for this header in all contexts.

# Example
```c
#include "MultiMapTemplate.h"
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
