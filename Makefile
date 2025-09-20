src = $(wildcard src/*.c)
test_src = $(filter-out src/main.c, $(wildcard src/*.c))
obj = $(patsubst src/%.c,bin/%.o,$(src))
test_obj = $(patsubst src/%.c,bin/%.o,$(test_src))
CC = gcc
CFLAGS = -g -pedantic -Wall -Wextra -Werror \
	-Dfread=BANNED_fread \
	-Dfwrite=BANNED_fwrite \
	-Dfscanf=BANNED_fscanf \
	-Dfprintf=BANNED_fprintf \
	-Dfopen=BANNED_fopen \
	-Dfclose=BANNED_fclose \
	-Dsscanf=BANNED_sscanf \
	-Dsprintf=BANNED_sprintf \
	-Dsnprintf=BANNED_snprintf \
	-Dscanf=BANNED_scanf \
	-Dgets=BANNED_gets \
	-Dputs=BANNED_puts \
	-Dputchar=BANNED_putchar \
	-Dgetchar=BANNED_getchar \
	-Dfgetc=BANNED_fgetc \
	-Dfputc=BANNED_fputc \
	-Dfgets=BANNED_fgets \
	-Dfputs=BANNED_fputs

PFLAGS =
ifeq ($(shell uname -s),Linux)
	PFLAGS = -lnsl
endif

LDFLAGS = $(PFLAGS)

build/a.out: $(obj)
	@($(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS))

run: build/a.out
	@(echo "--- Build complete ---")
	@(cd build)
	@(./build/a.out 9545)
	@(cd ..)

task-run: clean build/a.out
	@(echo "*--------------* build complete *--------------*")
	@(cd build && ./a.out 9545)
	@(echo "")
	@(exec /opt/homebrew/bin/zsh && cd ~/tufts_dev/cs_112/cs112_a0)

test/test_http_parser: test/test_http_parser.c $(test_obj)
	@($(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS))

task-test: clean test/test_http_parser
	@(echo "*--------------* build complete *--------------*")
	@(./test/test_http_parser)
	@(echo "")
	@(exec /opt/homebrew/bin/zsh && cd ~/tufts_dev/cs_112/cs112_a0)


# Compile src/foo.c into build/foo.o (ensure build/ exists first)
bin/%.o: src/%.c | bin
	@($(CC) $(CFLAGS) -c -o $@ $<)

# Create the output directory (not phony so Make can check its timestamp)
bin:
	@(mkdir -p $@)

.PHONY: clean
clean:
	rm -rf bin build/a.out test/test_http_parser test/test_http_parser.dSYM
