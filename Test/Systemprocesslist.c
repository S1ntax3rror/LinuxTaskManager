#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <ctype.h>
#include <string.h>


struct dirent *entry; // create struct for directory entries

typedef struct proc_stat { // create struct for storing process data
    int pid;
    char comm[256];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned int flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long starttime;
    unsigned long vsize;
    long rss;
} proc_stat;


int is_number(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(*str)) return 0;
    }
    return 1;
}

char **split_PID_stat_string(char* inp_string, int* num_strings){
    printf("%s", inp_string);
    
    proc_stat storage;
    char space = ' ';
    char bracket_open = '(';
    char bracket_close = ')';
    int inside_bracket = 0;

    char substring[300];
    int substring_index = 0;

    for (int i = 0; inp_string[i] != '\0'; i++) {
        char c = inp_string[i];

        if (inside_bracket){  // skip spaces while in bracket
            if (c == bracket_close) {
                inside_bracket = 0;
            }
        } else {
            if (c == space){ // reset substring
                printf("SPACE");
                printf("%s", substring);
                substring[0] = '\0';
                substring_index = 0;
            } else if (c == bracket_open){
                inside_bracket = 1;
            }
        }
        substring[substring_index] = c;
        substring[substring_index + 1] = '\0';
        substring_index += 1;

        printf("%c \n", c);
    }
    printf("%s", substring);
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
    printf("running Systemproclist\n");

    int* num_str;
    split_PID_stat_string("10350 (Isolated Web Co) S 3159 2417 2417 0 -1 4194560 9091 0 0 0 98 57 0 0 20 0 27 0 12842354 2505527296 26227 18446744073709551615 109946429318016 109946430173008 140726263227568 0 0 0 0 69634 1082134264 0 0 0 17 1 0 0 0 0 0 109946430188120 109946430188240 109946756247552 140726263234789 140726263235299 140726263235299 140726263242701 0\n", num_str);


    return 1;
    
    DIR *dp = opendir("/proc"); // open proc directory
    if (dp == NULL) {           // make sure it is opened correctly
        printf("/proc open error");
        return 1;
    }
    
    
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
