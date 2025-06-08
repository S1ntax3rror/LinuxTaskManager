#include "../include/sleeper_detection.h"
#include "../include/uthash.h"
#include <time.h>

ProcessInfo *process_map = NULL;

int update_sleeper_process(pid_t pid, long current_cpu_time, double memory_percent) {
    time_t now = time(NULL);
    ProcessInfo *p;
    HASH_FIND_INT(process_map, &pid, p);

    if (!p) {
        p = malloc(sizeof(ProcessInfo));
        p->pid = pid;
        p->last_active = now;
        p->last_cpu_time = current_cpu_time;
        HASH_ADD_INT(process_map, pid, p);
        return 0;
     }

    if (current_cpu_time > p->last_cpu_time) {
        p->last_cpu_time = current_cpu_time;
        p->last_active = now;
    }

    if (now - p->last_active > 30 && memory_percent > 1) {
        return 1;
    } else {
        return 0;
    }
}
