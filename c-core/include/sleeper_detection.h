#ifndef PROCESS_TRACKER_H
#define PROCESS_TRACKER_H

#include <time.h>
#include <unistd.h>
#include "uthash.h"

typedef struct {
    pid_t pid;
    time_t last_active;     // in seconds
    long last_cpu_time;     // in clock ticks
    UT_hash_handle hh;
} ProcessInfo;

extern ProcessInfo *process_map;

// Updates or adds process info and detects sleepers
int update_sleeper_process(pid_t pid, long current_cpu_time);

#endif // PROCESS_TRACKER_H
