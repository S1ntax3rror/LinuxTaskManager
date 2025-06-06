#include "../include/timeline.h"
#include "../include/cpu_usage.h"
#include <unistd.h>
#include <stdint.h>

void calculate_normalized_cpu_usage(proc_stat* before, proc_stat* after, uint64_t interval_seconds) {
    long clock_ticks = sysconf(_SC_CLK_TCK);
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    uint64_t proc_time_before = before->utime + before->stime;
    uint64_t proc_time_after = after->utime + after->stime;
    uint64_t delta_proc_time = proc_time_after - proc_time_before;
    double seconds_used = ((double)delta_proc_time*1000) / clock_ticks;
    
    if (delta_proc_time > 0){
        after->cpu_percent = 100.0 * (seconds_used / (interval_seconds * num_cores));
    } else {
        after->cpu_percent = 0.0;
    }
}

double calculate_avg_cpu_percent(proc_timeline* timeline) {
    int count = 0;
    double total = 0.0;

    for (int i = 0; i < MAX_TIMELINE; i++) {
        if (timeline->timeline[i].pid == -1) continue;  // skip uninitialized
        total += timeline->timeline[i].cpu_percent;
        count++;
    }

    return (count > 0) ? (total / count) : 0.0;
}
