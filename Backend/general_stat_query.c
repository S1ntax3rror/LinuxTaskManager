#include "general_stat_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_general_stat(general_stat *stat_container){
    printf("To be implemented");
};


void set_field_in_general_stat(general_stat* stat_container, int index, char* value){
    printf("to be implemented");
};


void print_cpu_stats(cpu_stats* cpu_container) {
    printf("name: %s\n", cpu_container->name);
    printf("nice: %ld\n", cpu_container->nice);
    printf("system: %ld\n", cpu_container->system);
    printf("idle: %ld\n", cpu_container->idle);
    printf("iowait: %ld\n", cpu_container->iowait);
    printf("irq: %ld\n", cpu_container->irq);
    printf("steal: %ld\n", cpu_container->steal);
    printf("guest: %ld\n", cpu_container->guest);
    printf("guest_nice: %ld\n", cpu_container->guest_nice);
}


void set_cpu_field(cpu_stats* cpu_container, int index, char* value){
    switch (index)
    {
    case 0: strncpy(cpu_container->name, value, sizeof(cpu_container->name) - 1); break;
    case 1: cpu_container->nice = atoi(value); break;
    case 2: cpu_container->system = atoi(value); break;
    case 3: cpu_container->idle = atoi(value); break;
    case 4: cpu_container->iowait = atoi(value); break;
    case 5: cpu_container->irq = atoi(value); break;
    case 6: cpu_container->steal = atoi(value); break;
    case 7: cpu_container->guest = atoi(value); break;
    case 8: cpu_container->guest_nice = atoi(value); break;
    default: break;
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
                char firstThree[4]; // get first 3 chars of string (to check if its a cpu stat thingy)
                strncpy(firstThree, line, 3);
                firstThree[3] = '\0';

                char firstThree[4]; // get first 3 chars of string (to check if its a cpu stat thingy)
                strncpy(firstThree, line, 3);
                firstThree[3] = '\0';

                if (strcmp(firstThree, "cpu") == 0 || strcmp(firstThree, "cpu ") == 0){
                    printf("%s \n", line);
                    cpu_stats cpu_stats_container;
                    set_cpu(line, &cpu_stats_container);
                    print_cpu_stats(&cpu_stats_container);

                    if (line[4] == ' '){
                        stat_pointer->cpu = cpu_stats_container;
                    } else {
                        stat_pointer->cores[cpu_count] = cpu_stats_container;
                        cpu_count++;
                    }
                }
            }

        }
        line[i-offset] = inp_string[i];
        
    }
    //printf("not printed stuff %s", line);
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