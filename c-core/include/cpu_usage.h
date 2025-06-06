#ifndef CPU_USAGE_H
#define CPU_USAGE_H

#include "proc_stat.h"

void calculate_normalized_cpu_usage(proc_stat* before, proc_stat* after, uint64_t interval_seconds);

#endif
