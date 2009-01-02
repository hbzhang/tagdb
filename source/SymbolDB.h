/*#io
docCopyright("Steve Dekorte", 2007)
docLicense("BSD revised")
*/

#ifndef SymbolDB_DEFINED
#define SymbolDB_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

#include "TagDBAPI.h"
#include "Datum.h"
#include "Uint64Array.h"

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
	char *s2iPath;
	char *i2sPath;
	TCBDB *i2s;
	BDBCUR *i2sCursor;
	TCBDB *s2i;
} SymbolDB;

TAGDB_API SymbolDB *SymbolDB_new(void);
TAGDB_API void SymbolDB_free(SymbolDB *self);

TAGDB_API void SymbolDB_setPath_(SymbolDB *self, char *path);
TAGDB_API char *SymbolDB_path(SymbolDB *self);
TAGDB_API void SymbolDB_delete(SymbolDB *self);

TAGDB_API int SymbolDB_open(SymbolDB *self);
TAGDB_API int SymbolDB_close(SymbolDB *self);
TAGDB_API void SymbolDB_sync(SymbolDB *self);
TAGDB_API void SymbolDB_show(SymbolDB *self);
TAGDB_API Uint64Array *SymbolDB_allSymbolIds(SymbolDB *self);

void SymbolDB_begin(SymbolDB *self);
void SymbolDB_commit(SymbolDB *self);

typedef uint64_t symbolid_t;

Datum *SymbolDB_symbolForId_(SymbolDB *self, symbolid_t key); // caller should Datum_free result
symbolid_t SymbolDB_idForSymbol_(SymbolDB *self, Datum *symbol);

#ifdef __cplusplus
}
#endif
#endif
