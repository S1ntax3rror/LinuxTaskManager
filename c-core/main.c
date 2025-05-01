#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

#include "proc_stat.h"
#include "utils.h"
#include "general_stat_query.h"
#include "cpu_usage.h"
#include "memory_stats.h"
#include "proc_entry.h"
#include "timeline.h"
#include "trimmed_info.h"

#define MAX_PROCS 2048
#define DOWN_TIME 2000000  //microseconds // 200000 for 200ms (to test i did 2000000 for 2s)


struct dirent *entry; // create struct for directory entries


int main() {
    printf("running Systemproclist\n");
    proc_timeline* history = calloc(MAX_PROCS, sizeof(proc_timeline));
    if (!history) {
        perror("calloc failed");
        return 1;
    }
    for (int i = 0; i < MAX_PROCS; i++) {
        history[i].pid = -1;
        history[i].latest_index = 0;
    }

    int* num_str;
    
    DIR *dp = opendir("/proc"); // open proc directory
    if (dp == NULL) {           // make sure it is opened correctly
        printf("/proc open error");
        return 1;
    }
    memory_stats meminfo;
    read_memory_stats(&meminfo);
    long page_size = sysconf(_SC_PAGESIZE);

    printf("\nSystem Memory Stats:\n");
    printf("Total: %.2f MB\n", meminfo.mem_total_kb / 1024.0);
    printf("Free: %.2f MB\n", meminfo.mem_free_kb / 1024.0);
    printf("Available: %.2f MB\n", meminfo.mem_available_kb / 1024.0);
    printf("Buffers: %.2f MB\n", meminfo.buffers_kb / 1024.0);
    printf("Cached: %.2f MB\n\n", meminfo.cached_kb / 1024.0);
    
    char* stat_data = read_general_stat("/proc/stat");
    //printf("%s\n", stat_data);
    general_stat general_stat_container;
    split_general_stat_string(stat_data, &general_stat_container);
    print_general_stat(&general_stat_container);
    //return 0; // TODO remove before merge

    int num_folders = count_folders("/proc"); // INITIALIZE LIST WITH ENOUGH SPACE FOR ALL PROCESS STATS
    printf("%i folders in /proc. Allocating space for %i potential stat lists. \n", num_folders, num_folders);
    proc_stat process_statistics_array[num_folders];    
    int current_list_entry_index = 0;
    
    
    proc_entry processes[MAX_PROCS];
    int proc_count = 0;
    
    // init of before_info
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
            
            double ram_usage_percent = (snapshot.rss * page_size * 100.0) / (meminfo.mem_total_kb * 1024.0);

            snapshot.ram_percent = ram_usage_percent;

            processes[proc_count].before_info = snapshot;
            processes[proc_count].pid = snapshot.pid;
            processes[proc_count].valid = 1;
            proc_count++;

            if (proc_count >= MAX_PROCS) {
                printf("to many processes\n");
                break;

            }
        }
    }
    closedir(dp);

    while(1){

    
            usleep(DOWN_TIME);  //Time Interval 

            read_memory_stats(&meminfo);

            
            
        
            //reopen proc
        DIR* dp2 = opendir("/proc");
        if (!dp2) {
            perror("reopen /proc");
            return 1;
        }

        while ((entry = readdir(dp2))) {
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

            snapshot.ram_percent =
                (snapshot.rss * page_size * 100.0) / (meminfo.mem_total_kb * 1024.0);

            // 🔍 Check if this PID existed in before_info an valid
            int proc_index = -1;

            for (int i = 0; i < proc_count; i++) {
                if (processes[i].pid == pid) {
                    processes[i].now_info = snapshot;
                    calculate_normalized_cpu_usage(&processes[i].before_info, &processes[i].now_info, (double)DOWN_TIME / 1000000);
                    processes[i].valid = 1;
                    processes[i].before_info = processes[i].now_info;
                    proc_index = i;
                    break;
                }
            }

            if (proc_index == -1 && proc_count < MAX_PROCS) {
                processes[proc_count].pid = pid;
                processes[proc_count].now_info = snapshot;
                processes[proc_count].before_info = snapshot;
                processes[proc_count].now_info.cpu_percent = 0.0;
                processes[proc_count].valid = 1;
                proc_index = proc_count;
                proc_count++;
            }

            if (proc_index != -1) {
                trimmed_info t = convert_to_trimmed_info(&processes[proc_index].now_info);
                history[proc_index].pid = t.pid;
                history[proc_index].timeline[history[proc_index].latest_index] = t;
                history[proc_index].latest_index = (history[proc_index].latest_index + 1) % MAX_TIMELINE;
            }
        }
        closedir(dp2);

        
        printf("\033[H\033[J");  // Clear screen
        printf("Live Process Monitor (refresh: %.1f sec)\n\n", (double)DOWN_TIME / 1000000.0);
        
        for (int i = 0; i < proc_count; i++) {
            if (!processes[i].valid) continue;
        
            int pid = processes[i].pid;
            printf("PID %d - %s\n", pid, processes[i].now_info.comm);
            printf("Time(ms)\tCPU %%\tRAM %%\n");
        
            int count = 0;
            int index = history[i].latest_index;
        
            // Print the last 5 entries (from newest to oldest)
            for (int j = 0; j < 5; j++) {
                int real_index = (index - 1 - j + MAX_TIMELINE) % MAX_TIMELINE;
                trimmed_info* t = &history[i].timeline[real_index];
        
                if (t->pid == -1) continue;  // skip uninitialized


        
                printf("%llu\t%.2f\t%.2f\n",
                       (unsigned long long)t->timestamp_ms,
                       t->cpu_percent,
                       t->ram_percent);
        
                count++;
            }
        
            if (count == 0) {
                printf("  No recent history.\n");
            }
        
            printf("\n");
        }

    }
    

return 0;
}
