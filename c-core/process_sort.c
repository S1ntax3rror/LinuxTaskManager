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