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
        print_cpu_stats(&core_stat);
        total_nonproductive_time = total_nonproductive_time + core_stat.iowait + core_stat.idle;
        total_cpu_time += core_stat.user + core_stat.nice + core_stat.system + core_stat.idle + core_stat.iowait + core_stat.irq + core_stat.steal;
    }
    float cpu_percent = total_nonproductive_time/total_cpu_time;
    printf("total time nonprod: %li, total prod time: %li, total cpu percent: %f \n\n", total_nonproductive_time, total_cpu_time, cpu_percent);
    g_stat_pointer->total_cpu_utilization_percent = total_nonproductive_time;
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
                    printf("Not parsed parameter: %s \n", first_word);
                }
            }
        }
        line[i-offset] = inp_string[i];
        
    }
    stat_pointer->num_cpus = cpu_count;
    calc_cpu_stats(stat_pointer);
};


char* read_general_stat(const char* path) {
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }
    
    size_t size = 0, capacity = 10000;
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
        data[capacity] = '\0'; // Null-terminate the string
    }

    fclose(fp);
    return data;
}