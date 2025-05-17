#ifndef PROC_ENTRY_H
#define PROC_ENTRY_H

#include "proc_stat.h"
#include "timeline.h"

typedef struct proc_entry {
    proc_stat before_info;
    proc_stat now_info;
    proc_timeline history;
    int valid; // 0 = not used, 1 = in use
    int pid;
} proc_entry;

#endif // PROC_ENTRY_H
