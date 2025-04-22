#ifndef PROC_STAT_H
#define PROC_STAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
    

void print_proc_stat(proc_stat *stat_container);
void set_field_in_proc_stat(proc_stat* stat_container, int index, char* value);
void split_PID_stat_string(char* inp_string, proc_stat* stat_pointer);
void read_stat(char* path, char* data_ptr, int size);

#endif // PROC_STAT_H