#ifndef HEADER_H
#define HEADER_H

#include "general_stat_query.h"

#define MAX_HEADER_ENTRIES 100

typedef struct {
    general_stat entries[MAX_HEADER_ENTRIES];
    int latest_index;
    int count;
} header;

void init_header(header* h);
void push_general_stat(header* h, general_stat* new_stat,float down_time_sec);
general_stat* get_latest_stat(header* h);

#endif // HEADER_H
