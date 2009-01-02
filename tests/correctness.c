#include <stdio.h>
#include <time.h>
#include <string.h>

#include "TagDB.h"

static TagDB *tdb;

void test(char *message, int ok)
{	
	printf("  %s: %s\n", message, ok ? "OK" : "FAILED");
	if (!ok) { exit(-1); }
}

void doSomeInserts(void)
{
	{
		Uint64Array *tags = Uint64Array_new();
		
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "black"));
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "1991"));
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "nsx"));
		
		test("TagDB_atKey_putTags_", TagDB_atKey_putTags_(tdb, TagDB_idForSymbol_(tdb, "joe"), tags));
		
		Uint64Array_free(tags);		
		test("insert1", TagDB_size(tdb) == 1);
	}
		
	{
		Uint64Array *tags = Uint64Array_new();
		
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "1996"));
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "lotus"));
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "esprit"));
		
		test("TagDB_atKey_putTags_", TagDB_atKey_putTags_(tdb, TagDB_idForSymbol_(tdb, "bob"), tags));
		
		Uint64Array_free(tags);		
		test("insert2", TagDB_size(tdb) == 2);
	}
	
	{
		Uint64Array *tags = Uint64Array_new();
		
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "red"));
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "nsx"));
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "1991"));
		
		test("TagDB_atKey_putTags_", TagDB_atKey_putTags_(tdb, TagDB_idForSymbol_(tdb, "alice"), tags));
		
		Uint64Array_free(tags);		
		test("insert3", TagDB_size(tdb) == 3);
	}
}

void verifyInserts(void)
{
	{
		Uint64Array *tags = TagDB_tagsAtKey_(tdb, TagDB_idForSymbol_(tdb, "joe"));
		test("joe tags", (int)tags);
		test("verify key black", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "black")));
		test("verify key black", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "1991")));
		test("verify key black", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "nsx")));
	}
	
	{
		Uint64Array *tags = TagDB_tagsAtKey_(tdb, TagDB_idForSymbol_(tdb, "bob"));
		test("bob tags", (int)tags);
		test("verify key 1996", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "1996")));
		test("verify key lotus", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "lotus")));
		test("verify key esprit", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "esprit")));
	}
	
	{
		Uint64Array *tags = TagDB_tagsAtKey_(tdb, TagDB_idForSymbol_(tdb, "alice"));
		test("alice tags", (int)tags);
		test("verify key 1991", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "1991")));
		test("verify key nsx", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "nsx")));
		test("verify key red", Uint64Array_contains_(tags, TagDB_idForSymbol_(tdb, "red")));
	}
}

int main2(int argc, const char * argv[]) 
{	
	tdb = TagDB_new();

	printf("\ntdb correctness tests:\n");
	
	TagDB_setPath_(tdb, "testdb");
	TagDB_delete(tdb);
	test("open", TagDB_open(tdb));

	doSomeInserts();
	verifyInserts();
	
	// do some searches
	
	{		
		Uint64Array *tags = Uint64Array_new();
		Uint64Array *keys;
		
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "nsx"));
		keys = TagDB_keysForTags_(tdb, tags);
		test("search nsx", Uint64Array_size(keys) == 2);
		
		Uint64Array_append_(tags, TagDB_idForSymbol_(tdb, "black"));
		keys = TagDB_keysForTags_(tdb, tags);
		test("search nsx, black", Uint64Array_size(keys) == 1);
		
		Uint64Array_free(tags);
	}

	test("close", TagDB_close(tdb));
	
	printf("  reopening...\n");
	
	test("open", TagDB_open(tdb));
	
	verifyInserts();	
	
	// remove keys
	
	{
		TagDB_removeKey_(tdb, TagDB_idForSymbol_(tdb, "bob"));
		test("remove bob", TagDB_size(tdb) == 2);
		TagDB_removeKey_(tdb, TagDB_idForSymbol_(tdb, "joe"));
		test("remove joe", TagDB_size(tdb) == 1);		
		TagDB_removeKey_(tdb, TagDB_idForSymbol_(tdb, "alice"));
		test("remove alice", TagDB_size(tdb) == 0);	
	}
		
	test("close", TagDB_close(tdb));	
	TagDB_delete(tdb);
	
	printf("  ALL TESTS PASSED\n\n"); 
	return 0;
}
