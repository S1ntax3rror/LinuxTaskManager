#ifndef CORE_INTERFACE_H
#define CORE_INTERFACE_H

#include "../c-core/include/trimmed_info.h"
#include "../c-core/include/general_stat_query.h"

/* Returns a heap‐allocated array of trimmed_info for current processes.
 * Caller must free() the returned pointer.
 * Sets *out_count to the number of entries. */
trimmed_info* get_process_list(int *out_count);

/* Returns current CPU stats by parsing /proc/stat */
general_stat get_cpu_stats(void);

#endif // CORE_INTERFACE_H
