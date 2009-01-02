
#ifndef Uint64Array_DEFINED
#define Uint64Array_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

#include "TagDBAPI.h"
#include "portable_stdint.h"
#include <stdio.h>

typedef struct
{
  uint64_t *ids;
  int size;
} Uint64Array;

TAGDB_API Uint64Array *Uint64Array_new(void);
TAGDB_API void Uint64Array_removeAll(Uint64Array *self);
TAGDB_API Uint64Array *Uint64Array_clone(Uint64Array *self);
TAGDB_API void Uint64Array_copy_(Uint64Array *self, Uint64Array *other);
TAGDB_API int Uint64Array_equals_(Uint64Array *self, Uint64Array *other);
TAGDB_API void Uint64Array_free(Uint64Array *self);
TAGDB_API void Uint64Array_append_(Uint64Array *self, uint64_t v);
TAGDB_API void Uint64Array_appendIfAbsent_(Uint64Array *self, uint64_t v);
TAGDB_API void Uint64Array_sort(Uint64Array *self);
TAGDB_API int Uint64Array_remove_(Uint64Array *self, uint64_t v);
TAGDB_API void Uint64Array_removeDuplicates(Uint64Array *self);
TAGDB_API size_t Uint64Array_size(Uint64Array *self);
TAGDB_API uint64_t Uint64Array_at_(Uint64Array *self, size_t index);
TAGDB_API int Uint64Array_contains_(Uint64Array *self, uint64_t v);
TAGDB_API void Uint64Array_show(Uint64Array *self);
TAGDB_API uint64_t Uint64Array_filter(Uint64Array *self);
TAGDB_API void *Uint64Array_data(Uint64Array *self);

#ifdef __cplusplus
}
#endif
#endif
