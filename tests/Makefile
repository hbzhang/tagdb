OS ?= $(shell uname -s)

CFLAGS := -I../_build/headers 
LFLAGS := -L../_build/lib -ltagdb -L../../basekit/_build/lib -ltokyocabinet

NEEDS_M := FreeBSD Linux NetBSD
ifneq (,$(findstring $(OS),$(NEEDS_M)))
LFLAGS +=-lm
endif

all:
	$(CC) $(CFLAGS) performance.c -o performance $(LFLAGS)

clean:
	-rm performance

test:
	./performance
