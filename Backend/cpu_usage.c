
#include "cpu_usage.h"
#include <unistd.h>

void calculate_normalized_cpu_usage(proc_stat* before, proc_stat* after, double interval_seconds) {
    long clock_ticks = sysconf(_SC_CLK_TCK);
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    unsigned long proc_time_before = before->utime + before->stime;
    unsigned long proc_time_after = after->utime + after->stime;
    unsigned long delta_proc_time = proc_time_after - proc_time_before;

    double seconds_used = (double)delta_proc_time / clock_ticks;
    after->cpu_percent = 100.0 * (seconds_used / (interval_seconds * num_cores));
}
