#ifndef MapDecl
#include<stddef.h>
#include<stdint.h>
#define MapDecl(Name,Key,Val,Extra)\
	typedef struct MapEntry_##Name MapEntry_##Name;\
	typedef struct Map_##Name Map_##Name;\
	typedef typeof(Key)MapKey_##Name;\
	struct MapEntry_##Name{\
		MapEntry_##Name*MapNext;\
		size_t MapHash;\
		MapKey_##Name MapKey;\
		Val};\
	struct Map_##Name{\
		Extra\
		size_t MapEntryCnt;\
		size_t MapBucketsSize;\
		MapEntry_##Name**MapBuckets;};\
	MapEntry_##Name**MapFind_##Name(Map_##Name*,MapKey_##Name);\
	MapEntry_##Name**MapFindNext_##Name(Map_##Name*,MapEntry_##Name*);\
	MapEntry_##Name**MapAdd_##Name(Map_##Name*,MapKey_##Name);\
	MapEntry_##Name**MapLocate_##Name(Map_##Name*,MapEntry_##Name*);\
	void MapRemove_##Name(Map_##Name*,MapEntry_##Name**);\
	void MapClear_##Name(Map_##Name*);
#define MapDef(Name,Hash,Cmp,Alloc,Free)\
	static void MapRESERVED__MapRehash_##Name(Map_##Name*MapRESERVED__Map,size_t MapRESERVED__NewBucketsSize){\
		MapEntry_##Name**MapRESERVED__NewBuckets=Alloc(MapRESERVED__NewBucketsSize*sizeof(*MapRESERVED__NewBuckets),MapRESERVED__Map);\
		if(!MapRESERVED__NewBuckets){\
			return;}\
		for(size_t MapRESERVED__Idx=0;MapRESERVED__Idx!=MapRESERVED__NewBucketsSize;++MapRESERVED__Idx){\
			MapRESERVED__NewBuckets[MapRESERVED__Idx]=0;}\
		for(size_t MapRESERVED__Idx=0;MapRESERVED__Idx!=MapRESERVED__Map->MapBucketsSize;++MapRESERVED__Idx){\
			MapEntry_##Name*MapRESERVED__Entry=MapRESERVED__Map->MapBuckets[MapRESERVED__Idx];\
			while(MapRESERVED__Entry){\
				MapEntry_##Name*MapRESERVED__Next=MapRESERVED__Entry->MapNext;\
				size_t MapRESERVED__NewIdx=MapRESERVED__Entry->MapHash&MapRESERVED__NewBucketsSize-1;\
				MapRESERVED__Entry->MapNext=MapRESERVED__NewBuckets[MapRESERVED__NewIdx];\
				MapRESERVED__NewBuckets[MapRESERVED__NewIdx]=MapRESERVED__Entry;\
				MapRESERVED__Entry=MapRESERVED__Next;}}\
		Free(MapRESERVED__Map->MapBuckets,MapRESERVED__Map);\
		MapRESERVED__Map->MapBuckets=MapRESERVED__NewBuckets;\
		MapRESERVED__Map->MapBucketsSize=MapRESERVED__NewBucketsSize;}\
	MapEntry_##Name**MapFind_##Name(Map_##Name*MapRESERVED__Map,MapKey_##Name MapRESERVED__Key){\
		if(!MapRESERVED__Map->MapBucketsSize){\
			return 0;}\
		size_t MapRESERVED__Idx=Hash(MapRESERVED__Key,MapRESERVED__Map)&MapRESERVED__Map->MapBucketsSize-1;\
		MapEntry_##Name*MapRESERVED__Entry=MapRESERVED__Map->MapBuckets[MapRESERVED__Idx];\
		if(!MapRESERVED__Entry){\
			return 0;}\
		if(Cmp(MapRESERVED__Key,MapRESERVED__Entry->MapKey,MapRESERVED__Map)){\
			return MapRESERVED__Map->MapBuckets+MapRESERVED__Idx;}\
		for(;;){\
			MapEntry_##Name*MapRESERVED__Next=MapRESERVED__Entry->MapNext;\
			if(!MapRESERVED__Next){\
				return 0;}\
			if(Cmp(MapRESERVED__Key,MapRESERVED__Next->MapKey,MapRESERVED__Map)){\
				return&MapRESERVED__Entry->MapNext;}\
			MapRESERVED__Entry=MapRESERVED__Next;}}\
	MapEntry_##Name**MapFindNext_##Name(Map_##Name*MapRESERVED__Map,MapEntry_##Name*MapRESERVED__Entry){\
		MapKey_##Name MapRESERVED__Key=MapRESERVED__Entry->MapKey;\
		for(;;){\
			MapEntry_##Name*MapRESERVED__Next=MapRESERVED__Entry->MapNext;\
			if(!MapRESERVED__Next){\
				return 0;}\
			if(Cmp(MapRESERVED__Key,MapRESERVED__Next->MapKey,MapRESERVED__Map)){\
				return&MapRESERVED__Entry->MapNext;}\
			MapRESERVED__Entry=MapRESERVED__Next;}}\
	MapEntry_##Name**MapAdd_##Name(Map_##Name*MapRESERVED__Map,MapKey_##Name MapRESERVED__Key){\
		if(MapRESERVED__Map->MapEntryCnt==SIZE_MAX){\
			return 0;}\
		MapEntry_##Name*MapRESERVED__Entry=Alloc(sizeof(*MapRESERVED__Entry),MapRESERVED__Map);\
		if(!MapRESERVED__Entry){\
			return 0;}\
		MapRESERVED__Entry->MapKey=MapRESERVED__Key;\
		if(!MapRESERVED__Map->MapBucketsSize){\
			if(sizeof(MapEntry_##Name*)>SIZE_MAX>>3||!(MapRESERVED__Map->MapBuckets=Alloc(sizeof(MapEntry_##Name*)<<3),MapRESERVED__Map)){\
				Free(MapRESERVED__Entry,MapRESERVED__Map);\
				return 0;}\
			size_t MapRESERVED__Idx=(MapRESERVED__Entry->MapHash=Hash(MapRESERVED__Key,MapRESERVED__Map))&7;\
			MapRESERVED__Map->MapBucketsSize=8;\
			MapRESERVED__Map->MapEntryCnt=1;\
			MapRESERVED__Entry->MapNext=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx]=MapRESERVED__Entry;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+1&7]=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+2&7]=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+3&7]=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+4&7]=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+5&7]=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+6&7]=0;\
			MapRESERVED__Map->MapBuckets[MapRESERVED__Idx+7&7]=0;\
			return MapRESERVED__Map->MapBuckets+MapRESERVED__Idx;}\
		if(++MapRESERVED__Map->MapEntryCnt>(MapRESERVED__Map->MapBucketsSize>>1)+(MapRESERVED__Map->MapBucketsSize>>2)&&MapRESERVED__Map->MapBucketsSize<=SIZE_MAX/sizeof(MapEntry_##Name*)>>1){\
			MapRESERVED__MapRehash_##Name(MapRESERVED__Map,MapRESERVED__Map->MapBucketsSize<<1);}\
		size_t MapRESERVED__Idx=(MapRESERVED__Entry->MapHash=Hash(MapRESERVED__Key,MapRESERVED__Map))&MapRESERVED__Map->MapBucketsSize-1;\
		MapRESERVED__Entry->MapNext=MapRESERVED__Map->MapBuckets[MapRESERVED__Idx];\
		MapRESERVED__Map->MapBuckets[MapRESERVED__Idx]=MapRESERVED__Entry;\
		return MapRESERVED__Map->MapBuckets+MapRESERVED__Idx;}\
	MapEntry_##Name**MapLocate_##Name(Map_##Name*MapRESERVED__Map,MapEntry_##Name*MapRESERVED__Entry){\
		size_t MapRESERVED__Idx=MapRESERVED__Entry->MapHash&MapRESERVED__Map->MapBucketsSize-1;\
		MapEntry_##Name*MapRESERVED__Prev=MapRESERVED__Map->MapBuckets[MapRESERVED__Idx];\
		if(MapRESERVED__Prev==MapRESERVED__Entry){\
			return MapRESERVED__Map->MapBuckets+MapRESERVED__Idx;}\
		for(;;){\
			MapEntry_##Name*MapRESERVED__Next=MapRESERVED__Prev->MapNext;\
			if(MapRESERVED__Next==MapRESERVED__Entry){\
				return&MapRESERVED__Prev->MapNext;}\
			MapRESERVED__Prev=MapRESERVED__Next;}}\
	void MapRemove_##Name(Map_##Name*MapRESERVED__Map,MapEntry_##Name**MapRESERVED__Entry){\
		MapEntry_##Name*MapRESERVED__Next=(*MapRESERVED__Entry)->MapNext;\
		Free(*MapRESERVED__Entry,MapRESERVED__Map);\
		*MapRESERVED__Entry=MapRESERVED__Next;\
		if(!--MapRESERVED__Map->MapEntryCnt){\
			Free(MapRESERVED__Map->MapBuckets,MapRESERVED__Map);\
			MapRESERVED__Map->MapBuckets=0;\
			MapRESERVED__Map->MapBucketsSize=0;}\
		else if(MapRESERVED__Map->MapEntryCnt<MapRESERVED__Map->MapBucketsSize>>3){\
			MapRESERVED__MapRehash_##Name(MapRESERVED__Map,MapRESERVED__Map->MapBucketsSize>>1);}}\
	void MapClear_##Name(Map_##Name*MapRESERVED__Map){\
		if(!MapRESERVED__Map->MapBucketsSize){\
			return;}\
		for(size_t MapRESERVED__Idx=0;;){\
			MapEntry_##Name*MapRESERVED__Entry=MapRESERVED__Map->MapBuckets[MapRESERVED__Idx];\
			while(MapRESERVED__Entry){\
				MapEntry_##Name*MapRESERVED__Next=MapRESERVED__Entry->MapNext;\
				Free(MapRESERVED__Entry,MapRESERVED__Map);\
				MapRESERVED__Entry=MapRESERVED__Next;}\
			if(++MapRESERVED__Idx==MapRESERVED__Map->MapBucketsSize){\
				break;}}\
		Free(MapRESERVED__Map->MapBuckets,MapRESERVED__Map);\
		MapRESERVED__Map->MapBuckets=0;\
		MapRESERVED__Map->MapBucketsSize=0;\
		MapRESERVED__Map->MapEntryCnt=0;}
#endif
