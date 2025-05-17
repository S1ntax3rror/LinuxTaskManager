#ifndef TRIMMED_INFO_H
#define TRIMMED_INFO_H

#include <stdint.h>
#include <time.h>
#include "proc_stat.h"

typedef struct trimmed_info {
    int       pid;
    char      comm[512];
    char      state;
    int       nice;            // <-- newly added

    double    cpu_percent;
    double    ram_percent;

    uint64_t  timestamp_ms;
    char      time_str[16];

    double    avg_cpu_percent;
    double    peak_ram_percent;
} trimmed_info;

/**
 * Build a trimmed_info from a full proc_stat snapshot.
 */
trimmed_info convert_to_trimmed_info(const proc_stat* proc);

#endif // TRIMMED_INFO_H
