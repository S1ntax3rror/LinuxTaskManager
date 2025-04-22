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


void split_general_stat_string(char* inp_string, general_stat* stat_pointer){
    char line[10000];
    int64_t offset = 0;
    for (int i = 0; inp_string[i] != '\0'; i++) {
        if (inp_string[i] == '\n'){
            line[i-offset] = '\0';    
            printf("%s", line);
            offset = i;
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