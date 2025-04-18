#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <ctype.h>

int is_number(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(*str)) return 0;
    }
    return 1;
}

int main() {
    struct dirent *entry; // create struct for directory entries
    printf("running Systemproclist\n");

    
    DIR *dp = opendir("/proc"); // open proc directory
    if (dp == NULL) {           // make sure it is opened correctly
        printf("/proc open error");
        return 1;
    }
    
    
    while ((entry = readdir(dp))) { // loop through all entries in the directory
        if (is_number(entry->d_name)) { // check if enty is a number --> its a process
            char path[512];
            snprintf(path, sizeof(path), "/proc/%s/comm",entry->d_name); // store /proc/PID/comm to path

            FILE *fp = fopen(path, "r"); // open file at path
            if (fp) {
                char name[256];
                if (fgets(name, sizeof(name), fp)) { // 
                    printf(name);
                    printf("PID: %s\tName: %s", entry->d_name, name);
                }
                fclose(fp);
            }

        }
    }

    closedir(dp);
    return 0;
}
