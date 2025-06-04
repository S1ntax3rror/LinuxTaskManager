#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

#include "../include/proc_stat.h"
#include "../include/utils.h"
#include "../include/general_stat_query.h"
#include "../include/cpu_usage.h"
#include "../include/memory_stats.h"
#include "../include/proc_entry.h"
#include "../include/trimmed_info.h"
#include "../include/process_sort.h"
#include "../include/header.h"

#define MAX_PROCS 4096
#define DOWN_TIME 2000000  //microseconds // 200000 for 200ms (to test i did 2000000 for 2s)


struct dirent *entry; // create struct for directory entries


int main() {
    printf("running Systemproclist\n");
    trimmed_info sorted_list[MAX_PROCS];
    SortMode current_sort = SORT_BY_PID;            
    int visible_count = 0; 

    header header_stats;
    init_header(&header_stats);

    proc_timeline* history = calloc(MAX_PROCS, sizeof(proc_timeline));
    if (!history) {
        perror("calloc failed");
        return 1;
    }
    for (int i = 0; i < MAX_PROCS; i++) {
        history[i].pid = -1;
        history[i].latest_index = 0;
    }

    //int* num_str;
    
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
    printf("Cached: %.2f MB\n", meminfo.cached_kb / 1024.0);
    printf("SwapTotal: %.2f MB\n", meminfo.swap_total_kb / 1024.0);
    printf("SwapFree: %.2f MB\n", meminfo.swap_free_kb / 1024.0);
    printf("SwapUsed: %.2f MB\n\n", meminfo.swap_used_kb / 1024.0);
    char* stat_data = read_general_stat("/proc/stat");
    general_stat general_stat_container;
    split_general_stat_string(stat_data, &general_stat_container);
    print_general_stat(&general_stat_container);

    printf("\nDisk Read: %.2f MB\n", general_stat_container.disk.read_MB);
    printf("Disk Write: %.2f MB\n", general_stat_container.disk.write_MB);

    printf("Download: %.2f MB\n", general_stat_container.net.total_download_MB);
    printf("Upload:   %.2f MB\n", general_stat_container.net.total_upload_MB);

    if (general_stat_container.gpu.nvidia_gpu) {
        printf("GPU Memory Used: %.2f MB\n", general_stat_container.gpu.gpu_MB);
        printf("GPU Utilization: %.2f%%\n", general_stat_container.gpu.gpu_util_percent);
    } else {
        printf("No NVIDIA GPU detected.\n");
    }
    
    int num_folders = count_folders("/proc"); // INITIALIZE LIST WITH ENOUGH SPACE FOR ALL PROCESS STATS
    printf("%i folders in /proc. Allocating space for %i potential stat lists. \n", num_folders, num_folders);
    //proc_stat process_statistics_array[num_folders];    
    //int current_list_entry_index = 0;
    
    proc_entry* processes = calloc(MAX_PROCS, sizeof(proc_entry));
    int proc_count = 0;
    exit(0);
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
            printf("PID %i\n", processes[proc_count-1].pid);
        }
    }
    printf("PID %i\n", processes[proc_count-1].pid);

    closedir(dp);
    
    //exit(0); //init_testing
    printf("starting loop\n");
    //int loopcount = 0;//for testing
    while(1){

    
            usleep(DOWN_TIME);  //Time Interval 
            
            read_memory_stats(&meminfo);
            char* stat_data = read_general_stat("/proc/stat");
            general_stat general_stat_container;
            split_general_stat_string(stat_data, &general_stat_container);
            push_general_stat(&header_stats, &general_stat_container, DOWN_TIME / 1000000.0);
            free(stat_data);
            
        
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

            int proc_index = -1;
            
            exit(0);
            printf("proc %i", processes[1].pid);


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

         // Step 1: Flatten the latest snapshot into sorted_list
        visible_count = 0;
        for (int i = 0; i < proc_count; i++) {
            int latest = (history[i].latest_index - 1 + MAX_TIMELINE) % MAX_TIMELINE;
            trimmed_info* t = &history[i].timeline[latest];
            if (t->pid == -1) continue;
 
            sorted_list[visible_count] = *t;
            sorted_list[visible_count].avg_cpu_percent = calculate_avg_cpu_percent(&history[i]);
            visible_count++;
        }
 
        current_sort = read_sort_mode_from_file("sort_mode.txt");
        switch (current_sort) {
            case SORT_BY_CPU:     sort_by_cpu(sorted_list, visible_count); break;
            case SORT_BY_RAM:     sort_by_ram(sorted_list, visible_count); break;
            case SORT_BY_NAME:    sort_by_name(sorted_list, visible_count); break;
            case SORT_BY_STATE:   sort_by_state(sorted_list, visible_count); break;
            case SORT_BY_PID:     sort_by_pid(sorted_list, visible_count); break;
            case SORT_BY_PID_A:   sort_by_pid_a(sorted_list, visible_count); break;
            case SORT_BY_AVG_CPU: sort_by_avg_cpu(sorted_list, visible_count); break;
        }
        

        //int last_index = (header_stats.latest_index - 1 + MAX_HEADER_ENTRIES) % MAX_HEADER_ENTRIES;
        //general_stat* latest = &header_stats.entries[last_index];

        // printf("\n--- Latest General Stat ---\n");
        // printf("CPU%%: %.2f\n", latest->total_cpu_utilization_percent);
        // printf("RAM Available: %.2f MB\n", latest->memory.mem_available_kb / 1024.0);
        // printf("Disk Read: %.2f MB\n", latest->avg_disk_read_MB);
        // printf("Download: %.2f MB\n", latest->net.total_download_MB);

        
        // printf("Download Speed: %.2f MB/s\n", latest->network_avg_download_speed);
        // printf("Upload Speed: %.2f MB/s\n", latest->network_avg_upload_speed);
        


        printf("\033[H\033[J");  // Clear screen
        printf("Live Process Monitor (refresh: %.1f sec)\n\n", (double)DOWN_TIME / 1000000.0);
        printf("Time\t\tCPU%% \tA_CPU%%\tRAM %%\tPID\tName\n");
        //printf("%s\n", sorted_list[1].time_str);
        int to_display = visible_count > 10 ? 10 : visible_count;
        for (int i = 0; i < to_display; i++) {
            trimmed_info* t = &sorted_list[i];
            printf("%s\t%.2f\t%.2f\t%.2f\t%d\t%s\n",
                   t->time_str,
                   t->cpu_percent,
                   t->avg_cpu_percent,
                   t->ram_percent,
                   t->pid,
                   t->comm);
        }
       

    }
    

return 0;
}
