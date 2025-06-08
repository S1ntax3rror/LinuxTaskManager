#include "../include/timeline.h"
#include "../include/cpu_usage.h"
#include "../include/general_stat_query.h"
#include <stdio.h>
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

void calculate_normalized_core_cpu_usage(general_stat* before, general_stat* after, uint64_t interval_ms) {
    long clock_ticks = sysconf(_SC_CLK_TCK);
    
    if (!before || !after || after->num_cpus <= 0) {
        memset(after->cores, 0, sizeof(after->cores));
        return;
    }
    
    for (int i=0;i<after->num_cpus;i++) {


        uint64_t proc_time_before =  before->cores[i].system + before->cores[i].user;
        uint64_t proc_time_after = after->cores[i].system + after->cores[i].user;
        uint64_t delta_proc_time = proc_time_after - proc_time_before;
        double seconds_used = ((double)delta_proc_time) / clock_ticks;

        if (delta_proc_time > 0){
            after->cores[i].core_percent = 100.0 * (seconds_used / ( (double) interval_ms/1000)); 
        } else {
            after->cores[i].core_percent = 0.0;
        }
        //printf("cpu %i -- Before: %lu After: %lu Seconds used: %f interval in ms: %lu \n", i, proc_time_before, proc_time_after, seconds_used, interval_ms);
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
