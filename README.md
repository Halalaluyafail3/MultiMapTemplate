# MultiMapTemplate
A header only multimap library using macros in C.

The header defines the two macros `MAP_DECLARATION` and `MAP_DEFINITION` and includes the headers `stdint.h` and `stddef.h`.

This library assumes `typeof` exists, specifically it relies on `typeof(type)`. If `typeof` is not provided by your compiler, it should be defined as a macro which calls your compiler's version of `typeof`. If `typeof` is defined as `#define typeof(...)__VA_ARGS__` (because `typeof` is not supported) or something equivalent, then the type provided as the type of the keys shall be specified such that the type of a pointer to a key can be obtained by postfixing a `*` to the type of the keys.

Hereinafter the notation `##Name` is used to indicate that `Name` is used as a suffix.

# The MAP_DECLARATION macro
Declares a map using the given name and types.

## Synopsis
```c
#define MAP_DECLARATION(Name,Key,Value,Extra) /* ... */
```

## Description
`Name` is used to identify different maps, all globally declared entities will use it as a suffix.<br>
`Key` is the type of the keys, which shall be a complete object type other than an array type.<br>
`Value` and `Extra` are both series of declarations which shall be valid inside of a `struct`, but no flexible array members may be specified.

This macro declares all types, structures, and functions later described.

## Constraints
The objects declared by `Value` and `Extra` shan't have names which conflict with the predefined names of the structs that they are in.

In each translation unit `MAP_DECLARATION` shall only be invoked once per name, duplicate calls are not allowed in the same translation unit.

All invocations of `MAP_DECLARATION` with the same name shall specify compatible types for `Key`. There shall be a one to one correspondence for the `Value` and `Extra` declaration lists between all invocations of `MAP_DECLARATION`, following the same rules as struct compatibility requirements.

This macro shall only be used at the global scope.

# The MAP_DEFINITION macro
Defines a map using the given name and functions.

## Synopsis
```c
#define MAP_DEFINITION(Name,Hash,IsEqual,Allocate,Free) /* ... */
```

## Description
`Name` is used to identify different maps, all globally defined entities will use it as a suffix.<br>
`Hash`, `IsEqual`, `Allocate`, and `Free` shall all be macro names or function names which take the following arguments:<br>
`Hash`: A key, and a pointer to a map.<br>
`IsEqual`: Two keys, and a pointer to a map.<br>
`Allocate`: A `size_t`, and a pointer to a map.<br>
`Free`: A pointer implicitly convertible to `void*`, and a pointer to a map.<br>
They shall also return the following types:<br>
`Hash`: A `size_t`.<br>
`IsEqual`: A value which is valid for an `if` condition.<br>
`Allocate`: A `void*`.<br>
`Free`: Any type, even incomplete types such as `void`.

This macro defines all functions declared by `MAP_DECLARATION`.

## Constraints
When `Hash`, `IsEqual`, `Allocate`, or `Free` are provided as macros they shall be fully protected by parentheses, and evaluate each of their arguments exactly once except for the pointer to map argument which may be evaluated any number of times.

There are some restrictions on the functions or macros provided:<br>
`Hash`: All keys passed to `MapAdd_##Name` and `MapFind_##Name` shall be valid to pass to `Hash`. This function or macro must be pure.<br>
`IsEqual`: All keys passed to `MapAdd_#Name` and `MapFind_##Name` shall be valid to pass as one of the arguments. This function or macro must be pure, and uphold the reflexive, symmetric, and transitive properties.<br>
`Allocate`: If a non-null pointer is returned, it shall be valid until `Free` is called for the pointer. It shall allocate at least the amount of bytes requested and the pointer shall be suitably aligned for any object with the alignment of a fundemental type.

For keys which are never passed to `MapAdd_##Name` or `MapFind_##Name` whether they make sense in `Hash` or `IsEqual` need not be considered, e.g. with float keys if NaN is never inserted or searched for then `IsEqual` need not make sure that NaN compares equal to NaN and `Hash` need not make sure that all NaN values hash to the same value.

`MAP_DEFINITION` shall only be invoked once per name. `MAP_DECLARATION` must be invoked in the translation unit (with the same name) before calling `MAP_DEFINITION`. If `MAP_DECLARATION` is invoked for a name, there shall be an invocation of `MAP_DEFINITION` with the same name in some translation unit.

This macro shall only be used at the global scope.

# The Map_##Name type and structure
A typedef and structure which contains information about a map. This is where the `Extra` argument from `MAP_DECLARATION` is evaluated, it will see the `Map_##Name` type and structure as incomplete types, it will see the `MapEntry_##Name` type and structure as complete structure types, and it will see `MapKey_##Name` as a complete object type other than an array type. All objects declared in the `Extra` argument are placed inside of this structure before all of the predefined members:<br>
`MapEntryCount` (`size_t`): The number of entries contained in the map.<br>
`MapBucketsSize` (`size_t`): The number of buckets (a power of two greater than or equal to 8, or zero iff no entries are contained).<br>
`MapBuckets` (`MapEntry_##Name**`): The pointer to an array of buckets (a pointer to the first entry, or null iff no entries are contained).

If all of the predefined structure fields are defined to be zero/null, the map will be an empty map.

# The MapEntry_##Name type and structure
A typedef and structure which contains information about an entry. This is where the `Value` argument from `MAP_DECLARATION` is evaluated, it will see the `Map_##Name` and `MapEntry_##Name` types and structures as incomplete types, and it will see `MapKey_##Name` as a complete object type other than an array type. All objects declared in the `Value` argument are placed inside of this structure after all of the predefined members:<br>
`MapNext` (`MapEntry_##Name*`): The pointer to the next entry which is placed at the same index, or null.<br>
`MapHash` (`size_t`): The hash of the key stored.<br>
`MapKey` (`MapKey_##Name`): The key stored in the entry.

# The MapKey_##Name type
A typedef for the type of the keys. This is where the `Key` argument from `MAP_DECLARATION` is evaluated, it will see the `Map_##Name` and `MapEntry_##Name` types and structures as incomplete types.

# The MapFind_##Name function

## Synopsis
```c
MapEntry_##Name**MapFind_##Name(Map_##Name*,MapKey_##Name);
```

## Description
Finds an entry with the key provided, if there are multiple entries with the same key then any of them can be chosen. The result will never change if no entries are inserted or removed. Returns null if no entry was found.

# The MapFindNext_##Name function

## Synopsis
```c
MapEntry_##Name**MapFindNext_##Name(Map_##Name*,MapEntry_##Name*);
```

## Description
Find the next entry with a key equal to the key of the entry provided, if there are multiple entries with the same key then any of them can be chosen as long as they don't come before the entry provided. The result will never change if no entries are inserted or removed. The entry provided must be from the map provided. Returns null if no entry was found.

# The MapAdd_##Name function

## Synopsis
```c
MapEntry_##Name**MapAdd_##Name(Map_##Name*,MapKey_##Name);
```

## Description
Inserts an entry into the map provided with the key provided and returns the pointer to the pointer to the entry, or returns null if the entry could not be inserted.

# The MapLocate_##Name function

## Synopsis
```c
MapEntry_##Name**MapLocate_##Name(Map_##Name*,MapEntry_##Name*);
```

## Description
Finds the pointer to a pointer to an entry from a pointer to an entry. The entry provided must be from the map provided.

# The MapRemove_##Name function

## Synopsis
```c
void MapRemove_##Name(Map_##Name*,MapEntry_##Name**);
```

## Description
Removes the entry from the map. The entry provided must be from the map provided.

# The MapClear_##Name function

## Synopsis
```c
void MapClear_##Name(Map_##Name*);
```

## Desciption
Clears all entries in the map, which will release all resources associated with the Map. An empty map is always one without any resources to free (even without calling `MapClear_##Name`).

# Pointer Lifetime
A pointer to an entry will exist for as long as the entry exists.<br>
A pointer to a pointer to an entry will exist until an entry is added to or removed from the map which the entry is a part of.

# Reserved Identifiers

This header reserves all identifiers beginning with `MapRESERVED__` in all contexts.

All function, type, and structure names declared by calls to MAP_DECLARATION shan't be used as macro names or declared globally.

The names of predefined structure members shan't be defined as macros.

The names `MAP_DECLARATION` and `MAP_DEFINITION` are reserved macro names for this header.

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
// keys are pointers to null terminated strings
// each entry gets an associated Value (of type int)
// and no extra data needs to be stored in the map itself
MAP_DECLARATION(I, const char *, int Value;, )
// the map parameter doesn't need to be evaluated
#define Hash(String, Map) HashString(String)
#define IsEqual(String1, String2, Map) (!strcmp(String1, String2))
#define Allocate(Size, Map) malloc(Size)
#define Free(Pointer, Map) free(Pointer)
MAP_DEFINITION(I, Hash, IsEqual, Allocate, Free)
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
