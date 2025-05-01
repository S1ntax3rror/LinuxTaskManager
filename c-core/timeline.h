#define MAX_TIMELINE 30

#include "trimmed_info.h"

typedef struct proc_timeline {
    trimmed_info timeline[MAX_TIMELINE];
    int latest_index;  // circular buffer index
    int pid;           // for easy reference
    int valid;         // optional
} proc_timeline;
