#ifndef GENERAL_STAT_QUERY_H
#define GENERAL_STAT_QUERY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "memory_stats.h"


typedef struct cpu_stats
{
    char name[512];
    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
    uint64_t guest;
    uint64_t guest_nice;
    double core_percent;
} cpu_stats;

typedef struct disk_stats {
    uint64_t read_sectors;
    uint64_t write_sectors;
    float read_MB;
    float write_MB;
} disk_stats;

typedef struct {
    float total_download_MB;
    float total_upload_MB;
    //int max_speed_mbps; maybe later
} network_stats;

typedef struct gpu_stats{
    bool nvidia_gpu;
    float gpu_MB;
    float gpu_util_percent;
}gpu_stats;


typedef struct general_stat { // create struct for storing process data
    cpu_stats cpu;
    cpu_stats cores[100];
    uint64_t intr_0;
    uint64_t ctxt;
    uint64_t btime;
    uint64_t processes;
    uint64_t procs_running;
    uint64_t procs_blocked;
    int num_cpus;
    float total_cpu_time;
    uint64_t cpu_nonproductive_time;
    float total_cpu_utilization_percent;
    float total_cpu_user;
    float total_cpu_system;
    float total_cpu_wait;
    float total_cpu_idle;
    float total_cpu_steal;
    float total_gpu;
    float total_disk_write_MB;
    float total_disk_read_MB;
    float avg_disk_read_MB;
    float avg_disk_write_MB; // last 100 frames
    float network_upload_speed;
    float network_download_speed;
    float network_avg_upload_speed;
    float network_avg_download_speed;
    memory_stats memory;
    disk_stats disk;
    network_stats net;
    gpu_stats gpu;
    uint64_t timestamp_ms;
} general_stat;
    

void print_general_stat(general_stat *stat_container);
void set_field_in_general_stat(general_stat* stat_container, int index, char* value);
void split_general_stat_string(char* inp_string, general_stat* stat_pointer);
char* read_general_stat(const char* path);
void read_disk_stats(disk_stats* disk);
void read_network_stats(network_stats* net);
void read_gpu_stats(gpu_stats* gpu);
int has_nvidia_gpu();

#endif // GENERAL_STAT_QUERY_H
