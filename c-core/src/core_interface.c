#define _GNU_SOURCE   // for prlimit(2)
#include "core_interface.h"
#include <sys/resource.h>  // prlimit, setpriority
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include "general_stat_query.h"
#include "trimmed_info.h"
#include "memory_stats.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

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

// wrapper around your /proc/stat parser:
general_stat get_cpu_stats(void) {
    char *raw = read_general_stat("/proc/stat");
    general_stat gs;
    split_general_stat_string(raw, &gs);
    free(raw);
    return gs;
}

// simple snapshot of every process in /proc
trimmed_info* get_process_list(int *out_count) {
    DIR *dp = opendir("/proc");
    if (!dp) {
        *out_count = 0;
        return NULL;
    }

    int cap = 64, cnt = 0;
    trimmed_info *arr = malloc(cap * sizeof(*arr));
    long page_size = sysconf(_SC_PAGESIZE);
    memory_stats mem; read_memory_stats(&mem);

    struct dirent *ent;
    while ((ent = readdir(dp))) {
        if (!is_number(ent->d_name)) continue;
        char path[64], buf[4096];
        snprintf(path, sizeof(path), "/proc/%s/stat", ent->d_name);
        read_stat(path, buf, sizeof(buf));

        proc_stat ps;
        split_PID_stat_string(buf, &ps);
        struct timeval tv; gettimeofday(&tv, NULL);
        ps.timestamp_ms = tv.tv_sec*1000 + tv.tv_usec/1000;
        ps.ram_percent = (ps.rss * page_size * 100.0f) / (mem.mem_total_kb * 1024.0f);

        trimmed_info ti = convert_to_trimmed_info(&ps);
        if (cnt == cap) {
            cap *= 2;
            arr = realloc(arr, cap * sizeof(*arr));
        }
        arr[cnt++] = ti;
    }
    closedir(dp);
    *out_count = cnt;
    return arr;
}
