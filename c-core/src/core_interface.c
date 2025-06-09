#include <stdint.h>
#define _GNU_SOURCE   // for prlimit(2)
#include "../include/core_interface.h"
#include <sys/resource.h>  // prlimit, setpriority
#include <sys/types.h>
#include <signal.h>
#include "../include/general_stat_query.h"
#include "../include/trimmed_info.h"
#include "../include/memory_stats.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include "../include/proc_stat.h"
#include "../include/utils.h"
#include "../include/cpu_usage.h"

#define MAX_PROCESS 2048
int DOWN_TIME = 200000;
struct timeval now;
uint64_t now_time, before_time;

proc_stat before_list[MAX_PROCESS]; 
proc_stat now_list[MAX_PROCESS]; 
int proc_count = 0;
int counter = 0;
static general_stat before_gs = {0};

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
    general_stat now_gs = {0};

    //uint64_t time_before_split = (now.tv_usec / 1000);
    
    split_general_stat_string(raw, &now_gs);
    free(raw);

    /*gettimeofday(&now, NULL);
    uint64_t time_after_split = (now.tv_usec / 1000);
    uint64_t time_diff = time_before_split - time_after_split;
    printf("time taken for splitting general stat: %lu", time_diff);
    */

    gettimeofday(&now, NULL);
    now_gs.timestamp_ms = (uint64_t)(now.tv_sec) * 1000 + (now.tv_usec / 1000);
    
    if (before_gs.timestamp_ms > 0) {
        uint64_t delta_time = now_gs.timestamp_ms - before_gs.timestamp_ms;
        calculate_normalized_core_cpu_usage(&before_gs, &now_gs, delta_time);
        calculate_normalized_general_cpu_usage(&before_gs, &now_gs, delta_time);
    }
    before_gs = now_gs;
    return now_gs;
}

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
    counter++;
    printf("counter: %i\n", counter);
    struct dirent *entry;
    
    while ((entry = readdir(dp))) {
            if (!is_number(entry->d_name)) continue;

            int pid = atoi(entry->d_name);
            char stat_path[512];
            snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

            char file_data[3000];
            FILE* fp = fopen(stat_path, "r");
            if (!fp) continue;
            fclose(fp);

            read_stat(stat_path, file_data, sizeof(file_data));

            proc_stat snapshot = {0}; //Set all fields to 0
            split_PID_stat_string(file_data, &snapshot);
            gettimeofday(&now, NULL);
            snapshot.timestamp_ms = (uint64_t)(now.tv_sec) * 1000 + (now.tv_usec / 1000);
            snapshot.ram_percent = (snapshot.rss * page_size * 100.0) / (mem.mem_total_kb * 1024.0);
            int proc_index = -1;

            for (int i = 0; i < MAX_PROCESS; i++) {
                if (before_list[i].pid == pid) {
                    now_list[i] = snapshot;
                    uint64_t down_time;
                    if (before_list[i].timestamp_ms > 0) {
                        down_time = now_list[i].timestamp_ms - before_list[i].timestamp_ms;
                    } else {
                        down_time = 200;
                    }
                    calculate_normalized_cpu_usage(&before_list[i], &now_list[i], down_time);
                    proc_index = i;
                    break;
                }
            }

            if (proc_index == -1 && proc_count < MAX_PROCESS) {
                before_list[proc_count] = snapshot;
                now_list[proc_count] = snapshot;
                now_list[proc_count].cpu_percent = 0.0;
                proc_index = proc_count;
                proc_count++;
            }

            if (proc_index != -1) {
                trimmed_info ti = convert_to_trimmed_info(&now_list[proc_index]);
                if( cnt == cap ) {
                    cap *= 2;
                    arr =realloc(arr,cap *sizeof(*arr));
                }
                arr[cnt++] = ti;
            }
        }
        
        memset(before_list, 0, sizeof(before_list));
        
        for (int i=0; i<proc_count; i++) {
            before_list[i] = now_list[i];
        }
        
        closedir(dp);
        *out_count = cnt;
        return arr;
}
