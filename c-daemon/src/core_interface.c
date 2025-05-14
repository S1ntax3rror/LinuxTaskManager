// core_interface.c
// ------------------------------------------------------------------
// Implements our two core-interface functions by reusing
// the parsing logic from the CLI version.
// ------------------------------------------------------------------

#include "core_interface.h"
#include "general_stat_query.h"
#include "trimmed_info.h"
#include "memory_stats.h"
#include "proc_stat.h"
#include "utils.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // sysconf
#include <sys/time.h>  // gettimeofday

// Build a one-shot list of all processes (no CPU delta yet).
trimmed_info* get_process_list(int *out_count) {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    if (!dp) return NULL;

    // read mem total for RAM% calculation
    memory_stats meminfo;
    read_memory_stats(&meminfo);
    long page_size = sysconf(_SC_PAGESIZE);

    // we'll realloc as needed
    int capacity = 256, count = 0;
    trimmed_info *list = malloc(capacity * sizeof(*list));

    while ((entry = readdir(dp))) {
        if (!is_number(entry->d_name)) continue;

        // read /proc/<pid>/stat
        char path[64];
        snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);

        char buf[2048] = {0};
        read_stat(path, buf, sizeof(buf));

        proc_stat ps = {0};
        split_PID_stat_string(buf, &ps);

        // compute RAM %
        ps.ram_percent = (ps.rss * page_size * 100.0)
                          / (meminfo.mem_total_kb * 1024.0);

        // convert full proc_stat → trimmed_info
        trimmed_info t = convert_to_trimmed_info(&ps);

        if (count >= capacity) {
            capacity *= 2;
            list = realloc(list, capacity * sizeof(*list));
        }
        list[count++] = t;
    }
    closedir(dp);

    *out_count = count;
    return list;
}

// Read + parse /proc/stat into our general_stat struct
general_stat get_cpu_stats(void) {
    char *raw = read_general_stat("/proc/stat");
    general_stat gs = {0};
    split_general_stat_string(raw, &gs);
    free(raw);
    return gs;
}
