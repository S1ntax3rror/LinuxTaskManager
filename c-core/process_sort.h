#ifndef PROCESS_SORT_H
#define PROCESS_SORT_H

#include "trimmed_info.h"


typedef enum {
    SORT_BY_CPU,
    SORT_BY_RAM,
    SORT_BY_AVG_CPU,
    SORT_BY_PID,
    SORT_BY_NAME,
    SORT_BY_STATE
} SortMode;

void sort_by_cpu(trimmed_info* list, int size);
void sort_by_ram(trimmed_info* list, int size);
void sort_by_name(trimmed_info* list, int size);
void sort_by_pid(trimmed_info* list, int size);
void sort_by_avg_cpu(trimmed_info* list, int size);
void sort_by_state(trimmed_info* list, int size);

#endif // PROCESS_SORT_H
