# MultiMapTemplate
A header only multimap library using macros in C.

The header defines the two macros `MapDecl` and `MapDef` and includes the headers `stdint.h` and `stddef.h`.

This library assumes `typeof` exists, specifically it relies on `typeof(type)`. If `typeof` is not provided by your compiler, it should be defined as a macro which calls your compiler's version of `typeof`. If `typeof` is defined as `#define typeof(...)__VA_ARGS__` (because `typeof` is not supported) or something equivalent, then the type provided as the type of the keys shall be specified such that the type of a pointer to a key can be obtained by postfixing a `*` to the type of the keys.

# The MapDecl macro
Declares a map using the given name and types.

## Synopsis
```c
#define MapDecl(Name,Key,Val,Extra) /* ... */
```

## Description
`Name` is used to identify different maps, all globally declared entities will use it as a suffix.<br>
`Key` is the type of the keys, which shall be a complete object type other than an array type.<br>
`Val` and `Extra` are both series of declarations which shall be valid inside of a `struct`, but no flexible array members may be specified.

This macro declares all types, structures, and functions later described.

## Constraints
The objects declared by `Val` and `Extra` shall not have names which conflict with the predefined names of the structs that they are in.

In each translation unit MapDecl shall only be called once per name, duplicate calls are never allowed. MapDecl invocations which use the same name shall all be equivalent.

This macro shall only be used at the global scope.

# The MapDef macro
Defines a map using the given name and functions.

## Synopsis
```c
#define MapDef(Name,Hash,Cmp,Alloc,Free) /* ... */
```

## Description
`Name` is used to identify different maps, all globally defined entities will use it as a suffix.<br>
`Hash`, `Cmp`, `Alloc`, and `Free` shall all be macro names or function names which take the following arguments:<br>
`Hash`: A key, and a pointer to a map<br>
`Cmp`: Two keys, and a pointer to a map<br>
`Alloc`: A `size_t`, and a pointer to a map<br>
`Free`: A pointer implicitly convertible to `void*`, and a pointer to a map<br>
They shall also return the following types:<br>
`Hash`: A `size_t`<br>
`Cmp`: A value which is valid for an `if` condition<br>
`Alloc`: A `void*`<br>
`Free`: Any type, even incomplete types such as `void`

This macro defines all functions declared by `MapDecl`.

## Constraints
When `Hash`, `Cmp`, `Alloc`, or `Free` are provided as macros they shall be fully protected by parentheses, and evaluate each of their arguments exactly once except for the pointer to map argument which may be evaluated any number of times.

There are some restrictions on the functions or macros provided:<br>
`Hash`: All keys passed to `MapAdd_##Name` and `MapFind_##Name` shall be valid to pass to `Hash`, and the return value shall never change<br>
`Cmp`: All keys passed to `MapAdd_#Name` and `MapFind_##Name` shall be valid to pass as one of the arguments, the return value shall never change for equivalent calls, and it should define a normal equality comparison (the reflexive, symmetric, and transitive properties shall all be upheld)
`Alloc`: If the pointer returned, it shall be valid until `Free` is called for the pointer.

For keys which are never passed to `MapAdd_##Name` or `MapFind_##Name` whether they make sense in `Hash` or `Cmp` need not be considered, e.g. with float keys if NaN is never inserted then `Cmp` need not make sure that NaN compares equal to NaN.

MapDef shall only be called once per name, duplicate calls are never allowed, even in seperate translation units. MapDecl must be called in the translation unit before calling MapDef. If MapDecl is used for a name, there shall be an invocation of MapDef with the same name somewhere.

This macro shall only be used at the global scope.

# The Map_##Name type and structure
A typedef and structure which contains information about a map. This is where the `Extra` argument from `MapDecl` is evaluated, it will see the `Map_##Name` type and structure as incomplete types, it will see the `MapEntry_##Name` type and structure as complete structure types, and it will see `MapKey_##Name` as a complete object type other than an array type. All objects declared in the `Extra` argument are placed inside of this structure before all of the predefined members:<br>
`EntryCnt` (`size_t`): The number of entries contained in the map<br>
`MapBucketsSize` (`size_t`): The number of buckets<br>
`Buckets` (`MapEntry_##Name**`): The pointer to an array of buckets (a pointer to the first entry at that index, or null)

# The MapEntry_##Name type and structure
A typedef and structure which contains information about an entry. This is where the `Val` argument from `MapDecl` is evaluated, it will see the `Map_##Name` and `MapEntry_##Name` types and structures as incomplete types, and it will see `MapKey_##Name` as a complete object type other than an array type. All objects declared in the `Val` argument are placed inside of this structure after all of the predefined members:<br>
`MapNext` (`MapEntry_##Name*`): The pointer to the next entry which is placed at the same index, or null<br>
`MapHash` (`size_t`): The hash of the key stored<br>
`MapKey` (`MapKey_##Name`): The key stored in the entry

# The MapKey_##Name type
A typedef for the type of the keys. This is where the `Key` argument from `MapDecl` is evaluated, it will see the `Map_##Name` and `MapEntry_##Name` types and structures as incomplete types.

# The MapFind_##Name function

## Synopsis
```c
MapEntry_##Name**MapFind_##Name(Map_##Name*,MapKey_##Name);
```

## Description
Finds an entry with the key provided, if there are multiple entries with the same key then any of them can be chosen. The result will never change if no entries are inserted or removed.

# The MapFindNext_##Name function

## Synopsis
```c
MapEntry_##Name**MapFindNext_##Name(Map_##Name*,MapEntry_##Name*);
```

## Description
Find the next entry with a key equal to the key of the entry provided, if there are multiple entries with the same key then any of them can be chosen as long as they don't come before the entry provided. The result will never change if no entries are inserted or removed. The entry provided must be from the map provided.

# The MapAdd_##Name function

## Synopsis
```c
MapEntry_##Name**MapAdd_##Name(Map_##Name*,MapKey_##Name);
```

## Description
Inserts an entry into the map provided with the key provided, and returns the pointer to the pointer to the entry. If the entry could not be inserted then the function will return the null pointer.

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
Clears all entries in the map, which will release all resources associated with the Map. An empty map is always one without any resources to free.

# Pointer Lifetime
A pointer to an entry will exist for as long as the entry exists.<br>
A pointer to a pointer to an entry will exist until an entry is added or removed.

# Reserved Identifiers

This header reserves all identifiers beginning with `MapRESERVED__` in all contexts.

All function, type, and structure names declared by calls to MapDecl shall not be used as macro names or declared globally.

The names of predefined structure members shall not be defined as macros.

The names `MapDecl` and `MapDef` are reserved macro names for this header.

# Example
```c
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include"MultiMapTemplate.h"
static size_t HashStr(const char*Str){
	size_t Res=strlen(Str);
	for(size_t Idx=0;Str[Idx];++Idx){
		Res^=(Res<<5)+(Res>>2)+Str[Idx];}
	return Res;}
MapDecl(I,const char*,int Val;,)
#define Hash(Str,...)HashStr(Str)
#define Cmp(Str1,Str2,...)(!strcmp(Str1,Str2))
#define Alloc(Size,...)malloc(Size)
#define Free(Ptr,...)free(Ptr)
MapDef(I,Hash,Cmp,Alloc,Free)
#undef Hash // undefine the names afterward since they aren't needed
#undef Cmp
#undef Alloc
#undef Free
static MapEntry_I**Insert(Map_I*Map,MapKey_I Key,int Val){
	MapEntry_I**Entry=MapAdd_I(Map,Key); // add the key
	if(!Entry){ // for this example handling an error isn't important
		exit(EXIT_FAILURE);}
	(*Entry)->Val=Val;
	return Entry;}
int main(void){
	Map_I m={0}; // create the map and initialize it to be empty
	MapEntry_I*e=*Insert(&m,"a",1); // a=1, save pointer to it
	Insert(&m,"b",2); // b=2
	Insert(&m,"c",3); // c=3
	Insert(&m,"d",4); // d=4
	MapRemove_I(&m,MapLocate_I(&m,e)); // since m changed, find e again, then remove it
	MapRemove_I(&m,MapFind_I(&m,"c")); // remove c
	Insert(&m,"b",3); // b=3, multiple values for the same key may be used
	e=*MapFind_I(&m,"b"); // find first b
	printf("%i %i\n",e->Val,(*MapFindNext_I(&m,e))->Val); // prints 2 and 3 in any order
	MapClear_I(&m); // clear m
	return 0;}
```
