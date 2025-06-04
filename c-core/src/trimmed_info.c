#define _DEFAULT_SOURCE
#include "../include/trimmed_info.h"
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

trimmed_info convert_to_trimmed_info(const proc_stat* proc) {
    trimmed_info t = {0};

    /* ─── COPY EXISTING proc_stat FIELDS ─── */
    t.pid         = proc->pid;
    strncpy(t.comm, proc->comm, sizeof(t.comm) - 1);
    t.comm[sizeof(t.comm) - 1] = '\0';
    t.state       = proc->state;
    t.nice        = proc->nice;            // kernel nice
    t.cpu_percent = proc->cpu_percent;
    t.ram_percent = proc->ram_percent;

    t.timestamp_ms = proc->timestamp_ms;
    {
        time_t secs = proc->timestamp_ms / 1000;
        struct tm* tm_info = localtime(&secs);
        strftime(t.time_str, sizeof(t.time_str), "%H:%M:%S", tm_info);
    }

    /* - new fields population - a bit more logic / therefore more code - */

    /* 1) Username: stat /proc/[pid], extract st_uid → getpwuid */
    {
        struct stat st;
        char proc_path[64];
        snprintf(proc_path, sizeof(proc_path), "/proc/%d", proc->pid);
        if (stat(proc_path, &st) == 0) {
            struct passwd *pw = getpwuid(st.st_uid);
            if (pw) {
                strncpy(t.username, pw->pw_name, sizeof(t.username) - 1);
                t.username[sizeof(t.username) - 1] = '\0';
            } else {
                /* Fallback to numeric UID string */
                snprintf(t.username, sizeof(t.username), "%u", st.st_uid);
            }
        } else {
            strcpy(t.username, "unknown");
        }
    }

    /* 2) Priority (“prio”) */
    t.prio = (int) proc->priority;

    /* 3) VIRT (in KiB) from proc->vsize (bytes / 1024) */
    t.virt_kb = (unsigned long) (proc->vsize / 1024UL);

    /* 4) RES (in KiB) from proc->rss (pages * page_size / 1024) */
    {
        long page_size = sysconf(_SC_PAGESIZE);
        unsigned long rss_bytes = (unsigned long) proc->rss * (unsigned long) page_size;
        t.res_kb = rss_bytes / 1024UL;
    }

    /* 5) SHARED (in KiB) from /proc/[pid]/statm (third field is “shared” pages) */
    {
        char statm_path[64];
        snprintf(statm_path, sizeof(statm_path), "/proc/%d/statm", proc->pid);
        FILE *f = fopen(statm_path, "r");
        if (f) {
            unsigned long size_pages = 0, res_pages = 0, shared_pages = 0;
            if (fscanf(f, "%lu %lu %lu", &size_pages, &res_pages, &shared_pages) >= 3) {
                long page_size = sysconf(_SC_PAGESIZE);
                unsigned long shared_bytes = shared_pages * (unsigned long) page_size;
                t.shared_kb = shared_bytes / 1024UL;
            }
            fclose(f);
        } else {
            t.shared_kb = 0;
        }
    }

    /* 6) Full command‐line (“cmd”) from /proc/[pid]/cmdline */
    {
        char cmdline_path[64];
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", proc->pid);
        FILE *f = fopen(cmdline_path, "r");
        if (f) {
            size_t len = fread(t.cmd, 1, sizeof(t.cmd) - 1, f);
            fclose(f);
            if (len > 0) {
                /* /proc/[pid]/cmdline is null‐separated arguments.
                   Replace nulls with spaces; ensure trailing null. */
                for (size_t i = 0; i < len - 1; i++) {
                    if (t.cmd[i] == '\0') {
                        t.cmd[i] = ' ';
                    }
                }
                t.cmd[len] = '\0';
            } else {
                /* Fallback to comm if cmdline is empty */
                strncpy(t.cmd, proc->comm, sizeof(t.cmd) - 1);
                t.cmd[sizeof(t.cmd) - 1] = '\0';
            }
        } else {
            /* Could not open cmdline: fallback to comm */
            strncpy(t.cmd, proc->comm, sizeof(t.cmd) - 1);
            t.cmd[sizeof(t.cmd) - 1] = '\0';
        }
    }

    /* 7) Up‐time (in seconds) = system_uptime − (starttime / CLK_TCK) */
    {
        /* a) read system uptime from /proc/uptime */
        double system_uptime = 0.0;
        FILE *f = fopen("/proc/uptime", "r");
        if (f) {
            fscanf(f, "%lf", &system_uptime);
            fclose(f);
        }
        /* b) convert proc->starttime (clock ticks since boot) into seconds */
        long ticks_per_sec = sysconf(_SC_CLK_TCK);
        double proc_start_secs = (double) proc->starttime / (double) ticks_per_sec;
        /* c) compute how many seconds this process has been alive */
        t.up_time_seconds = system_uptime - proc_start_secs;
        if (t.up_time_seconds < 0) {
            t.up_time_seconds = 0.0;
        }
    }

    return t;
}
