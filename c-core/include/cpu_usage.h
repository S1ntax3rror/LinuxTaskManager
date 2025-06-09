#ifndef CPU_USAGE_H
#define CPU_USAGE_H

#include "general_stat_query.h"
#include "proc_stat.h"

void calculate_normalized_cpu_usage(proc_stat* before, proc_stat* after, uint64_t interval_seconds);
void calculate_normalized_core_cpu_usage(general_stat* before, general_stat* after, uint64_t interval_seconds);
void calculate_normalized_general_cpu_usage(general_stat* before, general_stat* after, uint64_t interval_seconds);

#endif
