#include "../include/general_stat_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_cpu_stats(cpu_stats* cpu_container) {
    printf("\nname: %s\n\n", cpu_container->name);
    printf("user: %ld\n", cpu_container->user);
    printf("nice: %ld\n", cpu_container->nice);
    printf("system: %ld\n", cpu_container->system);
    printf("idle: %ld\n", cpu_container->idle);
    printf("iowait: %ld\n", cpu_container->iowait);
    printf("irq: %ld\n", cpu_container->irq);
    printf("steal: %ld\n", cpu_container->steal);
    printf("guest: %ld\n", cpu_container->guest);
    printf("guest_nice: %ld\n", cpu_container->guest_nice);
}


void print_cpu_stats_one_line(cpu_stats* cpu_container) {
    printf("name: %s    ", cpu_container->name);
    printf("user: %ld    ", cpu_container->user);
    printf("nice: %ld    ", cpu_container->nice);
    printf("system: %ld    ", cpu_container->system);
    printf("idle: %ld    ", cpu_container->idle);
    printf("iowait: %ld    ", cpu_container->iowait);
    printf("irq: %ld    ", cpu_container->irq);
    printf("steal: %ld    ", cpu_container->steal);
    printf("guest: %ld    ", cpu_container->guest);
    printf("guest_nice: %ld\n", cpu_container->guest_nice);
}


void print_general_stat(general_stat *stat_container){
    printf("General CPU stats:\n");
    print_cpu_stats(&stat_container->cpu);

    printf("\nCore specific CPU stats: \n\n");
    for (int i = 0; i < stat_container->num_cpus; i++) {
        print_cpu_stats_one_line(&stat_container->cores[i]);
    }

    printf("Interrupts: %li\n", stat_container->intr_0);
    printf("Context Switches: %li\n", stat_container->ctxt);
    printf("Boot Time: %li\n", stat_container->btime);
    printf("Processes: %li\n", stat_container->processes);
    printf("Running Processes: %li\n", stat_container->procs_running);
    printf("Blocked Processes: %li\n", stat_container->procs_blocked);
    printf("Number of CPUs: %i\n", stat_container->num_cpus);
};


char* get_seccond_arg(char* string_inp, int first_word_offset){
    int jj = first_word_offset;
    int ii = 0;
    char* num_str = (char*) malloc(200);
    while (string_inp[jj] != ' '){
        num_str[ii] = string_inp[jj];
        jj++;
        ii++;
    }
    num_str[ii+1] = '\0';

    return num_str;
}


void set_cpu_field(cpu_stats* cpu_container, int index, char* value){
    switch (index)
    {
    case 0: strncpy(cpu_container->name, value, sizeof(cpu_container->name) - 1); break;
    case 1: cpu_container->user = atoi(value); break;
    case 2: cpu_container->nice = atoi(value); break;
    case 3: cpu_container->system = atoi(value); break;
    case 4: cpu_container->idle = atoi(value); break;
    case 5: cpu_container->iowait = atoi(value); break;
    case 6: cpu_container->irq = atoi(value); break;
    case 7: cpu_container->steal = atoi(value); break;
    case 8: cpu_container->guest = atoi(value); break;
    case 9: cpu_container->guest_nice = atoi(value); break;
    default: break;
    }
}


void calc_cpu_stats(general_stat* g_stat_pointer){
    int num_cpu = g_stat_pointer->num_cpus;
    uint64_t total_nonproductive_time = 0;
    uint64_t total_cpu_time = 0;
    for (int i=0;i<num_cpu;i++){
        cpu_stats core_stat = g_stat_pointer->cores[i];
        //print_cpu_stats(&core_stat);
        total_nonproductive_time = total_nonproductive_time + core_stat.iowait + core_stat.idle;
        total_cpu_time += core_stat.user + core_stat.nice + core_stat.system + core_stat.idle + core_stat.iowait + core_stat.irq + core_stat.steal;
    }
    //float cpu_percent = total_nonproductive_time/total_cpu_time;
    //printf("total time nonprod: %li, total prod time: %li, total cpu percent: %f \n\n", total_nonproductive_time, total_cpu_time, cpu_percent);
    if (total_cpu_time > 0) {
        g_stat_pointer->total_cpu_utilization_percent =
            100.0 * (total_cpu_time - total_nonproductive_time) / total_cpu_time;
    } else {
        g_stat_pointer->total_cpu_utilization_percent = 0.0;
    }
}


void set_cpu(char* inp_string, cpu_stats* stat_pointer){
    char space = ' ';
    int counter = 0;
    char substring[100];
    int substring_index = 0;

    for (int i = 0; inp_string[i] != '\0'; i++) {
        char c = inp_string[i];
        
        if (c == space){ // reset substring
            while (inp_string[i+1] == space){ // skip spaces
                i++;
            }
            set_cpu_field(stat_pointer, counter, substring);
            //printf("counter state: %i ::: substring ::: %s \n\n", counter, substring);
            substring[0] = '\0';
            counter += 1;
            substring_index = 0;
        }
        substring[substring_index] = c;
        substring[substring_index + 1] = '\0';
        substring_index += 1;
    }
    set_cpu_field(stat_pointer, counter, substring); // set final field
}

void split_general_stat_string(char* inp_string, general_stat* stat_pointer){
    char line[10000];
    int64_t offset = 0;
    int cpu_count = 0;

    for (int i = 0; inp_string[i] != '\0'; i++) {
        if (inp_string[i] == '\n'){
            line[i-offset] = '\0';
            offset = i+1;
            if (sizeof(line) > 4){
                char firstThree[4]; // get first 3 chars of string (to check if its a cpu stat)
                strncpy(firstThree, line, 3);
                firstThree[3] = '\0';

                int w_count = 0; while (line[w_count] != ' '){w_count++;} // get size of first word
                char first_word[w_count+1];
                strncpy(first_word, line, w_count);
                first_word[w_count] = '\0';

                if (strcmp(firstThree, "cpu") == 0 || strcmp(firstThree, "cpu ") == 0){
                    cpu_stats cpu_stats_container;
                    set_cpu(line, &cpu_stats_container);

                    if (line[3] == ' '){ // if the fourth char is a space, its the general stats line
                        stat_pointer->cpu = cpu_stats_container;
                    } else { // it's the core stats line
                        stat_pointer->cores[cpu_count] = cpu_stats_container;
                        cpu_count++;
                    }
                } else if (strcmp(first_word, "intr") == 0) {
                    char* num_str = get_seccond_arg(line, 5); 
                    stat_pointer->intr_0 = atoi(num_str);
                } else if (strcmp(first_word, "ctxt") == 0) {
                    char* num_str = get_seccond_arg(line, 5); 
                    stat_pointer->ctxt = atoi(num_str);
                } else if (strcmp(first_word, "btime") == 0) {
                    char* num_str = get_seccond_arg(line, 6);
                    stat_pointer->btime = atoi(num_str);
                } else if (strcmp(first_word, "processes") == 0) {
                    char* num_str = get_seccond_arg(line, 10);
                    stat_pointer->processes = atoi(num_str);
                } else if (strcmp(first_word, "procs_running") == 0) {
                    char* num_str = get_seccond_arg(line, 14);
                    stat_pointer->procs_running = atoi(num_str);
                } else if (strcmp(first_word, "procs_blocked") == 0) {
                    char* num_str = get_seccond_arg(line, 14);
                    stat_pointer->procs_blocked = atoi(num_str);
                } else {
                    //printf("Not parsed parameter: %s \n", first_word);
                }
            }
        }
        line[i-offset] = inp_string[i];
        
    }
    stat_pointer->num_cpus = cpu_count;
    calc_cpu_stats(stat_pointer);

    read_memory_stats(&stat_pointer->memory);
    // disk, net, gpu
    read_disk_stats(&stat_pointer->disk);
    read_network_stats(&stat_pointer->net);
    
    read_gpu_stats(&stat_pointer->gpu);
    

};


char* read_general_stat(const char* path) {
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }
    
size_t capacity = 10000;
    char* data = malloc(capacity);

    if (data == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return NULL;
    }

    // Read the entire file content
    size_t bytes_read = fread(data, 1, capacity, fp);
    if (bytes_read < capacity) {
        if (ferror(fp)) {
            perror("Error reading file");
            free(data);
            fclose(fp);
            return NULL;
        }
        // Handle the case where the file might have been shorter than expected
        data[bytes_read] = '\0';
    } else {
        data[capacity-1] = '\0'; // Null-terminate the string
    }

    fclose(fp);
    return data;
}
void read_disk_stats(disk_stats* disk) {
    FILE* fp = fopen("/proc/diskstats", "r");
    if (!fp) {
        perror("Failed to open /proc/diskstats");
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        // Example line format (fields can vary slightly):
        // 8       0 sda 157698 2233 5023234 107284 301968 253103 6087329 425688 0 190192 532552

        if (strstr(line, "sda")) {  // only main disk
            unsigned long rd_ios, rd_merges, rd_sectors, rd_ticks;
            unsigned long wr_ios, wr_merges, wr_sectors, wr_ticks;
            sscanf(line,
                   "%*d %*d %*s %lu %lu %lu %lu %lu %lu %lu %lu",
                   &rd_ios, &rd_merges, &rd_sectors, &rd_ticks,
                   &wr_ios, &wr_merges, &wr_sectors, &wr_ticks);

            disk->read_sectors = rd_sectors;
            disk->write_sectors = wr_sectors;
            disk->read_MB = rd_sectors * 512.0 / (1024.0 * 1024.0);  // 512 bytes/sector
            disk->write_MB = wr_sectors * 512.0 / (1024.0 * 1024.0);
            break;
        }
    }

    fclose(fp);
}

void read_network_stats(network_stats* net) {
    FILE* fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        perror("Error opening /proc/net/dev");
        return;
    }

    char line[1024];
    int line_count = 0;
    net->total_download_MB = 0;
    net->total_upload_MB = 0;
    

    while (fgets(line, sizeof(line), fp)) {
        line_count++;
        if (line_count <= 2) continue; // Skip headers

        char iface[64];
        unsigned long r_bytes, t_bytes;
        sscanf(line, "%63[^:]: %lu %*s %*s %*s %*s %*s %*s %*s %lu", iface, &r_bytes, &t_bytes);

        // Skip loopback
        if (strncmp(iface, "lo", 2) == 0) continue;

        net->total_download_MB += r_bytes / (1024.0 * 1024.0);
        net->total_upload_MB += t_bytes / (1024.0 * 1024.0);
    }

    fclose(fp);
}
int has_nvidia_gpu() {
    FILE* fp = popen("nvidia-detector", "r");
    if (!fp) return 0;

    char buffer[64];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        pclose(fp);
        return 0;
    }

    pclose(fp);
    return (strncmp(buffer, "None", 4) != 0); // True if NOT "None"
}
void read_gpu_stats(gpu_stats* gpu) {
    gpu->nvidia_gpu = has_nvidia_gpu();
    gpu->gpu_MB = 0.0;
    gpu->gpu_util_percent = 0.0;

    if (!gpu->nvidia_gpu) {
        return;  // Exit early if no NVIDIA GPU
    }

    FILE* fp = popen("nvidia-smi --query-gpu=memory.used,utilization.gpu --format=csv,noheader,nounits", "r");
    if (!fp) {
        perror("nvidia-smi failed");
        gpu->nvidia_gpu = false;
        return;
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), fp)) {
        float mem_used = 0.0, gpu_util = 0.0;
        if (sscanf(buffer, "%f, %f", &mem_used, &gpu_util) == 2) {
            gpu->nvidia_gpu = true;
            gpu->gpu_MB = mem_used;
            gpu->gpu_util_percent = gpu_util;
        } else {
            gpu->nvidia_gpu = false;  // parsing failed
        }
    } else {
        gpu->nvidia_gpu = false;  // no output
    }

    pclose(fp);
}
