#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "proc_stat.h"
#include "utils.h"


struct dirent *entry; // create struct for directory entries


int main() {
    printf("running Systemproclist\n");

    int* num_str;
    
    DIR *dp = opendir("/proc"); // open proc directory
    if (dp == NULL) {           // make sure it is opened correctly
        printf("/proc open error");
        return 1;
    }
    int num_folders = count_folders("/proc"); // INITIALIZE LIST WITH ENOUGH SPACE FOR ALL PROCESS STATS
    printf("%i folders in /proc. Allocating space for %i potential stat lists. \n", num_folders, num_folders);
    proc_stat process_statistics_array[num_folders];

    int current_list_entry_index = 0;
    
    while ((entry = readdir(dp))) { // loop through all entries in the directory
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

    
    //read_stat("/proc/stat"); // TODO read general system stats

    closedir(dp);
    return 0;
}
