CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11 -Iinclude -MMD -MP
LDFLAGS=
DEVFLAGS=-fsanitize=address,undefined -g
RELFLAGS=-O3

LIBNAME=libclib.a
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

TESTSRC = $(wildcard tests/test_*.c)
TESTBIN = $(TESTSRC:tests/%.c=%)

DEPS = $(OBJ:.o=.d) $(TESTSRC:tests/%.c=%.d)

.PHONY: all clean test debug

all: $(LIBNAME)

debug: CFLAGS += $(DEVFLAGS)
debug: LDFLAGS += $(DEVFLAGS)
debug: all

$(LIBNAME): $(OBJ)
		ar rcs $@ $^

# rule for source objects
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: CFLAGS += $(DEVFLAGS)
test: LDFLAGS += $(DEVFLAGS)
test: $(TESTBIN)
		@for bin in $(TESTBIN); do ./$$bin; done

%: tests/%.c $(LIBNAME)
		$(CC) $(CFLAGS) $< -L. -lclib $(LDFLAGS) -o $@

-include $(DEPS)

clean:
		rm -f src/*.o src/*.d $(LIBNAME) $(TESTBIN)