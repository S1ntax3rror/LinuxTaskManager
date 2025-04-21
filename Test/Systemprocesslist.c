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
    char comm[512];
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
    unsigned long long starttime;
    unsigned long vsize;
    long rss;
    unsigned long rsslim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned int rt_priority;
    unsigned int policy;
    unsigned long long delayacct_blkio_ticks;
    unsigned long guest_time;
    long cguest_time;
    unsigned long start_data;
    unsigned long end_data;
    unsigned long start_brk;
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long env_start;
    unsigned long env_end;
    int exit_code;
} proc_stat;
    


int is_number(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(*str)) return 0;
    }
    return 1;
}

void print_proc_stat(proc_stat *stat_container) {
    printf("pid: %d\n", stat_container->pid);
    printf("comm: %s\n", stat_container->comm);
    printf("state: %c\n", stat_container->state);
    printf("ppid: %d\n", stat_container->ppid);
    printf("pgrp: %d\n", stat_container->pgrp);
    printf("session: %d\n", stat_container->session);
    printf("tty_nr: %d\n", stat_container->tty_nr);
    printf("tpgid: %d\n", stat_container->tpgid);
    printf("flags: %u\n", stat_container->flags);
    printf("minflt: %lu\n", stat_container->minflt);
    printf("cminflt: %lu\n", stat_container->cminflt);
    printf("majflt: %lu\n", stat_container->majflt);
    printf("cmajflt: %lu\n", stat_container->cmajflt);
    printf("utime: %lu\n", stat_container->utime);
    printf("stime: %lu\n", stat_container->stime);
    printf("cutime: %ld\n", stat_container->cutime);
    printf("cstime: %ld\n", stat_container->cstime);
    printf("priority: %ld\n", stat_container->priority);
    printf("nice: %ld\n", stat_container->nice);
    printf("num_threads: %ld\n", stat_container->num_threads);
    printf("itrealvalue: %ld\n", stat_container->itrealvalue);
    printf("starttime: %llu\n", stat_container->starttime);
    printf("vsize: %lu\n", stat_container->vsize);
    printf("rss: %ld\n", stat_container->rss);
    printf("rsslim: %lu\n", stat_container->rsslim);
    printf("startcode: %lu\n", stat_container->startcode);
    printf("endcode: %lu\n", stat_container->endcode);
    printf("startstack: %lu\n", stat_container->startstack);
    printf("kstkesp: %lu\n", stat_container->kstkesp);
    printf("kstkeip: %lu\n", stat_container->kstkeip);
    printf("signal: %lu\n", stat_container->signal);
    printf("blocked: %lu\n", stat_container->blocked);
    printf("sigignore: %lu\n", stat_container->sigignore);
    printf("sigcatch: %lu\n", stat_container->sigcatch);
    printf("wchan: %lu\n", stat_container->wchan);
    printf("nswap: %lu\n", stat_container->nswap);
    printf("cnswap: %lu\n", stat_container->cnswap);
    printf("exit_signal: %d\n", stat_container->exit_signal);
    printf("processor: %d\n", stat_container->processor);
    printf("rt_priority: %u\n", stat_container->rt_priority);
    printf("policy: %u\n", stat_container->policy);
    printf("delayacct_blkio_ticks: %llu\n", stat_container->delayacct_blkio_ticks);
    printf("guest_time: %lu\n", stat_container->guest_time);
    printf("cguest_time: %ld\n", stat_container->cguest_time);
    printf("start_data: %lu\n", stat_container->start_data);
    printf("end_data: %lu\n", stat_container->end_data);
    printf("start_brk: %lu\n", stat_container->start_brk);
    printf("arg_start: %lu\n", stat_container->arg_start);
    printf("arg_end: %lu\n", stat_container->arg_end);
    printf("env_start: %lu\n", stat_container->env_start);
    printf("env_end: %lu\n", stat_container->env_end);
    printf("exit_code: %d\n", stat_container->exit_code);
}


void set_field_in_proc_stat(proc_stat* stat_container, int index, char* value){
    switch (index)
    {
    case 0: stat_container->pid = atoi(value); break; // int pid;
    case 1: strncpy(stat_container->comm, value, sizeof(stat_container->comm) - 1); printf("%s\n\n", value); break; // char comm[512];
    case 2: stat_container->state = value[0]; break; // char state;
    case 3: stat_container->ppid = atoi(value); break; //int ppid;
    case 4: stat_container->pgrp = atoi(value); break; // int pgrp;
    case 5: stat_container->session = atoi(value); break; // int session;
    case 6: stat_container->tty_nr = atoi(value); break; // int tty_nr;
    case 7: stat_container->tpgid = atoi(value); break; // int tpgid;
    case 8: stat_container->flags = strtoul(value, NULL, 10); break; // unsigned int flags;
    case 9: stat_container->minflt = strtoul(value, NULL, 10); break; // unsigned long minflt;
    case 10: stat_container->cminflt = strtoul(value, NULL, 10); break; // unsigned long cminflt;
    case 11: stat_container->majflt = strtoul(value, NULL, 10); break; // unsigned long majflt;
    case 12: stat_container->cmajflt = strtoul(value, NULL, 10); break;// unsigned long cmajflt;
    case 13: stat_container->utime = strtoul(value, NULL, 10); break; // unsigned long utime;
    case 14: stat_container->stime = strtoul(value, NULL, 10); break; // unsigned long stime;
    case 15: stat_container->cutime = atol(value); break; // long cutime;
    case 16: stat_container->cstime = atol(value); break; // long cstime;
    case 17: stat_container->priority = atol(value); break; // long priority;
    case 18: stat_container->nice = atol(value); break; // long nice;
    case 19: stat_container->num_threads = atol(value); break; // long num_threads;
    case 20: stat_container->itrealvalue = atol(value); break; // long itrealvalue;
    case 21: stat_container->starttime = strtoull(value, NULL, 10); break; // unsigned long long starttime;
    case 22: stat_container->vsize = strtoul(value, NULL, 10); break; // unsigned long vsize;
    case 23: stat_container->rss = atol(value); break; // long rss;
    case 24: stat_container->rsslim = strtoul(value, NULL, 10); break; // unsigned long rsslim;
    case 25: stat_container->startcode = strtoul(value, NULL, 10); break; // unsigned long startcode;
    case 26: stat_container->endcode = strtoul(value, NULL, 10); break; // unsigned long endcode;
    case 27: stat_container->startstack = strtoul(value, NULL, 10); break; // unsigned long startstack;
    case 28: stat_container->kstkesp = strtoul(value, NULL, 10); break; //     unsigned long kstkesp;
    case 29: stat_container->kstkeip = strtoul(value, NULL, 10); break; // unsigned long kstkeip;
    case 30: stat_container->signal = strtoul(value, NULL, 10); break; // unsigned long signal;
    case 31: stat_container->blocked = strtoul(value, NULL, 10); break; // unsigned long blocked;
    case 32: stat_container->sigignore = strtoul(value, NULL, 10); break; // unsigned long sigignore;
    case 33: stat_container->sigcatch = strtoul(value, NULL, 10); break; // unsigned long sigcatch;
    case 34: stat_container->wchan = strtoul(value, NULL, 10); break; // unsigned long wchan;
    case 35: stat_container->nswap = strtoul(value, NULL, 10); break; // unsigned long nswap;
    case 36: stat_container->cnswap = strtoul(value, NULL, 10); break; // unsigned long cnswap;
    case 37: stat_container->exit_signal = atoi(value); break; //     int exit_signal;
    case 38: stat_container->processor = atoi(value); break; // int processor;
    case 39: stat_container->rt_priority = strtoul(value, NULL, 10); break; // unsigned int rt_priority;
    case 40: stat_container->policy = strtoul(value, NULL, 10); break; // unsigned int policy;
    case 41: stat_container->delayacct_blkio_ticks = strtoull(value, NULL, 10); break; // unsigned long long delayacct_blkio_ticks;
    case 42: stat_container->guest_time = strtoul(value, NULL, 10); break; // unsigned long guest_time;
    case 43: stat_container->cguest_time = atol(value); break; // long cguest_time;
    case 44: stat_container->start_data = strtoul(value, NULL, 10); break; // unsigned long start_data;
    case 45: stat_container->end_data = strtoul(value, NULL, 10); break; // unsigned long end_data;
    case 46: stat_container->start_brk = strtoul(value, NULL, 10); break; // unsigned long start_brk;
    case 47: stat_container->arg_start = strtoul(value, NULL, 10); break; // unsigned long arg_start;
    case 48: stat_container->arg_end = strtoul(value, NULL, 10); break; // unsigned long arg_end;
    case 49: stat_container->env_start = strtoul(value, NULL, 10); break; // unsigned long env_start;
    case 50: stat_container->env_end = strtoul(value, NULL, 10); break; // unsigned long env_end;
    case 51: stat_container->exit_code = atoi(value); break; // int exit_code;
    default:
        break;
    }
}

void split_PID_stat_string(char* inp_string, proc_stat* stat_pointer){
    printf("%s", inp_string);
    
    proc_stat storage;
    char space = ' ';
    char bracket_open = '(';
    char bracket_close = ')';
    int inside_bracket = 0;

    char substring[300];
    int substring_index = 0;
    int counter = 0; // keeps track of the word count

    for (int i = 0; inp_string[i] != '\0'; i++) {
        char c = inp_string[i];

        if (inside_bracket){  // skip spaces while in bracket
            if (c == bracket_close) {
                inside_bracket = 0;
            }
        } else {
            if (c == space){ // reset substring
                //printf("SPACE");
                //printf("%s", substring);
                
                set_field_in_proc_stat(stat_pointer, counter, substring);
                substring[0] = '\0';
                substring_index = 0;
                counter += 1;

            } else if (c == bracket_open){ // keep brackets
                inside_bracket = 1;
            }
        }
        substring[substring_index] = c;
        substring[substring_index + 1] = '\0';
        substring_index += 1;

        //printf("%c \n", c);
    }
    //printf("%s", substring);
}

void read_stat(char* path, char* data_ptr) {
    FILE *fp = fopen(path, "r"); // open file at path
    if (fp) {
        if (fgets(data_ptr, sizeof(data_ptr), fp)) { // read filed content and store in data
            // printf("Stats: %s", data_ptr);
        }
        fclose(fp);
    }
}


int main() {
    printf("running Systemproclist\n");

    int* num_str;
    
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
                    //printf("PID: %s\tName: %s", entry->d_name, name);
                }
                fclose(fp);
            }
            char file_data[2048];
            proc_stat stats_container;
            read_stat(stat_path, file_data);
            split_PID_stat_string(file_data, &stats_container);
            print_proc_stat(&stats_container);
        }
    }

    //read_stat("/proc/stat");

    closedir(dp);
    return 0;
}
