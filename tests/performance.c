#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

double Date_SecondsFrom1970ToNow(void)
{
	double s, us, t;
	struct timeval timeval;
	struct timezone timezone;

	if(gettimeofday(&timeval, &timezone) == -1) { printf("gettime failed\n"); exit(-1); }
	s = timeval.tv_sec;
	us = timeval.tv_usec/1000000.0;
	s = ((long)s) % 1000;
	//printf("s  = %f\n", (float)s);
	//printf("us = %f\n", (float)us);
	t = s + us;
	//s += us;
	//printf("t = %f\n", (float)t);
	return t;
}

#include "TagDB.h"

static double timerStart;

void Timer_Begin(void)
{	
	timerStart = Date_SecondsFrom1970ToNow();
}

double Timer_End(void)
{
	double t = Date_SecondsFrom1970ToNow();
	//printf("timerStart = %f\n", (float)timerStart);
	//printf("Date_SecondsFrom1970ToNow() = %f\n", (float)t);
	return t - timerStart;
}

void Timer_EndShowNumPerSecond(float num, char *op)
{
	float dt = Timer_End();
	
	if (dt && num)
	{
		char *suffix = "";
		float v = num/dt;
		if (v > 1000000.0) { v /= 1000000.0; suffix = "M"; }
		if (v > 10000.0)   { v /= 1000.0; suffix = "K"; }
		
		printf("  %i%s %ss per second\n", (int)v, suffix, op);
		//printf("  %f seconds for %i %ss\n", (float)dt, (int)num, op);
		//printf("  %f microseconds per %s\n", (float)((dt*1000000.0)/num), op);
	}
	else
	{
		printf("dt = %f\n", dt);
		printf("num = %f\n", num);
		
	}
}

void Timer_EndShowMillisecondsPer(int num, char *op)
{
	if (num) printf("  %ims per %s\n", (int)(Timer_End()*1000.0/num), op);
}

int main(int argc, const char * argv[]) 
{
	TagDB *tagDB = TagDB_new();
	int i;
	float max = 1000000;
	long maxTags = 10000;
	
	printf("\ntagdb performance tests:\n\n");
	printf("  number of entries used for tests: %i\n\n", (int)max);
	
	//Timer_Begin();
		
	TagDB_setPath_(tagDB, "testdb");
	TagDB_delete(tagDB);

	if (TagDB_open(tagDB) == 0)
	{
		exit(-1);
	}
		
	//Timer_EndShowNumPerSecond(TagDB_size(tagDB), " load-record-from-disk op");

	//printf("\n  inserting...\n");
	
	srand(123);
	if (TagDB_size(tagDB) < max)
	{		
		Uint64Array *tags = Uint64Array_new();
		Timer_Begin();
		TagDB_begin(tagDB);
		
		for (i = TagDB_size(tagDB); i < max; i = i + 1)
		{
			Uint64Array_removeAll(tags);
			Uint64Array_append_(tags, rand() % maxTags);
			Uint64Array_append_(tags, rand() % maxTags);
			//Uint64Array_show(tags);
			
			TagDB_atKey_putTags_(tagDB, i, tags);
			
		}
		TagDB_commit(tagDB);
		Uint64Array_free(tags);
		
		Timer_EndShowNumPerSecond(max, "insert");		
	}

	
	//printf("  searching...\n");
	//printf("  TagDB_size(tagDB) = %i\n", TagDB_size(tagDB));
	//TagDB_buildFilters(tagDB);
	
	srand(123);
	{
		int n, numSearches = 100;
		Uint64Array *tags = Uint64Array_new();
		
		Timer_Begin();
		
		for (n = 0; n < numSearches; n ++)
		{
			Uint64Array *results;
		
			Uint64Array_removeAll(tags);
			Uint64Array_append_(tags, rand() % maxTags);
			Uint64Array_append_(tags, rand() % maxTags);
			//Uint64Array_show(tags);

			results = TagDB_keysForTags_(tagDB, tags);
			//printf("%i matches found\n", (int)results->size);
			
		}
		Uint64Array_free(tags);
		
		
		Timer_EndShowNumPerSecond((numSearches), "searche");
	}

	//printf("  closing...\n");

	TagDB_close(tagDB);
	//printf("  reopening...\n");

	Timer_Begin();
	TagDB_open(tagDB);
	Timer_EndShowNumPerSecond(TagDB_size(tagDB), "read");
	
	if (TagDB_size(tagDB) == 0)
	{
		printf("error - no records not found after open\n");
		exit(-1);
	}
	
	//printf("  removing...\n");
	
	{
		size_t numToRemove = 100; //TagDB_size(tagDB);
		
		Timer_Begin();
		TagDB_begin(tagDB);
		for (i = 0; i < numToRemove; i = i + 1)
		{
			TagDB_removeKey_(tagDB, i);
		}
		TagDB_commit(tagDB);
		
		Timer_EndShowNumPerSecond(numToRemove, "remove");
	}
	
	
	TagDB_close(tagDB);
	TagDB_delete(tagDB);

	printf("\n");
	
	return 0;
}
