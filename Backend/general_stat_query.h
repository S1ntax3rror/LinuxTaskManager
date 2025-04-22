#ifndef GENERAL_STAT_QUERY_H
#define GENERAL_STAT_QUERY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct cpu_stats
{
    char name[512];
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
    uint64_t guest;
    uint64_t guest_nice;
} cpu_stats;


typedef struct general_stat { // create struct for storing process data
    cpu_stats cpu;
    cpu_stats cores[100];
    uint64_t iter_0;
    uint64_t ctxt;
    uint64_t btime;
    uint64_t processes;
    uint64_t procs_running;
    uint64_t procs_blocked;
    // add more if needed
} general_stat;
    

void print_general_stat(general_stat *stat_container);
void set_field_in_general_stat(general_stat* stat_container, int index, char* value);
void split_general_stat_string(char* inp_string, general_stat* stat_pointer);
char* read_general_stat(const char* path);

#endif // GENERAL_STAT_QUERY_H