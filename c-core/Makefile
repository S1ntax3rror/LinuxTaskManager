# c-core/Makefile

.PHONY: all debug clean

# Use _DEFAULT_SOURCE so that DT_DIR & stuff from <dirent.h> are exposed
CFLAGS   := -D_DEFAULT_SOURCE -Iinclude -Wall -O2

all:
	gcc $(CFLAGS) -o process_monitor_query                      \
	    src/main.c                                            \
	    src/proc_stat.c                                       \
	    src/utils.c                                           \
	    src/general_stat_query.c                              \
	    src/cpu_usage.c                                       \
	    src/memory_stats.c                                    \
	    src/trimmed_info.c                                    \
	    src/process_sort.c					  \
	    src/header.c

debug:
	gcc -g $(CFLAGS) -o process_monitor_query_debug             \
	    src/main.c src/proc_stat.c src/utils.c                  \
	    src/general_stat_query.c src/cpu_usage.c                \
	    src/memory_stats.c src/trimmed_info.c src/process_sort.c 							\ 
	    src/header.c

clean:
	rm -f process_monitor_query process_monitor_query_debug
