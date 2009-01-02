/*#io
docCopyright("Steve Dekorte", 2007)
docLicense("BSD revised")
docDescription("A fast in-memory tagging database using QDBM as a backing store.
All key/tags records are read on start up and earches are done on the in-memory table.
Insert/updates are transactionally saved to QDBM as they are made.")
*/

#ifndef TagDB_DEFINED
#define TagDB_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

#include "TagDBAPI.h"
#include "TagDBItem.h"
#include "SymbolDB.h"
#include "Datum.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <tcutil.h>
#include <tcbdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	char *path;
	char *k2tPath;
	SymbolDB *symbols;
	TCBDB *keyToTags;
	BDBCUR *cursor;
	TagDBItem **items;
	size_t itemCount;
	Uint64Array *results;
	
	//Uint64Array *filters;
	//size_t filterArea;
	//int needsToBuildFilters;
} TagDB;

TAGDB_API TagDB *TagDB_new(void);
TAGDB_API void TagDB_free(TagDB *self);
TAGDB_API void TagDB_delete(TagDB *self);

TAGDB_API void TagDB_setPath_(TagDB *self, char *path);
TAGDB_API char *TagDB_path(TagDB *self);

TAGDB_API int TagDB_open(TagDB *self);
TAGDB_API int TagDB_close(TagDB *self);
TAGDB_API void TagDB_begin(TagDB *self);
TAGDB_API void TagDB_commit(TagDB *self);

// low level API

TAGDB_API int TagDB_atKey_putTags_(TagDB *self, uint64_t key, Uint64Array *tags);
TAGDB_API Uint64Array *TagDB_tagsAtKey_(TagDB *self, uint64_t key); // don't free result
TAGDB_API Datum *TagDB_keyAtIndex_(TagDB *self, size_t index);
TAGDB_API int TagDB_removeKey_(TagDB *self, uint64_t key);
TAGDB_API size_t TagDB_removeTag_(TagDB *self, uint64_t tag);
TAGDB_API Uint64Array *TagDB_keysForTags_exclusive_(TagDB *self, Uint64Array *tags, int exclusive);
TAGDB_API Uint64Array *TagDB_keysForTags_(TagDB *self, Uint64Array *tags);
TAGDB_API Uint64Array *TagDB_untaggedKeys(TagDB *self);
TAGDB_API size_t TagDB_size(TagDB *self);

TAGDB_API void TagDB_show(TagDB *self);

TAGDB_API Datum *TagDB_symbolForId_(TagDB *self, symbolid_t key); // caller should free
TAGDB_API symbolid_t TagDB_idForSymbol_size_(TagDB *self, const char *s, size_t size);
TAGDB_API symbolid_t TagDB_idForSymbol_(TagDB *self, const char *s);
//TAGDB_API void TagDB_buildFilters(TagDB *self);
TAGDB_API Uint64Array *TagDB_allUniqueTags(TagDB *self); // caller should free result array

#ifdef __cplusplus
}
#endif
#endif
