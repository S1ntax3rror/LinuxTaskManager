#ifndef MEMORY_STATS_H
#define MEMORY_STATS_H

typedef struct memory_stats {
    unsigned long mem_total_kb;
    unsigned long mem_free_kb;
    unsigned long mem_available_kb;
    unsigned long buffers_kb;
    unsigned long cached_kb;
    unsigned long swap_total_kb;
    unsigned long swap_free_kb;
    unsigned long swap_used_kb;
} memory_stats;

void read_memory_stats(memory_stats* memstats);

#endif
