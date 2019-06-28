CFLAGS := -std=c99 # -Wall -Wextra -Werror

all: check
snprintf_realloc: snprintf_realloc.c

.PHONY: check clean
check: snprintf_realloc
	./snprintf_realloc
clean:
	rm -f snprintf_realloc
