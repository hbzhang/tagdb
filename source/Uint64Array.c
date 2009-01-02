
#include "Uint64Array.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Uint64Array *Uint64Array_new(void)
{
	Uint64Array *self = calloc(1, sizeof(Uint64Array));
	return self;
}

void Uint64Array_removeAll(Uint64Array *self)
{
	free(self->ids);
	self->ids = NULL;
	self->size = 0;
}

void Uint64Array_append_(Uint64Array *self, uint64_t v)
{
	self->ids = realloc(self->ids, (self->size + 1) * sizeof(uint64_t));
	self->ids[self->size] = v;
	self->size ++;
}

void Uint64Array_appendIfAbsent_(Uint64Array *self, uint64_t v)
{
	if (!Uint64Array_contains_(self, v))
	{
		Uint64Array_append_(self, v);
	}
}


Uint64Array *Uint64Array_clone(Uint64Array *self)
{
	Uint64Array *ka = Uint64Array_new();
	Uint64Array_copy_(ka, self);
	return ka;
}

void Uint64Array_copy_(Uint64Array *self, Uint64Array *other)
{
	self->size = other->size;
	self->ids = realloc(self->ids, other->size * sizeof(uint64_t));
	memcpy(self->ids, other->ids, other->size * sizeof(uint64_t));
}

int Uint64Array_equals_(Uint64Array *self, Uint64Array *other)
{	
	if (self->size != other->size) return 0;
	return memcmp(self->ids, other->ids, sizeof(uint64_t) * self->size) == 0;
}

static int TagIdCompare(const void *a, const void *b)
{
	const uint64_t aa = *(uint64_t *)a;
	const uint64_t bb = *(uint64_t *)b;
	if (aa == bb) return 0;
	return (aa > bb) ? 1 : -1;
}

void Uint64Array_sort(Uint64Array *self)
{
	qsort(self->ids, self->size, sizeof(uint64_t), TagIdCompare);
}

int Uint64Array_remove_(Uint64Array *self, uint64_t v)
{
	size_t i;
	size_t j = 0;
	int result = 0;
	
	for (i = 0; i < self->size; i ++)
	{
		if (i != j) self->ids[j] = self->ids[i];
		
		if (self->ids[i] != v) 
		{
			j ++;
			result = 1;
		}
	}
	
	
	self->size = j;
	
	return result;
}

void Uint64Array_removeDuplicates(Uint64Array *self)
{
	//qsort(self->ids, self->size, sizeof(uint64_t), TagIdCompare);
}

size_t Uint64Array_size(Uint64Array *self)
{
	return self->size;
}

uint64_t Uint64Array_at_(Uint64Array *self, size_t index)
{
	return self->ids[index];
}

int Uint64Array_contains_(Uint64Array *self, uint64_t v)
{
	size_t i;
		
	for (i = 0; i < self->size; i ++)
	{
		if (self->ids[i] == v) return 1;
	}

	return 0;
}


void Uint64Array_show(Uint64Array *self)
{
	size_t i;
	
	printf("[");
	
	for (i = 0; i < self->size; i ++)
	{
		if (i > 0) printf(" ");
		printf("%i", (int)self->ids[i]);
	}
	
	printf("]");
}

void Uint64Array_free(Uint64Array *self)
{
	if (self->ids) free(self->ids);
	free(self);
}

uint64_t Uint64Array_filter(Uint64Array *self)
{
	size_t i;
	uint64_t filter = 0;
	
	for (i = 0; i < self->size; i ++)
	{
		int shift = self->ids[i] % 63;
		uint64_t f = 0x1 << shift;
		filter |= f;
	}
	
	return filter;
}

void *Uint64Array_data(Uint64Array *self)
{
	return (void *)self->ids;
}

