#ifndef TRIMMED_INFO_H
#define TRIMMED_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "proc_stat.h"

typedef struct trimmed_info {
    int pid;
    char comm[512];
    char state;

    double cpu_percent;
    double ram_percent;

    uint64_t timestamp_ms;
    char time_str[16];         

    double avg_cpu_percent;      
    double peak_ram_percent; 

} trimmed_info;

trimmed_info convert_to_trimmed_info(const proc_stat* stat);

#endif // TRIMMED_INFO_H