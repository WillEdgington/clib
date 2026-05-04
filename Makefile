CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11 -Iinclude -MMD -MP
LDFLAGS=
DEVFLAGS=-fsanitize=address,undefined -g
RELFLAGS=-O3

LIBNAME=libclib.a
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

TESTSRC = $(wildcard tests/test_*.c)
TESTBIN = $(TESTSRC:.c=)

METRICSRCS = $(wildcard metrics/*.c)
METRICBIN = $(METRICSRCS:.c=)

DEPS = $(OBJ:.o=.d) $(TESTSRC:.c=.d) $(METRICSRCS:.c=.d)

.PHONY: all clean test debug metric

all: $(LIBNAME)

debug: CFLAGS += $(DEVFLAGS)
debug: LDFLAGS += $(DEVFLAGS)
debug: all

$(LIBNAME): $(OBJ)
		ar rcs $@ $^

# rule for source objects
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# make test
test: CFLAGS += $(DEVFLAGS)
test: LDFLAGS += $(DEVFLAGS)
test: $(TESTBIN)
		@for bin in $(TESTBIN); do ./$$bin; done

tests/%: tests/%.c $(LIBNAME)
	$(CC) $(CFLAGS) $< -L. -lclib $(LDFLAGS) -o $@

# make metric
metric: CFLAGS += $(RELFLAGS)
metric: $(METRICBIN)
		@for bin in $(METRICBIN); do ./$$bin; done

metrics/%: metrics/%.c $(LIBNAME)
	$(CC) $(CFLAGS) $< -L. -lclib $(LDFLAGS) -o $@

-include $(DEPS)

clean:
	rm -f src/*.o $(LIBNAME) $(TESTBIN) $(METRICBIN) $(DEPS)
