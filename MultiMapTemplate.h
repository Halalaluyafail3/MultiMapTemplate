#ifndef MapRESERVED__MULTIMAPTEMPLATE_H
#define MapRESERVED__MULTIMAPTEMPLATE_H
#include <stddef.h>
#include <stdint.h>
#define MAP_DECLARATION(Name, Qualifiers, Key, Value, Extra)                   \
  typedef struct MapEntry_##Name MapEntry_##Name;                              \
  typedef struct Map_##Name Map_##Name;                                        \
  typedef typeof(Key) MapKey_##Name;                                           \
  struct MapEntry_##Name {                                                     \
    MapEntry_##Name *MapNext;                                                  \
    size_t MapHash;                                                            \
    MapKey_##Name MapKey;                                                      \
    Value                                                                      \
  };                                                                           \
  struct Map_##Name {                                                          \
    Extra size_t MapEntryCount;                                                \
    size_t MapBucketsSize;                                                     \
    MapEntry_##Name **MapBuckets;                                              \
  };                                                                           \
  Qualifiers void MapRESERVED__MapRehash_##Name(Map_##Name *, size_t);         \
  Qualifiers MapEntry_##Name **MapFind_##Name(Map_##Name *, MapKey_##Name);    \
  Qualifiers MapEntry_##Name **MapFindNext_##Name(Map_##Name *,                \
                                                  MapEntry_##Name *);          \
  Qualifiers MapEntry_##Name **MapAdd_##Name(Map_##Name *, MapKey_##Name);     \
  Qualifiers MapEntry_##Name **MapLocate_##Name(Map_##Name *,                  \
                                                MapEntry_##Name *);            \
  Qualifiers void MapRemove_##Name(Map_##Name *, MapEntry_##Name **);          \
  Qualifiers void MapClear_##Name(Map_##Name *);
#define MAP_DEFINITION(Name, Qualifiers, Hash, IsEqual, Allocate, Free)        \
  Qualifiers void MapRESERVED__MapRehash_##Name(                               \
      Map_##Name *MapRESERVED__Map, size_t MapRESERVED__NewBucketsSize) {      \
    MapEntry_##Name **MapRESERVED__NewBuckets = Allocate(                      \
        MapRESERVED__NewBucketsSize * sizeof(*MapRESERVED__NewBuckets),        \
        MapRESERVED__Map);                                                     \
    if (!MapRESERVED__NewBuckets) {                                            \
      return;                                                                  \
    }                                                                          \
    for (size_t MapRESERVED__Index = 0;                                        \
         MapRESERVED__Index != MapRESERVED__NewBucketsSize;                    \
         ++MapRESERVED__Index) {                                               \
      MapRESERVED__NewBuckets[MapRESERVED__Index] = 0;                         \
    } /* insert all of the old entries into the new table (doesn't preserve    \
         any order) */                                                         \
    for (size_t MapRESERVED__Index = 0;                                        \
         MapRESERVED__Index != MapRESERVED__Map->MapBucketsSize;               \
         ++MapRESERVED__Index) {                                               \
      MapEntry_##Name *MapRESERVED__Entry =                                    \
          MapRESERVED__Map->MapBuckets[MapRESERVED__Index];                    \
      while (MapRESERVED__Entry) {                                             \
        MapEntry_##Name *MapRESERVED__Next = MapRESERVED__Entry->MapNext;      \
        size_t MapRESERVED__NewIndex =                                         \
            MapRESERVED__Entry->MapHash & MapRESERVED__NewBucketsSize - 1;     \
        MapRESERVED__Entry->MapNext =                                          \
            MapRESERVED__NewBuckets[MapRESERVED__NewIndex];                    \
        MapRESERVED__NewBuckets[MapRESERVED__NewIndex] = MapRESERVED__Entry;   \
        MapRESERVED__Entry = MapRESERVED__Next;                                \
      }                                                                        \
    }                                                                          \
    Free(MapRESERVED__Map->MapBuckets, MapRESERVED__Map);                      \
    MapRESERVED__Map->MapBuckets = MapRESERVED__NewBuckets;                    \
    MapRESERVED__Map->MapBucketsSize = MapRESERVED__NewBucketsSize;            \
  }                                                                            \
  Qualifiers MapEntry_##Name **MapFind_##Name(                                 \
      Map_##Name *MapRESERVED__Map, MapKey_##Name MapRESERVED__Key) {          \
    if (!MapRESERVED__Map->MapBucketsSize) {                                   \
      return 0;                                                                \
    }                                                                          \
    size_t MapRESERVED__Hash = Hash(MapRESERVED__Key, MapRESERVED__Map);       \
    size_t MapRESERVED__Index =                                                \
        MapRESERVED__Hash & MapRESERVED__Map->MapBucketsSize - 1;              \
    MapEntry_##Name *MapRESERVED__Entry =                                      \
        MapRESERVED__Map->MapBuckets[MapRESERVED__Index];                      \
    if (!MapRESERVED__Entry) {                                                 \
      return 0;                                                                \
    } /* find the first entry in the bucket that is equal to the key, compare  \
         hashes first in case IsEqual is expensive */                          \
    if (MapRESERVED__Hash == MapRESERVED__Entry->MapHash &&                    \
        IsEqual(MapRESERVED__Key, MapRESERVED__Entry->MapKey,                  \
                MapRESERVED__Map)) {                                           \
      return MapRESERVED__Map->MapBuckets + MapRESERVED__Index;                \
    }                                                                          \
    for (;;) {                                                                 \
      MapEntry_##Name *MapRESERVED__Next = MapRESERVED__Entry->MapNext;        \
      if (!MapRESERVED__Next) {                                                \
        return 0;                                                              \
      }                                                                        \
      if (MapRESERVED__Hash == MapRESERVED__Entry->MapHash &&                  \
          IsEqual(MapRESERVED__Key, MapRESERVED__Next->MapKey,                 \
                  MapRESERVED__Map)) {                                         \
        return &MapRESERVED__Entry->MapNext;                                   \
      }                                                                        \
      MapRESERVED__Entry = MapRESERVED__Next;                                  \
    }                                                                          \
  }                                                                            \
  Qualifiers MapEntry_##Name **MapFindNext_##Name(                             \
      Map_##Name *MapRESERVED__Map, MapEntry_##Name *MapRESERVED__Entry) {     \
    size_t MapRESERVED__Hash = MapRESERVED__Entry->MapHash;                    \
    for (MapKey_##Name MapRESERVED__Key = MapRESERVED__Entry->MapKey;;) {      \
      MapEntry_##Name *MapRESERVED__Next = MapRESERVED__Entry->MapNext;        \
      if (!MapRESERVED__Next) {                                                \
        return 0;                                                              \
      }                                                                        \
      if (MapRESERVED__Hash == MapRESERVED__Next->MapHash &&                   \
          IsEqual(MapRESERVED__Key, MapRESERVED__Next->MapKey,                 \
                  MapRESERVED__Map)) {                                         \
        return &MapRESERVED__Entry->MapNext;                                   \
      }                                                                        \
      MapRESERVED__Entry = MapRESERVED__Next;                                  \
    }                                                                          \
  }                                                                            \
  Qualifiers MapEntry_##Name **MapAdd_##Name(Map_##Name *MapRESERVED__Map,     \
                                             MapKey_##Name MapRESERVED__Key) { \
    if (MapRESERVED__Map->MapEntryCount == SIZE_MAX) {                         \
      return 0;                                                                \
    }                                                                          \
    MapEntry_##Name *MapRESERVED__Entry =                                      \
        Allocate(sizeof(*MapRESERVED__Entry), MapRESERVED__Map);               \
    if (!MapRESERVED__Entry) {                                                 \
      return 0;                                                                \
    }                                                                          \
    MapRESERVED__Entry->MapKey = MapRESERVED__Key;                             \
    if (!MapRESERVED__Map                                                      \
             ->MapBucketsSize) { /* empty map, allocate space for 8 buckets    \
                                    and insert the new element */              \
      if (sizeof(MapEntry_##Name *) > SIZE_MAX >> 3 ||                         \
          !(MapRESERVED__Map->MapBuckets =                                     \
                Allocate(sizeof(MapEntry_##Name *) << 3, MapRESERVED__Map))) { \
        Free(MapRESERVED__Entry, MapRESERVED__Map);                            \
        return 0;                                                              \
      }                                                                        \
      size_t MapRESERVED__Index = (MapRESERVED__Entry->MapHash = Hash(         \
                                       MapRESERVED__Key, MapRESERVED__Map)) &  \
                                  7;                                           \
      MapRESERVED__Map->MapBucketsSize = 8;                                    \
      MapRESERVED__Map->MapEntryCount = 1;                                     \
      MapRESERVED__Entry->MapNext = 0;                                         \
      MapRESERVED__Map->MapBuckets[MapRESERVED__Index] = MapRESERVED__Entry;   \
      MapRESERVED__Map->MapBuckets[MapRESERVED__Index + 1 & 7] =               \
          MapRESERVED__Map->MapBuckets[MapRESERVED__Index + 2 & 7] =           \
              MapRESERVED__Map->MapBuckets[MapRESERVED__Index + 3 & 7] =       \
                  MapRESERVED__Map->MapBuckets[MapRESERVED__Index + 4 & 7] =   \
                      MapRESERVED__Map                                         \
                          ->MapBuckets[MapRESERVED__Index + 5 & 7] =           \
                          MapRESERVED__Map                                     \
                              ->MapBuckets[MapRESERVED__Index + 6 & 7] =       \
                              MapRESERVED__Map                                 \
                                  ->MapBuckets[MapRESERVED__Index + 7 & 7] =   \
                                  0;                                           \
      return MapRESERVED__Map->MapBuckets + MapRESERVED__Index;                \
    } /* when the ratio of the number of entries to buckets is more than 3:4,  \
         try to reallocate to add more buckets */                              \
    if (++MapRESERVED__Map->MapEntryCount >                                    \
            (MapRESERVED__Map->MapBucketsSize >> 1) +                          \
                (MapRESERVED__Map->MapBucketsSize >> 2) &&                     \
        MapRESERVED__Map->MapBucketsSize <=                                    \
            SIZE_MAX / sizeof(MapEntry_##Name *) >> 1) {                       \
      MapRESERVED__MapRehash_##Name(MapRESERVED__Map,                          \
                                    MapRESERVED__Map->MapBucketsSize << 1);    \
    } /* add the new token into the bucket */                                  \
    size_t MapRESERVED__Index = (MapRESERVED__Entry->MapHash = Hash(           \
                                     MapRESERVED__Key, MapRESERVED__Map)) &    \
                                MapRESERVED__Map->MapBucketsSize - 1;          \
    MapRESERVED__Entry->MapNext =                                              \
        MapRESERVED__Map->MapBuckets[MapRESERVED__Index];                      \
    MapRESERVED__Map->MapBuckets[MapRESERVED__Index] = MapRESERVED__Entry;     \
    return MapRESERVED__Map->MapBuckets + MapRESERVED__Index;                  \
  }                                                                            \
  Qualifiers MapEntry_##Name **MapLocate_##Name(                               \
      Map_##Name *MapRESERVED__Map, MapEntry_##Name *MapRESERVED__Entry) {     \
    size_t MapRESERVED__Index =                                                \
        MapRESERVED__Entry->MapHash & MapRESERVED__Map->MapBucketsSize - 1;    \
    MapEntry_##Name *MapRESERVED__Previous =                                   \
        MapRESERVED__Map->MapBuckets[MapRESERVED__Index];                      \
    /* search into the bucket to find what points at the entry */              \
    if (MapRESERVED__Previous == MapRESERVED__Entry) {                         \
      return MapRESERVED__Map->MapBuckets + MapRESERVED__Index;                \
    }                                                                          \
    for (;;) {                                                                 \
      MapEntry_##Name *MapRESERVED__Next = MapRESERVED__Previous->MapNext;     \
      if (MapRESERVED__Next == MapRESERVED__Entry) {                           \
        return &MapRESERVED__Previous->MapNext;                                \
      }                                                                        \
      MapRESERVED__Previous = MapRESERVED__Next;                               \
    }                                                                          \
  }                                                                            \
  Qualifiers void MapRemove_##Name(Map_##Name *MapRESERVED__Map,               \
                                   MapEntry_##Name **MapRESERVED__Entry) {     \
    MapEntry_##Name *MapRESERVED__Next = (*MapRESERVED__Entry)->MapNext;       \
    Free(*MapRESERVED__Entry, MapRESERVED__Map);                               \
    *MapRESERVED__Entry = MapRESERVED__Next;                                   \
    if (!--MapRESERVED__Map->MapEntryCount) { /* an empty map must have no     \
                                                 memory left to free */        \
      Free(MapRESERVED__Map->MapBuckets, MapRESERVED__Map);                    \
      MapRESERVED__Map->MapBuckets = 0;                                        \
      MapRESERVED__Map->MapBucketsSize = 0;                                    \
    } else if (MapRESERVED__Map->MapEntryCount <                               \
               MapRESERVED__Map->MapBucketsSize >>                             \
               3) { /* when the ratio of the number of entries to buckets is   \
                       less than 1:8, try to shrink to save space */           \
      MapRESERVED__MapRehash_##Name(MapRESERVED__Map,                          \
                                    MapRESERVED__Map->MapBucketsSize >> 1);    \
    }                                                                          \
  }                                                                            \
  Qualifiers void MapClear_##Name(Map_##Name *MapRESERVED__Map) {              \
    if (!MapRESERVED__Map->MapBucketsSize) {                                   \
      return;                                                                  \
    }                                                                          \
    for (size_t MapRESERVED__Index = 0;;) {                                    \
      MapEntry_##Name *MapRESERVED__Entry =                                    \
          MapRESERVED__Map->MapBuckets[MapRESERVED__Index];                    \
      while (MapRESERVED__Entry) {                                             \
        MapEntry_##Name *MapRESERVED__Next = MapRESERVED__Entry->MapNext;      \
        Free(MapRESERVED__Entry, MapRESERVED__Map);                            \
        MapRESERVED__Entry = MapRESERVED__Next;                                \
      }                                                                        \
      if (++MapRESERVED__Index == MapRESERVED__Map->MapBucketsSize) {          \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    Free(MapRESERVED__Map->MapBuckets, MapRESERVED__Map);                      \
    MapRESERVED__Map->MapBuckets = 0;                                          \
    MapRESERVED__Map->MapBucketsSize = MapRESERVED__Map->MapEntryCount = 0;    \
  }
#define MAP_FUNCTION_DECLARATIONS(Name, Qualifiers)                            \
  Qualifiers void MapRESERVED__MapRehash_##Name(Map_##Name *, size_t);         \
  Qualifiers MapEntry_##Name **MapFind_##Name(Map_##Name *, MapKey_##Name);    \
  Qualifiers MapEntry_##Name **MapFindNext_##Name(Map_##Name *,                \
                                                  MapEntry_##Name *);          \
  Qualifiers MapEntry_##Name **MapAdd_##Name(Map_##Name *, MapKey_##Name);     \
  Qualifiers MapEntry_##Name **MapLocate_##Name(Map_##Name *,                  \
                                                MapEntry_##Name *);            \
  Qualifiers void MapRemove_##Name(Map_##Name *, MapEntry_##Name **);          \
  Qualifiers void MapClear_##Name(Map_##Name *);
#endif
