.RECIPEPREFIX = >
.PHONY: all clean

CC        := gcc
# expose GNU extensions (prlimit, MHD_USE_GNU_SOURCE)
CFLAGS    := -D_GNU_SOURCE -D_DEFAULT_SOURCE -Iinclude -I../c-core/include -Wall -O2
LIBS      := -lmicrohttpd -lcjson

SRCDIR      := src
CORE_SRCDIR := ../c-core/src

SRC := \
    $(wildcard $(SRCDIR)/*.c)                           \
    $(CORE_SRCDIR)/general_stat_query.c                  \
    $(CORE_SRCDIR)/proc_stat.c                           \
    $(CORE_SRCDIR)/trimmed_info.c                        \
    $(CORE_SRCDIR)/memory_stats.c                        \
    $(CORE_SRCDIR)/utils.c                               \
    $(CORE_SRCDIR)/cpu_usage.c                           \
    $(CORE_SRCDIR)/process_sort.c                        \
    $(CORE_SRCDIR)/core_interface.c						 \
    $(CORE_SRCDIR)/sleeper_detection.c

OBJ := $(SRC:.c=.o)

all: http_server

# link
http_server: $(OBJ)
>   $(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# compile
%.o: %.c
>   $(CC) $(CFLAGS) -c $< -o $@

clean:
>   rm -f $(OBJ) http_server
