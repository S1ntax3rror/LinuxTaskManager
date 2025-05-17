#ifndef CORE_INTERFACE_H
#define CORE_INTERFACE_H

#include "trimmed_info.h"
#include "general_stat_query.h"

/**
 * Returns a freshly malloc()'ed array of trimmed_info for every live process.
 * *out_count is set to the number of entries.  Caller must free() the returned pointer.
 */
trimmed_info *get_process_list(int *out_count);

/**
 * Parses /proc/stat and returns a general_stat snapshot.
 */
general_stat get_cpu_stats(void);

#endif // CORE_INTERFACE_H
