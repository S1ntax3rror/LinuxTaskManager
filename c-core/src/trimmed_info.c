#include "../include/trimmed_info.h"
#include <string.h>
#include <time.h>
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

    t.timestamp_ms = proc-> timestamp_ms;
    time_t secs = proc->timestamp_ms / 1000;
    struct tm* tm_info = localtime(&secs);
    strftime(t.time_str, sizeof(t.time_str), "%H:%M:%S", tm_info);
    //printf("trimmed");
    

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
    
    long activetime = proc->utime + proc->stime;
    if (update_sleeper_process(t.pid, activetime) == 1 && t.cmd[0] != '\0') {
        t.is_sleeper = 1;
    } else {
        t.is_sleeper = 0;
    }
    //printf("is sleeper is set to: %i", t.is_sleeper);
    

    return t;
}
