.RECIPEPREFIX = >

CC      = gcc
CFLAGS  = -Iinclude -I../c-core/include -Wall -O2
LIBS    = -lmicrohttpd -lcjson

SRC = \
    src/main.c \
    src/server.c \
    src/routes.c \
    src/process_api.c \
    src/stats_api.c \
    src/core_interface.c \
    ../c-core/src/general_stat_query.c \
    ../c-core/src/proc_stat.c \
    ../c-core/src/trimmed_info.c \
    ../c-core/src/memory_stats.c \
    ../c-core/src/utils.c \
    ../c-core/src/cpu_usage.c \
    ../c-core/src/process_sort.c

OBJ = $(SRC:.c=.o)

all: http_server

http_server: $(OBJ)
>$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
>rm -f $(OBJ) http_server
