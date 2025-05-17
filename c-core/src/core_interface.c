#define _GNU_SOURCE   // for prlimit(2)
#include "core_interface.h"
#include <sys/resource.h>  // prlimit, setpriority
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

/**
 * send_signal: wrapper around kill(2).
 */
int send_signal(int pid, int signo) {
    return kill(pid, signo);
}

int renice_process(int pid, int nice_val) {
    // PRIO_PROCESS → change “nice” for a process
    if (setpriority(PRIO_PROCESS, pid, nice_val) == 0)
        return 0;
    else
        return -1;
}

int set_cpu_limit(int pid, rlim_t seconds) {
    struct rlimit rl = { .rlim_cur = seconds, .rlim_max = seconds };
    if (prlimit(pid, RLIMIT_CPU, &rl, NULL) == 0)
        return 0;
    else
        return -1;
}

int set_ram_limit(int pid, rlim_t bytes) {
    struct rlimit rl = { .rlim_cur = bytes,  .rlim_max = bytes };
    if (prlimit(pid, RLIMIT_AS, &rl, NULL) == 0)
        return 0;
    else
        return -1;
}
