#ifndef TRIMMED_INFO_H
#define TRIMMED_INFO_H

#include <stdint.h>
#include <time.h>
#include "proc_stat.h"

typedef struct trimmed_info {
    int       pid;
    char      comm[512];
    char      state;
    int       nice;  
    
      /* ─── NEW FIELDS ─── */
    char      username[64];      // process owner’s username
    int       prio;              // priority from proc_stat.priority
    unsigned long virt_kb;       // virtual memory size in KiB
    unsigned long res_kb;        // resident set size in KiB
    unsigned long shared_kb;     // shared “text” memory in KiB
    char      cmd[4096];         // full command line (args)
    double    up_time_seconds;   // how long this process has been running

    double    cpu_percent;
    double    ram_percent;

    uint64_t  timestamp_ms;
    char      time_str[16];

    double    avg_cpu_percent;
    double    peak_ram_percent;

    char      cmd[500];
    int       is_sleeper;
} trimmed_info;

/**
 * Build a trimmed_info from a full proc_stat snapshot.
 */
trimmed_info convert_to_trimmed_info(const proc_stat* proc);

#endif // TRIMMED_INFO_H
