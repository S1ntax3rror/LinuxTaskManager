#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#include "proc_stat.h"
#include "utils.h"
#include "general_stat_query.h"
#include "cpu_usage.h"
#include "memory_stats.h"

#define MAX_PROCS 2048
struct dirent *entry; // create struct for directory entries


int main() {
    printf("running Systemproclist\n");

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
    
/*    while ((entry = readdir(dp))) { // loop through all entries in the directory
        if (is_number(entry->d_name)) { // check if enty is a number --> its a process

            char path[512];
            char stat_path[512];
            snprintf(path, sizeof(path), "/proc/%s/comm",entry->d_name); // store /proc/PID/comm to path
            snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat",entry->d_name); // store /proc/PID/stat to stat_path

            FILE *fp = fopen(path, "r"); // open file at path
            if (fp) {
                char name[256];
                if (fgets(name, sizeof(name), fp)) { // read file content and store in name
                    // printf("PID: %s\tName: %s \n", entry->d_name, name);
                }
                fclose(fp);
            }
            proc_stat stats_container;
            char file_data[3000]; // large number picked. To be fixed later
            read_stat(stat_path, file_data, 3000);
            split_PID_stat_string(file_data, &stats_container);
            
            process_statistics_array[current_list_entry_index] = stats_container;

            // print_proc_stat(&stats_container);

            current_list_entry_index++;
        }
    }

    print_proc_stat(&process_statistics_array[current_list_entry_index-10]);

    closedir(dp);
*/    

    
    proc_stat before_stats[MAX_PROCS];
    proc_stat after_stats[MAX_PROCS];
    int pids[MAX_PROCS];
    int proc_count = 0;
    
    
    while ((entry = readdir(dp))) {
        if (is_number(entry->d_name)) {
            char stat_path[512];
            snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);

            char file_data[3000];
            read_stat(stat_path, file_data, sizeof(file_data));

            proc_stat snapshot;
            split_PID_stat_string(file_data, &snapshot);
            
            double ram_usage_percent = (snapshot.rss * page_size * 100.0) / (meminfo.mem_total_kb * 1024.0);

            snapshot.ram_percent = ram_usage_percent;

            before_stats[proc_count] = snapshot;
            pids[proc_count] = atoi(entry->d_name);
            proc_count++;

            if (proc_count >= MAX_PROCS) break;
        }
    }
    closedir(dp);

    sleep(1);  // ---------- Time Interval (1s) ----------

    // ---------- Second Snapshot + CPU Calculation ----------
    for (int i = 0; i < proc_count; i++) {
        char stat_path[512];
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pids[i]);

        char file_data[3000];
        FILE *fp = fopen(stat_path, "r");
        if (!fp) continue; // process may have exited
        fclose(fp);

        read_stat(stat_path, file_data, sizeof(file_data));
        split_PID_stat_string(file_data, &after_stats[i]);

        calculate_normalized_cpu_usage(&before_stats[i], &after_stats[i], 1.0);

        if (after_stats[i].cpu_percent > 0.1) {  // skip idle processes
            printf("PID: %d\tCPU: %.2f%%\tName: %s\n",
                   after_stats[i].pid,
                   after_stats[i].cpu_percent,
                   after_stats[i].comm);
        }
        if (before_stats[i].ram_percent > 0.5) {  // skip low ram usage
            printf("PID: %d\tRAM: %.2f%%\tName: %s\n",
                   before_stats[i].pid,
                   before_stats[i].ram_percent,
                   before_stats[i].comm);
        }
    }

return 0;
}
