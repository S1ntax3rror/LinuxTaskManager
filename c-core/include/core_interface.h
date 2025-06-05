#ifndef CORE_INTERFACE_H
#define CORE_INTERFACE_H

#include "trimmed_info.h"       // for trimmed_info
#include "general_stat_query.h" // for general_stat
#include <sys/resource.h>       // for rlim_t



/**
 * Return an array of trimmed_info snapshots.
 * The caller must free() the returned pointer.
 */
trimmed_info* get_process_list(int *out_count);

/**
 * Read aggregate CPU stats from /proc/stat
 */
general_stat get_cpu_stats(void);

/**
 * Send UNIX signal 'signo' to process 'pid'.
 * Returns  0 on success, -1 on error (errno set).
 */
int send_signal(int pid, int signo);

/**
 * Change the “nice” value of a running process.
 * Returns 0 on success, -1 on error (errno set).
 */
int renice_process(int pid, int nice_val);

/**
 * Limit the total CPU‐time (in seconds) a process may consume.
 * Returns 0 on success, -1 on error (errno set).
 */
int set_cpu_limit(int pid, rlim_t seconds);

/**
 * Limit the address‐space (virtual memory) of a process (in bytes).
 * Returns 0 on success, -1 on error (errno set).
 */
int set_ram_limit(int pid, rlim_t bytes);

#endif // CORE_INTERFACE_H
