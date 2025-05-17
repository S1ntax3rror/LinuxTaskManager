#include "core_interface.h"
#include "proc_entry.h"        // now has .history
#include "utils.h"             // is_number(), count_folders()
#include "proc_stat.h"         // split_PID_stat_string(), read_stat()
#include "cpu_usage.h"         // calculate_normalized_cpu_usage()
#include "memory_stats.h"      // read_memory_stats()
#include "trimmed_info.h"      // convert_to_trimmed_info()
#include "general_stat_query.h"
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>            // usleep, sysconf
#include <sys/time.h>          // gettimeofday

#define MAX_PROCS  2048
#define SAMPLE_US 200000       // 200 ms sampling

static proc_entry entries[MAX_PROCS];
static int        num_entries = 0;
static int        first_run   = 1;

trimmed_info *
get_process_list(int *out_count) {
    struct dirent *ent;
    DIR           *dp;
    memory_stats   mem;
    read_memory_stats(&mem);
    long pagesz     = sysconf(_SC_PAGESIZE);
    double interval = SAMPLE_US / 1e6;

    // ─── 1) on first call, build entries[].before_info ───
    if (first_run) {
        dp = opendir("/proc");
        while (dp && (ent = readdir(dp))) {
            if (!is_number(ent->d_name)) continue;
            int pid = atoi(ent->d_name);
            char path[64];
            snprintf(path, sizeof(path), "/proc/%d/stat", pid);
            char buf[2048];
            read_stat(path, buf, sizeof(buf));

            proc_stat s = {0};
            split_PID_stat_string(buf, &s);

            entries[num_entries].pid              = pid;
            entries[num_entries].before_info      = s;
            entries[num_entries].history.latest_index = 0;
            entries[num_entries].valid            = 1;

            if (++num_entries >= MAX_PROCS) break;
        }
        if (dp) closedir(dp);
        first_run = 0;
    }

    // ─── 2) wait exactly one interval ───
    usleep(SAMPLE_US);

    // ─── 3) read “now” & update each matching entry ───
    dp = opendir("/proc");
    while (dp && (ent = readdir(dp))) {
        if (!is_number(ent->d_name)) continue;
        int pid = atoi(ent->d_name);

        // read now snapshot
        proc_stat now = {0};
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        char buf[2048];
        read_stat(path, buf, sizeof(buf));
        split_PID_stat_string(buf, &now);

        // compute CPU% & RAM%
        calculate_normalized_cpu_usage(
          &entries[0].before_info, &now, interval
        );
        now.ram_percent = (now.rss * pagesz * 100.0)
                          / (mem.mem_total_kb * 1024.0);

        // find its slot
        for (int i = 0; i < num_entries; i++) {
            if (!entries[i].valid || entries[i].pid != pid) continue;

            // push into ring
            trimmed_info t = convert_to_trimmed_info(&now);
            proc_timeline *tl = &entries[i].history;
            tl->timeline[tl->latest_index] = t;
            tl->latest_index = (tl->latest_index + 1) % MAX_TIMELINE;

            // shift
            entries[i].before_info = now;
            break;
        }
    }
    if (dp) closedir(dp);

    // ─── 4) flatten into a new array ───
    trimmed_info *out = malloc(num_entries * sizeof(*out));
    int cnt = 0;
    for (int i = 0; i < num_entries; i++) {
        proc_timeline *tl = &entries[i].history;
        int idx = (tl->latest_index - 1 + MAX_TIMELINE) % MAX_TIMELINE;
        out[cnt++] = tl->timeline[idx];
    }

    *out_count = cnt;
    return out;
}

general_stat
get_cpu_stats(void) {
    char *raw = read_general_stat("/proc/stat");
    general_stat gs = {0};
    split_general_stat_string(raw, &gs);
    free(raw);
    return gs;
}
