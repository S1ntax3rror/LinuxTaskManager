#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <ctype.h>
#include <string.h>

int is_number(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(*str)) return 0;
    }
    return 1;
}

void read_stat(char* path) {
    FILE *fp = fopen(path, "r"); // open file at path
    if (fp) {
        char stats[1024];
        if (fgets(stats, sizeof(stats), fp)) { // read filed content and store in stats
            printf("Stats: %s", stats);
        }
        fclose(fp);
    }
}

int main() {
    struct dirent *entry; // create struct for directory entries
    printf("running Systemproclist\n");

    
    DIR *dp = opendir("/proc"); // open proc directory
    if (dp == NULL) {           // make sure it is opened correctly
        printf("/proc open error");
        return 1;
    }
    
    // todo read stat file

    
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
                    printf("PID: %s\tName: %s", entry->d_name, name);
                }
                fclose(fp);
            }
            read_stat(stat_path);
        }
    }

    read_stat("/proc/stat");

    closedir(dp);
    return 0;
}
