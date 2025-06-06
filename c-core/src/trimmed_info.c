#define _DEFAULT_SOURCE   // for getpwuid, sysconf, etc.

#include "../include/../include/trimmed_info.h"
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/sleeper_detection.h"

trimmed_info convert_to_trimmed_info(const proc_stat* proc) {
    trimmed_info t = {0};

    t.pid = proc->pid;
    strncpy(t.comm, proc->comm, sizeof(t.comm));
    t.comm[sizeof(t.comm) - 1] = '\0'; // ensure null-termination
    t.state = proc->state;
    t.nice = proc->nice;         // <-- grab the kernel “nice” value

    t.cpu_percent = proc->cpu_percent;
    t.peak_ram_percent = 0;
    t.ram_percent = proc->ram_percent;
    if (t.ram_percent > t.peak_ram_percent){
        t.peak_ram_percent = t.ram_percent;
    }

    t.timestamp_ms = proc->timestamp_ms;
    {
        time_t secs = proc->timestamp_ms / 1000;
        struct tm* tm_info = localtime(&secs);
        strftime(t.time_str, sizeof(t.time_str), "%H:%M:%S", tm_info);
    }

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
    t.prio = (int) proc->priority;

    t.virt_kb = (unsigned long) (proc->vsize / 1024UL);
    {
        long page_size = sysconf(_SC_PAGESIZE);
        unsigned long rss_bytes = (unsigned long) proc->rss * (unsigned long) page_size;
        t.res_kb = rss_bytes / 1024UL;
    }

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
    {
        char cmdline_path[64];
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", proc->pid);
        FILE *f = fopen(cmdline_path, "r");
        if (f) {
            size_t len = fread(t.cmd, 1, sizeof(t.cmd) - 1, f);
            fclose(f);
            if (len > 0) {
                for (size_t i = 0; i < len - 1; i++) {
                    if (t.cmd[i] == '\0') {
                        t.cmd[i] = ' ';
                    }
                }
                t.cmd[len] = '\0';
            } else {
                t.cmd[0] = '\0';
            }
        } else {
            t.cmd[0] = '\0';
        }
    }

    {
        double system_uptime = 0.0;
        FILE *f = fopen("/proc/uptime", "r");
        if (f) {
            fscanf(f, "%lf", &system_uptime);
            fclose(f);
        }
        long ticks_per_sec = sysconf(_SC_CLK_TCK);
        double proc_start_secs = (double) proc->starttime / (double) ticks_per_sec;
        t.up_time_seconds = system_uptime - proc_start_secs;
        if (t.up_time_seconds < 0) {
            t.up_time_seconds = 0.0;
        }
    }
    
    long activetime = proc->utime + proc->stime;
    if (update_sleeper_process(t.pid, activetime, t.ram_percent) == 1 && t.cmd[0] != '\0') {
        t.is_sleeper = 1;
    } else {
        t.is_sleeper = 0;
    }

    return t;
}
