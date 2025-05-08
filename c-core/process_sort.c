#include "process_sort.h"
#include <stdlib.h>  
#include <string.h>

//cpu
int compare_cpu(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;

    if (t2->cpu_percent > t1->cpu_percent) return 1;
    if (t2->cpu_percent < t1->cpu_percent) return -1;
    return 0;
}

void sort_by_cpu(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_cpu);
}
//ram
int compare_ram(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;

    if (t2->ram_percent > t1->ram_percent) return 1;
    if (t2->ram_percent < t1->ram_percent) return -1;
    return 0;
}

void sort_by_ram(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_ram);
}
//avg_cpu
int compare_avg_cpu(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;

    if (t2->avg_cpu_percent > t1->avg_cpu_percent) return 1;
    if (t2->avg_cpu_percent < t1->avg_cpu_percent) return -1;
    return 0;
}

void sort_by_avg_cpu(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_avg_cpu);

}
//pid
int compare_pid(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;

    if (t2->pid > t1->pid) return 1;
    if (t2->pid < t1->pid) return -1;
    return 0;
}

void sort_by_pid(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_pid);
}

int compare_pid_a(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;

    if (t2->pid < t1->pid) return 1;
    if (t2->pid > t1->pid) return -1;
    return 0;
}

void sort_by_pid_a(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_pid_a);
}
//name
int compare_by_name(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;
    return strcmp(t1->comm, t2->comm);
}

void sort_by_name(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_by_name);
}

//state
int compare_by_state(const void* a, const void* b) {
    const trimmed_info* t1 = (const trimmed_info*)a;
    const trimmed_info* t2 = (const trimmed_info*)b;

    return (int)t1->state - (int)t2->state;  
}

void sort_by_state(trimmed_info* list, int count) {
    qsort(list, count, sizeof(trimmed_info), compare_by_state);
}

SortMode read_sort_mode_from_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return SORT_BY_PID;  // fallback default

    char buffer[64];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        printf("false file input needs: <sortingstyle> :cpu, ram, name, state, pid_a, pid_d, avg_cpu");//remove later
        return SORT_BY_PID;
    }
    fclose(fp);

    // remove newline
    buffer[strcspn(buffer, "\n")] = 0;

    if (strcmp(buffer, "cpu") == 0) return SORT_BY_CPU;
    if (strcmp(buffer, "ram") == 0) return SORT_BY_RAM;
    if (strcmp(buffer, "name") == 0) return SORT_BY_NAME;
    if (strcmp(buffer, "state") == 0) return SORT_BY_STATE;
    if (strcmp(buffer, "pid_d") == 0) return SORT_BY_PID;
    if (strcmp(buffer, "pid_a") == 0) return SORT_BY_PID_A;
    if (strcmp(buffer, "avg_cpu") == 0) return SORT_BY_AVG_CPU;

    return SORT_BY_PID;  // fallback
}