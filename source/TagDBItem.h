
#ifndef TagDBItem_DEFINED
#define TagDBItem_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

#include "TagDBAPI.h"
#include "Uint64Array.h"
#include "Uint64Array.h"

typedef struct
{
	uint64_t key;
	Uint64Array tags;
} TagDBItem;

TAGDB_API TagDBItem *TagDBItem_new(uint64_t key, uint64_t *tags, size_t numTags);
TAGDB_API void TagDBItem_free(TagDBItem *self);

TAGDB_API int TagDBItem_compareByKeyWith_(TagDBItem *self, TagDBItem *other);
TAGDB_API void TagDBItem_sortTags(TagDBItem *self);
TAGDB_API void TagDBItem_setTags_(TagDBItem *self, Uint64Array *tags);

TAGDB_API void TagDBItem_show(TagDBItem *self);
TAGDB_API Uint64Array *TagDBItem_tags(TagDBItem *self);

#ifdef __cplusplus
}
#endif
#endif
