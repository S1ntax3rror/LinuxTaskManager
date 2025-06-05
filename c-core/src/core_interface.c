#define _GNU_SOURCE   // for prlimit(2)
#include "../include/core_interface.h"
#include <sys/resource.h>  // prlimit, setpriority
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include "../include/general_stat_query.h"
#include "../include/trimmed_info.h"
#include "../include/memory_stats.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include "../include/proc_stat.h"

#define MAX_PROCESS 2048
int DOWN_TIME = 200000;

proc_stat before_list[MAX_PROCESS]; 
proc_stat now_list[MAX_PROCESS]; 
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
    int proc_count = 0;
    int cap = 64, cnt = 0;
    trimmed_info *arr = malloc(cap * sizeof(*arr));
    long page_size = sysconf(_SC_PAGESIZE);
    memory_stats mem; read_memory_stats(&mem);

    struct dirent *entry;
    if (!before_list[1].pid){

    
        while ((entry = readdir(dp))) {
            if (is_number(entry->d_name)) {
                char stat_path[512];
                snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);

                char file_data[3000];
                read_stat(stat_path, file_data, sizeof(file_data));

                proc_stat snapshot;
                split_PID_stat_string(file_data, &snapshot);
                struct timeval now;
                gettimeofday(&now, NULL);
                snapshot.timestamp_ms = (uint64_t)(now.tv_sec) * 1000 + (now.tv_usec / 1000);
                
                double ram_usage_percent = (snapshot.rss * page_size * 100.0) / (mem.mem_total_kb * 1024.0);

                snapshot.ram_percent = ram_usage_percent;

                before_list[proc_count] = snapshot;
                
                
                proc_count++;

                if (proc_count >= MAX_PROCESS) {
                    printf("to many processes\n");
                    break;
                }
                
            }
        }
    }
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

            proc_stat snapshot;
            split_PID_stat_string(file_data, &snapshot);
            struct timeval now;
            gettimeofday(&now, NULL);
            snapshot.timestamp_ms = (uint64_t)(now.tv_sec) * 1000 + (now.tv_usec / 1000);

            snapshot.ram_percent = (snapshot.rss * page_size * 100.0) / (mem.mem_total_kb * 1024.0);

            int proc_index = -1;

            for (int i = 0; i < proc_count; i++) {
                if (before_list[i].pid == pid) {
                    now_list[i] = snapshot;
                    calculate_normalized_cpu_usage(&before_list[i], &now_list[i], (double)DOWN_TIME / 1000000);
                    before_list[i] = now_list[i];
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
        

        
        closedir(dp);

        *out_count = cnt;
        return arr;
}
