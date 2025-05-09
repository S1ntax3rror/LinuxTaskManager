all:
	gcc -o process_monitor_query src/main.c src/proc_stat.c src/utils.c src/general_stat_query.c src/cpu_usage.c src/memory_stats.c src/trimmed_info.c src/process_sort.c

debug:
	gcc -g -o process_monitor_query main.c proc_stat.c utils.c general_stat_query.c cpu_usage.c memory_stats.c trimmed_info.c process_sort.c


clean:
	rm -f process_monitor_query
