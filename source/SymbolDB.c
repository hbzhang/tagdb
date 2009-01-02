
#include "SymbolDB.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

SymbolDB *SymbolDB_new(void)
{
	SymbolDB *self = calloc(1, sizeof(SymbolDB));
	return self;
}

void SymbolDB_free(SymbolDB *self) 
{	
	if (self->path) free(self->path);
	if (self->s2iPath) free(self->s2iPath);
	if (self->i2sPath) free(self->i2sPath);
	SymbolDB_close(self);
	free(self);
}

void SymbolDB_setPath_(SymbolDB *self, char *path)
{
	char *ext;
	self->path = strcpy(realloc(self->path, strlen(path) + 1), path);
	
	ext = ".s2i";
	self->s2iPath = realloc(self->s2iPath, strlen(self->path) + strlen(ext) + 1);
	strcat(strcpy(self->s2iPath, path), ext);
	
	ext = ".i2s";
	self->i2sPath = realloc(self->i2sPath, strlen(self->path) + strlen(ext) + 1);
	strcat(strcpy(self->i2sPath, path), ext);	
}

char *SymbolDB_path(SymbolDB *self)
{
	return self->path;
}

void SymbolDB_delete(SymbolDB *self)
{
	remove(self->s2iPath);
	remove(self->i2sPath);
}

// -------------------------------------------------------- 

int SymbolDB_open(SymbolDB *self)
{	
	SymbolDB_close(self);
	
	self->s2i = tcbdbnew();
	tcbdbsetcmpfunc(self->s2i, tcbdbcmplexical, NULL);	
	if (!tcbdbopen(self->s2i, self->s2iPath, BDBOWRITER | BDBOCREAT | BDBOLCKNB))
	{
		fprintf(stderr, "tcbdbopen '%s' failed\n", self->s2iPath);
		SymbolDB_close(self);
		return 0;
	}
	
	self->i2s = tcbdbnew();
	tcbdbsetcmpfunc(self->i2s, tcbdbcmpint64, NULL); 
	if (!tcbdbopen(self->i2s, self->i2sPath, BDBOWRITER | BDBOCREAT | BDBOLCKNB))
	{
		fprintf(stderr, "tcbdbopen '%s' failed\n", self->i2sPath);
		SymbolDB_close(self);
		return 0;
	}
	
	self->i2sCursor = tcbdbcurnew(self->i2s);	
	if (!self->i2sCursor)
	{
		fprintf(stderr, "tcbdbcurnew on '%s' failed\n", self->i2sPath);
		SymbolDB_close(self);
		return 0;
	}	
		
	return 1;
}

int SymbolDB_close(SymbolDB *self)
{
	if (self->s2i) 
	{
		tcbdbclose(self->s2i);		
		self->s2i = NULL;
	}
	
	if (self->i2sCursor) 
	{
		tcbdbcurdel(self->i2sCursor);
		self->i2sCursor = NULL;
	}
	
	if (self->i2s) 
	{
		tcbdbclose(self->i2s);
		self->i2s = NULL;
	}
	
	return 1;
}

void SymbolDB_begin(SymbolDB *self)
{
	if (self->s2i) 
	{
		tcbdbtranbegin(self->s2i);		
	}
	
	if (self->i2s) 
	{
		tcbdbtranbegin(self->i2s);
	}
}

void SymbolDB_commit(SymbolDB *self)
{
	if (self->s2i) 
	{
		tcbdbtrancommit(self->s2i);		
	}
	
	if (self->i2s) 
	{
		tcbdbtrancommit(self->i2s);
	}
}


size_t SymbolDB_size(SymbolDB *self)
{
	return self->s2i ? tcbdbrnum(self->s2i) : 0;
}


void SymbolDB_show(SymbolDB *self)
{
	printf("SymbolDB:\n");
		
	printf("\n");
}

// high level API

Datum *SymbolDB_symbolForId_(SymbolDB *self, symbolid_t key)
{
	int size;
	unsigned char *name = (unsigned char *)tcbdbget(self->i2s, (const char *)&key, sizeof(symbolid_t), &size);	
	return name == NULL ? NULL : Datum_newData_size_copy_(name, size, 0); 
}

symbolid_t SymbolDB_getNewIncrementalId(SymbolDB *self)
{
	int idSize;
	char *data;
	
	tcbdbcurlast(self->i2sCursor);

	data = tcbdbcurkey(self->i2sCursor, &idSize);
	return (!data) ? 0 : (*(symbolid_t *)data) + 1;
}

symbolid_t SymbolDB_getNewRandomId(SymbolDB *self)
{
	// if this is used, make sure srandom() is not called with the same constant every time the program starts!
	int idSize;
	uint64_t id;
	
	for (;;)
	{
		char *tmp = NULL;
		//printf("id %i already used\n", (int)id);
		id = rand();
		id = id << 32;
		id |= rand();
		tmp = tcbdbget(self->i2s, (const char *)&id, sizeof(symbolid_t), &idSize);
		if (tmp)
		{
			free(tmp);
		}
		else
		{
			free(tmp);
			break;
		}
	} 
	
	return id;
}

symbolid_t SymbolDB_idForSymbol_(SymbolDB *self, Datum *symbol)
{
	int idSize;
	symbolid_t id;
	char *data = tcbdbget(self->s2i, (const char *)Datum_data(symbol), Datum_size(symbol), &idSize);

	// if the symbol is known, return id
	if (data)
	{
		// should probably verify that's it's in the id2symbol table and insert it if not
		id = *(symbolid_t *)data;
		free(data);
	}
	else // otherwise, create an available id
	{		
		id = SymbolDB_getNewIncrementalId(self);
		//id = SymbolDB_getNewRandomId(self);
		
		//assert(keySize == sizeof(symbolid_t));
		
		// add a symbol entry
		
		//tcbdbtranbegin(self->i2s);
		tcbdbput(self->i2s, 
					(const char *)&(id), sizeof(symbolid_t), 
					(const char *)Datum_data(symbol), Datum_size(symbol));
		//tcbdbtrancommit(self->i2s);	
		
		//tcbdbtranbegin(self->s2i);
		tcbdbput(self->s2i, 
					(const char *)Datum_data(symbol), Datum_size(symbol), 
					(const char *)&(id), sizeof(symbolid_t));
		//tcbdbtrancommit(self->s2i);	
	}
	
	return id;
}


Uint64Array *SymbolDB_allSymbolIds(SymbolDB *self)
{
	Uint64Array *ids = Uint64Array_new();
	BDBCUR *cursor = tcbdbcurnew(self->i2s);
	//size_t size = tcbdbrnum(v);
	
	tcbdbcurfirst(cursor);
	
	for(;;)
	{
		int keySize;
		char *keyValue = tcbdbcurkey(cursor, &keySize);
		uint64_t key;
		
		if (!keyValue) break;
		if (keySize != sizeof(uint64_t)) 
		{
			printf("TagDB error: keySize != sizeof(uint64_t)\n");
			return 0;
		}
		
		key = ((uint64_t *)keyValue)[0];
		
		Uint64Array_append_(ids, key);
		
		tcbdbcurnext(cursor);
	}
	
	tcbdbcurdel(cursor);
	
	return ids;
}

