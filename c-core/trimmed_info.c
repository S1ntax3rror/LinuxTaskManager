#include "trimmed_info.h"
#include "proc_stat.h"
#include <string.h>
#include <time.h>      
#include <stdio.h>     
#include <stdint.h> 

trimmed_info convert_to_trimmed_info(const proc_stat* proc) {
    trimmed_info t;
    t.pid = proc->pid;
    strncpy(t.comm, proc->comm, sizeof(t.comm));
    t.comm[sizeof(t.comm) - 1] = '\0'; // ensure null-termination
    t.state = proc->state;
    t.cpu_percent = proc->cpu_percent;

    t.ram_percent = proc->ram_percent;
    if (t.ram_percent > t.peak_ram_percent){
        t.peak_ram_percent = t.ram_percent;
    }

    t.timestamp_ms = proc-> timestamp_ms;
    time_t secs = proc->timestamp_ms / 1000;
    struct tm* tm_info = localtime(&secs);
    strftime(t.time_str, sizeof(t.time_str), "%H:%M:%S", tm_info);

    return t;

}

