# MultiMapTemplate
A header only multimap library using macros in C. The header defines the three macros `MAP_DECLARATION`, `MAP_DEFINITION`, and `MAP_FUNCTION_DECLARATIONS`, and includes the headers `stdint.h` and `stddef.h`. Including the header after it has already been included has no effect. An example is provided at the end for how to use this library.

This library assumes `typeof` exists, specifically it relies on `typeof(type)`. If `typeof` is not provided by the compiler, it should be defined as a macro which uses the compiler's version of `typeof` (e.g. `#define typeof(...)__typeof__(__VA_ARGS__)` or `#define typeof __typeof__`). If `typeof` is not a feature supported by the compiler, it should be defined as `#define typeof(...)__VA_ARGS__` or something equivalent, and the type provided as the type of the keys to `MAP_DECLARATION` shall be constrained: a pointer to this type may be obtained by adding `*` after the type. In any case, such a macro only needs to be defined during an expansion of `MAP_DECLARATION` and any restrictions imposed by the macro only apply to invocations of `MAP_DECLARATION` done while the macro is defined. When compiling in a C23 compiler `typeof` shall not be defined as a macro because it is a standard keyword (prior to including the header or prior to expanding any of the three macros, similar to how the standard headers work).

# Macro definitions
The notation `##Name` means that the name provided to one of the three macros is used as a suffix without doing macro expansions. For example, it is valid to use `I` as a name even when it is defined as a macro after including `complex.h`. If expanding the argument is desired, this can be done by defining a macro that forwards its arguments to one of the three macros then invoking that new macro. A parameter named `Name` in any of the three macros will only be used as the second operand in a `##` operator where the first operand is an identifier which consists of only digits, letters, and underscores. A parameter named `Name` in any of the three macros shall either be empty or a single preprocessing token which can be concatenated with any identifier (which consists only of digits, letters, and underscores) where `Name` is the second operand of `##`. For example, in a C23 compiler using a preprocessing token that consists of only a combining diaeresis is not valid since concatenating it with some identifiers would result in an identifier which doesn't conform to Normalization Form C.

Each of the three macros expand to sequences of preprocessing tokens which can be converted into tokens. These tokens shall be valid to form one or more declarations which can be valid at file scope or block scope, except for `MAP_DEFINITION`. The tokens which `MAP_DEFINITION` expands to shall be valid to form one or more declarations or function definitions which can be valid at file scope. If the GCC local function extension is supported, the declarations or function definitions can also be valid at block scope. Notably, the tokens include a semicolon at the end if it is necessary so a semicolon should not be placed after an invocation of any of the three macros. It is unspecified how many commas are included in the sequence of preprocessing tokens outside of any parentheses, if the preprocessing tokens need to passed through multiple macros this should be done with variadic parameters or other means to avoid issues with unparenthesized commas. Rescanning the sequence of preprocessing tokens for macro expansions will have no effect (whitespace separation being changed counts as an effect). Moreover, the first preprocessing token will not be an opening parenthesis and the last preprocessing token will not be an identifier (in some situations those preprocessing tokens could introduce macro expansions).

All macro parameters except `Name` in any of the three macros shall be sequences of preprocessing tokens which can be converted into tokens and where rescanning for macro expansions has no effect. Any macro parameter may contain unparenthesized commas (which can only be done via macros that expand to unparenthesized commas) if they meet the other requirements of the macro parameter. Hereinafter, all macros and macro parameters will be referred to the grammatical constructs which they expand to instead of just preprocessing tokens.

# The MAP_DECLARATION macro

## Synopsis
```c
#define MAP_DECLARATION(Name, Qualifiers, Key, Value, Extra) /* ... */
```

## Description
Expands to a sequence of one or more declarations which contains at least the following declarations, in the described order:
```c
// types are declared in this order, which guarantees which types are complete in certain places
struct MapEntry_##Name;
struct Map_##Name;
// the first structure declarations ensure that in the case of a redeclaration in an inner scope
// these typedefs below will refer to the structures inside of the inner scope not the outer scope
// if this was not done, redeclaring again in the same inner scope would cause an error because of typedefs
// that use different (even though compatible in C23) types
typedef struct MapEntry_##Name MapEntry_##Name;
typedef struct Map_##Name Map_##Name;
typedef typeof(Key) MapKey_##Name;
// typeof(Key) is used so types such as int(*)(int) can be used without a typedef

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
Extra declarations may be added and the specified declarations can be changed while retaining the same meaning, it is not correct to assume this is the exact spelling (e.g. extra whitespace can be added). Any extra declarations added will not impose further restrictions on how the macro can be used, aside from the spelling. Notably, there may be extra function declarations; however, these must use `Qualifiers` as well. Furthermore, all extra declarations will use identifiers reserved to this library to avoid conflict with other declarations and macros. If all function declarations need to be redeclared, use `MAP_FUNCTION_DECLARATIONS` which will include all extra function declarations as well. The structure declarations are exact, extra members cannot be added.

If the declarations from multiple `MAP_DECLARATION` invocations with the same `Name` are used in different translation units where `Qualifiers` causes the function declarations to have external linkage, all declared types must be compatible. When compiling in a C23 compiler, it is valid to use the declarations from multiple `MAP_DECLARATION` invocations with the same `Name` within a translation unit as long as such invocations result in all declared types being compatible. If the declarations from multiple `MAP_DECLARATION` invocations with the same `Name` are used in the same scope, then all declared types must be the same instead of just being compatible.

## Argument descriptions
The arguments provided for `Key`, `Value`, and `Extra` are guaranteed to be expanded only once.

`Qualifiers` is used to control the linkage of the functions declarations. The meaning of the qualifiers are equivalent to what they mean with regular functions declarations.

`Key` is a type name that defines the type of the keys, it shall be a type that can be used as the type of a structure member and it shall be a type where an lvalue with the same type is a modifiable lvalue.

`Extra` specifies zero or more structure members at the start of `Map_##Name`. A flexible array member shall not be used. This is intended to be used to provide context to the functions provided to `MAP_DEFINITION`, and can be left empty if not needed.

`Value` specifies zero or more structure members at the end of `MapEntry_##Name`. A flexible array member shall not be used. This is intended to be used to associate the entry with some values, though it can be empty to create a type that is more like a set.

## Structure member descriptions
The map type represents a closed addressing hash table. `MapEntryCount` is the number of entries in the map. `MapBucketsSize` is an integer power of two which is greater than or equal to eight, or is zero if and only if `MapEntryCount` is zero; it represents the size of the buckets array. `MapBuckets` is a pointer to the first element in the buckets array, or null if and only if `MapEntryCount` is zero. Each element in the buckets array is a pointer to the first entry of the bucket, or null if there are no entries in the bucket. An empty map will have all three of these members equal to zero, in all other cases these members will be non-zero (here, a null pointer is called zero). These three members are collectively referred to as the built in members of the map.

For simplicity, the wording here treats the buckets array as if it is a complete array. However, it is only required that `MapBuckets` be a pointer such that pointer arithmetic and accesses to objects that would be valid with an array of exactly `MapBucketsSize` elements are valid. Therefore it is valid to have `MapBuckets` point into the middle of an array, and it is valid to have `MapBucketSize` be smaller than the amount of available space (i.e. the array can be larger than necessary).

The entry type represents an entry in a closed addressing hash table. `MapNext` is the pointer to the next entry in the bucket, or null if there are no more entries in the bucket (a.k.a. the next pointer). `MapHash` is the hash of the key in the entry. `MapKey` is the key of this entry. These three members are collectively referred to as the built in members of the entry.

Each element in the buckets array of a map forms a singly linked list of entries, which shall be acyclic. Each of these singly linked lists can be traversed by using the `MapNext` members of the entries. Collectively an element in the buckets array and all of the entries in its singly linked list (if any) are referred to as a bucket. The hash of an entry determines which bucket it will be placed in, specifically the expression `MapHash%MapBucketsSize` is used as an offset from the first bucket. An entry shall not be in any other bucket in the same map. Multiple maps can use the same entries, however using any of the built in functions which mutate any of the maps may leave the other maps an invalid state and therefore cause undefined behavior when using them. Similarly, multiple maps can share the same buckets array and using any of the built in functions which mutate any of the maps may leave the other maps in an invalid state.

Because of how the built in members of `Map_##Name` work, copying each of the built in members works fine as long as the map is not mutated. When the map is mutated by using one of the built in functions that will mutate the map, the map used for mutation is the only one that is guaranteed to be valid. Members in the `Extra` argument do not need to obey this.

The built in members can be manually modified, however if they are set to something invalid (e.g. setting `MapEntryCount` to zero but keeping `MapBucketsSize` and `MapBuckets` non-zero, or `MapEntryCount` being inaccurate) then using the map with any of the built in functions will result in undefined behavior. Moreover, if any element of the buckets array is set to something invalid, or if the built in members of any entry in any bucket (e.g. making an entry's next pointer point at itself) are set to something invalid then using the map with any of the built in functions will result in undefined behavior.

## Function descriptions
The functions described here are collectively referred to as the built in functions of the map. While removing an entry it is necessary to update the next pointer of the previous entry in the bucket, or the pointer in the buckets array if it is the first entry in the bucket. To achieve this without needing a doubly linked list or searching the bucket which the entry is in, a pointer to this pointer is used which is referred to as a prev-pointer to the entry. A prev-pointer references things beyond the entry which it refers to, and therefore may become invalid whenever the map is mutated by using one of the built in functions to mutate the map. The built in functions will only invalidate a pointer to an entry when that entry is removed from the map.

`MapFind_##Name` searches for an entry with the given key. If such an entry is found a prev-pointer to the entry is returned, otherwise null is returned. If multiple entries have keys that compare equal to the provided key to search for, the earliest entry in the bucket is returned.

`MapFindNext_##Name` searches for the next entry in the bucket with the same key as the provided entry. The entry must be in the provided map. If another entry is found a prev-pointer to the next entry is returned, otherwise null is returned. The order of entries within a bucket shouldn't be relied up, but will not be changed by built in functions which do not mutate the map. This function does not need information about what points at the provided entry, so it only takes a pointer to the entry.

`MapAdd_##Name` adds an entry to the table with the provided key. If successful a prev-pointer to the new entry is returned, otherwise null is returned. Members specified in the `Value` argument are not initialized by this function.

`MapLocate_##Name` takes a pointer to an entry and searches through the bucket that it is in to return a prev-pointer to the entry. The entry must be in the provided map. The purpose of this function is to be able to keep a pointer to an entry, then later to remove it from the map.

`MapRemove_##Name` removes an entry (specified using a prev-pointer) from the map. This function will never fail.

`MapClear_##Name` clears all entries from the given map. This function is useful for ensuring all resources are cleaned up when the map should be removed.

The built in functions `MapAdd_##Name`, `MapRemove_##Name`, and `MapClear_##Name` will mutate the provided map, the other built in functions will not. Mutating the map includes changing any of the built in members of the map, any element of the buckets array, or any of the built in members of any entry in the map. For example, it is valid to use `MapFind_##Name` on a map declared with const.

# The MAP_DEFINITION macro

## Synopsis
```c
#define MAP_DEFINITION(Name, Qualifiers, Hash, IsEqual, Allocate, Free) /* ... */
```

## Description
`MAP_DEFINITION` expands to a sequence of one or more declarations or function definitions. These declarations and function definitions are only valid if the declarations from expanding `MAP_DECLARATION` with `Name` were placed before such that they are visible or were shadowed by equivalent declarations. This macro will provide the definitions to the functions declared by `MAP_DECLARATION`. `Qualifiers` is used to control the linkage of the function definitions, and has the same meaning as qualifiers of regular function definitions.

`Hash`, `IsEqual`, `Allocate`, and `Free` shall be identifiers that refer to functions or function like macros that expand to expressions which evaluate all of their arguments except the `Map` argument exactly once, properly parenthesize the arguments (except the `Map` argument) and operators, and convert the arguments except the `Map` argument to their expected input type by implicit or explicit conversion (e.g. `Free` should accept any pointer to an unqualified object type). A function like macro expanding to an expression means that it expands to a sequence of preprocessing tokens which can be converted to tokens that form an expression if all arguments are also sequences of preprocessing tokens which can be converted to tokens that form valid expressions. A function like macro expanding to an expression only needs to work with sequences of preprocessing tokens where after scanning for macro expansions once any further rescanning has no effect, and it shall expand to sequence of preprocessing tokens such that rescanning for macros has no effect. Hereinafter, `Hash`, `IsEqual`, `Allocate`, and `Free` are referred to as functions even if they are function like macros. Furthermore, they are also collectively referred to as the argument functions.

Synopses for `Hash`, `IsEqual`, `Allocate`, and `Free`:
```c
size_t Hash(MapKey_##Name Key, Map_##Name *Map);
bool IsEqual(MapKey_##Name Key1, MapKey_##Name Key2, Map_##Name *Map);
// a return type that is convertible to bool is acceptable, using bool is not required
void *Allocate(size_t Size, Map_##Name *Map);
void Free(void *Block, Map_##Name *Map);
// the return value of Free is ignored, if a value is returned it'll be ignored
```

`Hash` and `IsEqual` shall be pure functions within a map, i.e. within a map the meaning of hashing and equality must be consistent (not to require that they can't have side effects). `IsEqual` shall work like a comparison function and follow the basic rules of comparison functions, such as being reflexive (`X==X` is always true), symmetric (`X==Y` implies `Y==X`), and transitive (`X==Y&&Y==Z` implies `X==Z`). These rules will only matter for keys that are inserted into the table or are searched for. `IsEqual` will only be invoked when comparing two values that have the same result from `Hash`. Hence, if it is known that no two values have the same hash then `IsEqual` can be implemented as always returning true.

`Allocate` and `Free` shall work like the functions `malloc` and `free` with an extra `Map` argument, except that `Free` need not support being called with a null pointer. To work like `malloc` and `free` requires the same semantics from the result but does not require that `Allocate` and `free` can be used together, nor that `malloc` and `Free` can be used together (using `Allocate` and `Free` together must be valid). Additionally, if the members specified in the `Value` argument cause `MapEntry_##Name` to have an extended alignment requirement then an invocation of `Allocate` which returns a non-null pointer shall also meet the alignment requirements of `MapEntry_##Name` if the requested size is greater than or equal to the size of `MapEntry_##Name`.

All invocations of the argument functions have sequence points between them, and all of the provided arguments will not contain unparenthesized commas (such as `1?2,3:4`) that would interfere with how function like macros interpret arguments (however, no such restriction applies to the results of the macros). The state of the map's built in members, the elements of the map's buckets array, and the built in members of all entries in the map are unspecified during an invocation of any of the built in functions which mutate the map (the built in members should not be used inside of an argument function). The purpose of the `Map` argument is to provide access to the fields defined in the `Extra` argument provided to `MAP_DECLARATION`. None of the built in functions should be invoked on the same map and in the same thread during an invocation of one of the built in functions, i.e. the functions are not reentrant. If an argument function does not return during an invocation of a built in function which mutates the map, there is no safe way to deal with the map. For these reasons, using `longjmp` to jump outside of the function call or accessing a global map from a function registered with `atexit`, `signal`, or other similar functions (if one of the argument functions might invoke `exit`, `raise`, etc.) should be avoided.

Built in functions are limited to only invoke specific argument functions. `MapLocate_##Name` will not invoke any argument functions. `MapFind_##Name` will only invoke `Hash` and `IsEqual`. `MapFindNext_##Name` will only invoke `IsEqual`. `MapAdd_##Name` will only invoke `Allocate`, `Free`, and `Hash`. `MapRemove_##Name` will only invoke `Allocate` and `Free`. `MapClear_##Name` will only invoke `Free`. Each invocation of a built in function can invoke the argument functions it is permitted to invoke any number of times (including zero). When invoking a built in function that can invoke `Free`, the buckets array shall have been allocated such that it can be used with `Free`. When invoking `MapRemove_##Name`, the removed entry shall have been allocated such that it can be used with `Free`. When invoking `MapClear_##Name`, all entries shall been allocated such that they can be used with `Free`. If any of the built in functions invoke `Free` to deallocate the buckets array a new buckets array will be created only using `Allocate`, except if the map becomes empty in which case there will be no buckets array. `MapAdd_##Name` will create new entires only using `Allocate`, and the `MapHash` member of the new entry will be set to the result of `Hash` for the given key. `MapFind_##Name` and `MapFindNext_##Name` assume that the `MapHash` members of all entries is set to a value equal to using `Hash`, if the hash is incorrect then the entry is impossible to find. Once an invocation of one of the built in functions which mutates the map is complete, the map, the buckets array (if it exists), and all entries in the map (if any) are guaranteed to be in a valid state.

None of the built in functions are signal safe, and none of the built in functions will introduce data races by accessing objects outside of the map, the buckets array (if it exists), and all entries in the map (if any). If multiple threads are invoking built in functions on the same map (without synchronization), or different maps that refer to the same buckets array or entries then if at least one of those invocations was modifying a map the result may be a data race.

# The MAP_FUNCTION_DECLARATIONS macro

## Synopsis
```c
#define MAP_FUNCTION_DECLARATIONS(Name, Qualifiers) /* ... */
```

## Description
Expands to a sequence of one or more declarations. These declarations are only valid if the declarations from expanding `MAP_DECLARATION` with `Name` were placed before such that they are visible or were shadowed by equivalent declarations. This macro will declare the functions declared in `MAP_DECLARATION` again. `Qualifiers` is used to control the linkage of the function definitions, and has the same meaning as qualifiers of regular function definitions.

# The Qualifiers argument
The purpose of the `Qualifiers` argument is to provide a way of controlling the storage classes of the functions, and the choice of inline. For example:
```c
// in a header
MAP_DECLARATION(Name, extern, ...)
// in a source file
#include "header.h"
MAP_DEFINITION(Name, extern, ...)
```
Here, using `extern` (or nothing) will require that one translation unit use `MAP_DEFINITION` similar to defining an `extern` function. `static` or `static inline` will require that `MAP_DEFINITION` is provided in that translation unit and the function definitions will not be visible in other translation units. `inline` definitions are more complicated due to how C requires one translation unit to provide an external declaration:
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
The `MAP_DECLARATION` needs to be used with `extern` (or something equivalent) in the translation unit that provides the external definition, and since `MAP_DECLARATION` can only be invoked once with a given name in a translation unit (prior to C23) there needs to be a macro controlling whether extern or inline is used. For this reason, the macro `MAP_FUNCTION_DECLARATIONS` is provided to make this simpler:
```c
// in a header
MAP_DECLARATION(Name, inline, ...)
MAP_DEFINITION(Name, inline, ...)
// in a source file
#include "header.h"
MAP_FUNCTION_DECLARATIONS(Name, extern)
```
The GCC local function extension can be used with qualifiers by specifying `auto`, if this is used then the `Hash`, `IsEqual`, `Allocate`, and `Free` arguments of `MAP_DEFINITION` must ensure that nothing is captured.

# Reserved identifiers
This header defines the identifiers `MAP_DECLARATION`, `MAP_DEFINITON`, and `MAP_FUNCTION_DECLARATIONS` as macros, these macros shouldn't be defined prior to including the header file for the first time. Identifiers used by `MAP_DECLARATION` in its description for a given `Name` and keywords shouldn't be defined as macros when any of the three macros are being expanded with the same `Name`, except `typeof` when not compiling in a C23 compiler. Identifiers starting with `MapRESERVED__` are reserved for this header in all contexts.

All of the three macros can expand to include identifiers defined by the headers `stdint.h` and `stddef.h`, if declarations are added which shadow these identifiers then they shall not change the meaning. For example, `typedef int size_t;` is valid at block scope but none of the three macros can be used afterwards within that block (not counting nested blocks where the original definition could be restored).

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
// static because it is only needed in this file
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
#undef Hash // undefine the macros afterward since they aren't needed
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
