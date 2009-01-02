/*#io
QDBM ioDoc(
		 docCopyright("Steve Dekorte", 2002)
		 docLicense("BSD revised")
		 docDescription("A key/value idsbase.")
		 docCategory("Databases")
		 */

#include "TagDB.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

TagDB *TagDB_new(void)
{
	TagDB *self = calloc(1, sizeof(TagDB));
	self->symbols = SymbolDB_new();
	self->results = Uint64Array_new();
	//self->filters = Uint64Array_new();
	//self->filterArea = 64;
	return self;
}

void TagDB_free(TagDB *self) 
{	
	TagDB_close(self);
	SymbolDB_close(self->symbols);
	SymbolDB_free(self->symbols);
	//Uint64Array_free(self->results);
	//Uint64Array_free(self->filters);
	if (self->path) free(self->path);
	if (self->k2tPath) free(self->k2tPath);
	free(self);
}

void TagDB_delete(TagDB *self)
{
	remove(self->k2tPath);
	SymbolDB_delete(self->symbols);
}

void TagDB_setPath_(TagDB *self, char *path)
{
	char *ext = ".k2t";
	self->path = strcpy(realloc(self->path, strlen(path) + 1), path);
	self->k2tPath = realloc(self->k2tPath, strlen(self->path) + strlen(ext) + 1);
	strcat(strcpy(self->k2tPath, path), ext);
	
	SymbolDB_setPath_(self->symbols, path);
}

char *TagDB_path(TagDB *self)
{
	return self->path;
}

// -------------------------------------------------------- 

#include <assert.h>

int TagDB_read(TagDB *self)
{
	TCBDB *v = self->keyToTags;
	
	uint64_t key;
	int keySize;
	
	uint64_t *value;
	int valueSize;
	
	size_t index = 0;
	size_t size = tcbdbrnum(v);
	
	self->items = realloc(self->items, size * sizeof(void *));
	
	tcbdbcurfirst(self->cursor);
	
	for(;;)
	{
		char *keyValue = tcbdbcurkey(self->cursor, &keySize);
		
		if (!keyValue) break;
		if (keySize != sizeof(uint64_t)) 
		{
			printf("TagDB error: keySize != sizeof(uint64_t)\n");
			return 0;
		}
		
		key = ((uint64_t *)keyValue)[0];
		value = (uint64_t *)tcbdbcurval(self->cursor, &valueSize);
		
		if (valueSize % sizeof(uint64_t) != 0)
		{
			printf("TagDB error: valueSize %% sizeof(uint64_t) != 0\n");
			return 0;
		}
		
		self->items[index] = TagDBItem_new(key, value, valueSize / sizeof(uint64_t));
		
		if (keyValue) free(keyValue);
		if (value) free(value);
		index ++;
		tcbdbcurnext(self->cursor);
	}
	
	self->itemCount = index;
	return 1;
}

int TagDB_open(TagDB *self)
{
	TagDB_close(self);
	
	self->keyToTags = tcbdbnew();

	tcbdbsetcmpfunc(self->keyToTags, tcbdbcmpint64, NULL); // tcbdbcmplexical
	
	if (!tcbdbopen(self->keyToTags, self->k2tPath, BDBOWRITER | BDBOCREAT | BDBOLCKNB))
	{
		fprintf(stderr, "tcbdbopen '%s' failed\n", self->k2tPath);
		return 0;
	}
	
	self->cursor = tcbdbcurnew(self->keyToTags);
	
	if (!self->cursor)
	{
		fprintf(stderr, "tcbdbcurnew on '%s' failed\n", self->k2tPath);
		return 0;
	}	
		
	SymbolDB_setPath_(self->symbols, self->path);
	return TagDB_read(self) && 	SymbolDB_open(self->symbols);
}

int TagDB_close(TagDB *self)
{
	if (self->keyToTags) 
	{
		tcbdbcurdel(self->cursor);
		tcbdbclose(self->keyToTags);
		self->keyToTags = NULL;
	}
	
	if (self->items)
	{
		size_t i;
		
		for (i = 0; i < self->itemCount; i ++)
		{
			TagDBItem_free(self->items[i]);
		}
		
		free(self->items);
		self->items = NULL;
		
		self->itemCount = 0;
	}
	
	return 1;
}

void TagDB_begin(TagDB *self)
{
	if (self->keyToTags)
	{
		tcbdbtranbegin(self->keyToTags);
		SymbolDB_begin(self->symbols);
	}
	//printf("begin\n");
}

void TagDB_commit(TagDB *self)
{
	if (self->keyToTags)
	{
		tcbdbtrancommit(self->keyToTags);
		SymbolDB_commit(self->symbols);
	}
	//printf("commit\n");
}

size_t TagDB_indexToInsertKey(TagDB *self, uint64_t key)
{ 
	long low  = -1; 
	long high = self->itemCount;
	
	// most will be appends
	
	if (self->itemCount > 0 && key > self->items[self->itemCount - 1]->key) 
	{
		return self->itemCount;
	}
	
	while (high - low > 1)
	{
		long i = (high + low) / 2;
		uint64_t k;
		
		k = self->items[i]->key;
		
		if (k == key) { return i; }
		if (k > key) { high = i; } else { low  = i; }
	}
	
	return high;
}

long TagDB_indexForKey(TagDB *self, uint64_t key)
{	
	long index = TagDB_indexToInsertKey(self, key);
	if (index == self->itemCount) return -1;
	return (self->items[index]->key == key) ? index : -1;
}

int TagDB_atKey_putTags_(TagDB *self, uint64_t key, Uint64Array *tags)
{
	size_t itemCount = self->itemCount;
	long index = TagDB_indexToInsertKey(self, key);
	TagDBItem *item;
	int result;
		
	if (index == itemCount || self->items[index]->key != key)
	{		 
		//printf("insert %i\n", index);
		
		item = TagDBItem_new(key, tags->ids, tags->size);
		self->items = realloc(self->items, (self->itemCount + 1) * sizeof(void *));
		
		if (index != itemCount)
		{
			memmove(self->items + index + 1, self->items + index, 
				   (itemCount - index) * sizeof(void *));
		}
		
		self->items[index] = item;
		self->itemCount ++;
	}
	else
	{
		//printf("update %i\n", index);

		item = self->items[index];
		TagDBItem_setTags_(item, tags);
	}
	
	// insert/update on-disk
		
	//tcbdbtranbegin(self->keyToTags);
	result = tcbdbput(self->keyToTags, 
				(const char *)&(item->key), sizeof(uint64_t), 
				(const char *)item->tags.ids, item->tags.size * sizeof(uint64_t));
	//tcbdbtrancommit(self->keyToTags);
	if (!result) printf("error writting to tadDB\n");
	
	//self->needsToBuildFilters = 1;
	return result;
}

// access

Uint64Array *TagDB_tagsAtKey_(TagDB *self, uint64_t key)
{
	long index = TagDB_indexForKey(self, key);
	
	if (index != -1)
	{
		TagDBItem *item = self->items[index];
		return &(item->tags);
	}
	
	return NULL;
}

Datum *TagDB_keyAtIndex_(TagDB *self, size_t index)
{
	if (index < self->itemCount)
	{
		return TagDB_symbolForId_(self, self->items[index]->key);
	}
	
	return NULL;
}

// remove

int TagDB_removeKey_(TagDB *self, uint64_t key)
{
	int result = 0;	
	long index = TagDB_indexForKey(self, key);
	
	if (index != -1)
	{
		// remove in-memory
		
		memmove(self->items + index, self->items + index + 1, 
			   (self->itemCount - (index + 1)) * sizeof(void *));
		self->itemCount --;
		
		// remove on-disk
		
		//tcbdbtranbegin(self->keyToTags);
		result = tcbdbout(self->keyToTags, (const char *)&key, sizeof(uint64_t));
		//tcbdbtrancommit(self->keyToTags);
	}
	
	//self->needsToBuildFilters = 1;
	return result;
}

TAGDB_API size_t TagDB_removeTag_(TagDB *self, uint64_t tag)
{	
	size_t size = self->itemCount;
	size_t index; 
	TagDBItem **items = self->items;
	size_t changeCount = 0;
	
	for (index = 0; index < size; index ++)
	{
		TagDBItem *item = items[index];
		changeCount += Uint64Array_remove_(TagDBItem_tags(item), tag);
		tcbdbput(self->keyToTags, 
						  (const char *)&(item->key), sizeof(uint64_t), 
						  (const char *)item->tags.ids, item->tags.size * sizeof(uint64_t));
	}
	
	return changeCount;
}

size_t TagDB_size(TagDB *self)
{
	if (!self->keyToTags) return 0;
	return self->itemCount;
}

#ifdef WIN32
#define inline __inline
#endif

inline static int TagDBItem_matches_(TagDBItem *self, Uint64Array *tags, int exclusive)
{
	uint64_t *a = tags->ids;
	int aSize  = tags->size;
	
	uint64_t *b = self->tags.ids;
	int bSize = self->tags.size;
	
	int i = 0, j = 0;
	
	// return true if all of a are found in b - assume no duplicate items
	
	//if (aSize > bSize) return 0;
	if (exclusive)
	{
		return Uint64Array_equals_(TagDBItem_tags(self), tags);
	}
	else
	{
		while (i < aSize)
		{		
			for(; j < bSize; j ++)
			{
				if (a[i] <  b[j]) { return 0;  } // since they are sorted
				if (a[i] == b[j]) { goto next; }			
			}
			
			return 0;
		next:
			i ++;
		}
	}
	
	return 1;
}

Uint64Array *TagDB_keysForTags_exclusive_(TagDB *self, Uint64Array *tags, int exclusive)
{	
	size_t size = self->itemCount;
	size_t index; 
	TagDBItem **items = self->items;
	Uint64Array *results = self->results;
	
	Uint64Array_sort(tags);
	
	results->ids = realloc(results->ids, size * sizeof(uint64_t));
	results->size = 0;
	
	for (index = 0; index < size; index ++)
	{
		TagDBItem *item = items[index];
		
		if (TagDBItem_matches_(item, tags, exclusive))
		{
			results->ids[results->size] = item->key;
			results->size ++;
		}
	}
	
	return results;
}

Uint64Array *TagDB_keysForTags_(TagDB *self, Uint64Array *tags)
{	
	return TagDB_keysForTags_exclusive_(self, tags, 0);
}

Uint64Array *TagDB_untaggedKeys(TagDB *self)
{
	Uint64Array *tags = Uint64Array_new();
	Uint64Array *r = TagDB_keysForTags_exclusive_(self, tags, 1);
	Uint64Array_free(tags);
	return r;
}

/*
Uint64Array *TagDB_keysForTags_(TagDB *self, Uint64Array *tags)
{	
	size_t size = self->itemCount;
	size_t index; 
	TagDBItem **items = self->items;
	Uint64Array *results = self->results;
	size_t filterIndex; 
	uint64_t filter = Uint64Array_filter(tags);
	
	Uint64Array_sort(tags);
	
	results->ids = realloc(results->ids, size * sizeof(uint64_t));
	results->size = 0;
	

	TagDB_buildFilters(self);
	for (filterIndex = 0; filterIndex < Uint64Array_size(self->filters);  filterIndex ++)
	{
		uint64_t f = Uint64Array_at_(self->filters, filterIndex);
		if ((filter & f) == filter)
		{	
			size_t begin = filterIndex * self->filterArea;
			size_t end = (filterIndex + 1) * self->filterArea;
			
			if (end > size) end = size;

			for (index = begin; index < end; index ++)
			{
				TagDBItem *item = items[index];
						
				if (TagDBItem_matches_(item, tags))
				{
					results->ids[results->size] = item->key;
					results->size ++;
				}
			}
		}
		else
		{
			//printf("skipped %i-%i\n", filterIndex * self->filterArea, (filterIndex + 1) * self->filterArea);
		}
	}
	
	return results;
}
*/

void TagDB_show(TagDB *self)
{
	size_t i;
	
	printf("TagDB:\n");
	
	for (i = 0; i < self->itemCount; i ++)
	{
		TagDBItem *item = self->items[i];
		TagDBItem_show(item);
	}
	
	printf("\n");
}

Datum *TagDB_symbolForId_(TagDB *self, symbolid_t key)
{
	return SymbolDB_symbolForId_(self->symbols, key);
}

symbolid_t TagDB_idForSymbol_size_(TagDB *self, const char *s, size_t size)
{
	symbolid_t id;
	Datum d;
	d.data = (unsigned char *)s;
	d.size = size;
	id = SymbolDB_idForSymbol_(self->symbols, &d);
	//printf("%s %i\n", s, (int)id);
	return id;
}

symbolid_t TagDB_idForSymbol_(TagDB *self, const char *s)
{
	return TagDB_idForSymbol_size_(self, s, strlen(s));
}

/*
void TagDB_buildFilters(TagDB *self)
{
	if (self->needsToBuildFilters)
	{
		size_t index;
		uint64_t filter = 0;
		
		printf("build a filter\n");
		
		Uint64Array_removeAll(self->filters);
		
		for (index = 0; index < self->itemCount; index ++)
		{
			uint64_t f = Uint64Array_filter(TagDBItem_tags(self->items[index]));
			filter = filter | f;
			
			if (index > 0 && index % self->filterArea == 0)
			{
				Uint64Array_append_(self->filters, filter);
				filter = 0;
			}
		}
		
		if (index % self->filterArea != 0)
		{
			Uint64Array_append_(self->filters, filter);
		}
		
		self->needsToBuildFilters = 0;
	}
}
*/

Uint64Array *TagDB_allUniqueTags(TagDB *self)
{	
	// inefficient if number of unique tags is high 
	// move to using a hash table for uniqueTags later
	size_t size = self->itemCount;
	TagDBItem **items = self->items;
	Uint64Array *uniqueTags = Uint64Array_new();
	size_t index;
	
	for (index = 0; index < size; index ++)
	{
		Uint64Array *itemTags = TagDBItem_tags(items[index]);
		size_t i;
		
		for (i = 0; i < Uint64Array_size(itemTags); i ++)
		{
			uint64_t id = Uint64Array_at_(itemTags, i);
			
			if (!Uint64Array_contains_(uniqueTags, id))
			{
				Uint64Array_append_(uniqueTags, id);
			}
		}
	}

	return uniqueTags;
}


