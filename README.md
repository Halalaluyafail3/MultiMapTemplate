# MultiMapTemplate
A header only multimap library using macros in C.

The header defines two macros and includes the headers `stdint.h` and `stddef.h`.

This library assumes `typeof` exists, specifically it relies on `typeof(type)`. If `typeof` is not provided by your compiler, it should be defined as a macro which calls your compiler's version of `typeof`. If `typeof` is defined as `#define typeof(...)__VA_ARGS__` (because `typeof` is not supported) or something equivalent, then the type provided as the type of the keys shall be specified such that the type of a pointer to a key can be obtained by postfixing a `*` to the type of the keys.

# The MapDecl macro
Declares a map using the given name and types. In each translation unit MapDecl shall only be called once for each name provided, duplicate calls are never allowed.

## Synopsis
```c
#define MapDecl(Name,Key,Val,Extra) /* ... */
```

## Description
`Name` is used to identify different maps, all globally declared entities will use it as a suffix.
`Key` is the type of the keys, which shall be a complete object type other than an array type.
`Val` and `Extra` are both series of declarations which shall be valid inside of a `struct`, but no flexible array members may be specified.

# The MapDef macro
Defines a map using the given name and functions. MapDef shall only be called once for each name provided, duplicate calls are never allowed, even in seperate translation units.

## Synopsis
```c
#define MapDef(Name,Hash,Cmp,Alloc,Free) /* ... */
```

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