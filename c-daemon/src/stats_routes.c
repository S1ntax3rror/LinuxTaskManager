#include "../include/stats_routes.h"
#include "../../c-core/include/core_interface.h"
#include "../include/server.h"
#include <cjson/cJSON.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

/* GET /api/stats/cpu → basic cpu/interrupt/etc stats */
static int handle_cpu_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    cJSON *root = cJSON_CreateObject();
    cJSON *cpu  = cJSON_CreateObject();
    // pull out the aggregate line
    cJSON_AddStringToObject(cpu, "name", gs.cpu.name);
    cJSON_AddNumberToObject(cpu, "nice", gs.cpu.nice);
    cJSON_AddNumberToObject(cpu, "system", gs.cpu.system);
    cJSON_AddNumberToObject(cpu, "idle", gs.cpu.idle);
    cJSON_AddNumberToObject(cpu, "iowait", gs.cpu.iowait);
    cJSON_AddNumberToObject(cpu, "irq", gs.cpu.irq);
    cJSON_AddNumberToObject(cpu, "softirq", gs.cpu.softirq);
    cJSON_AddNumberToObject(cpu, "steal", gs.cpu.steal);
    cJSON_AddNumberToObject(cpu, "guest", gs.cpu.guest);
    cJSON_AddNumberToObject(cpu, "guest_nice", gs.cpu.guest_nice);
    cJSON_AddItemToObject(root, "cpu", cpu);

    // add per-core idle so front-end can compute % itself if needed
    cJSON *cores = cJSON_CreateArray();
    for (int i = 0; i < gs.num_cpus; ++i) {
        cJSON *o = cJSON_CreateObject();
        cJSON_AddStringToObject(o, "name", gs.cores[i].name);
        cJSON_AddNumberToObject(o, "idle", gs.cores[i].idle);
        cJSON_AddItemToArray(cores, o);
    }
    cJSON_AddItemToObject(root, "cores", cores);

    // interrupts, context switches, etc
    cJSON_AddNumberToObject(root, "intr", gs.intr_0);
    cJSON_AddNumberToObject(root, "ctxt", gs.ctxt);
    cJSON_AddNumberToObject(root, "btime", gs.btime);
    cJSON_AddNumberToObject(root, "processes", gs.processes);
    cJSON_AddNumberToObject(root, "procs_running", gs.procs_running);
    cJSON_AddNumberToObject(root, "procs_blocked", gs.procs_blocked);
    cJSON_AddNumberToObject(root, "num_cpus", gs.num_cpus);

    return send_json_response(conn, root);
}

/* GET /api/stats/network → download/upload + timestamp */
static int handle_network_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "total_download_MB", gs.net.total_download_MB);
    cJSON_AddNumberToObject(root, "total_upload_MB", gs.net.total_upload_MB);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    cJSON_AddNumberToObject(root, "timestamp_ms", ts);
    return send_json_response(conn, root);
}

/* GET /api/stats/disk → cumulative read/write */
static int handle_disk_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "total_read_MB", gs.disk.read_MB);
    cJSON_AddNumberToObject(root, "total_write_MB", gs.disk.write_MB);
    return send_json_response(conn, root);
}

/* GET /api/stats/general → loadavg, tasks, cpu%, memory MB */
static int handle_general_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    double la1, la5, la15;
    int run, tot;
    FILE *f = fopen("/proc/loadavg", "r");
    if (f) {
        fscanf(f, "%lf %lf %lf %d/%d %*d", &la1, &la5, &la15, &run, &tot);
        fclose(f);
    } else {
        la1 = la5 = la15 = 0;
        run = tot = 0;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "loadavg1", la1);
    cJSON_AddNumberToObject(root, "loadavg5", la5);
    cJSON_AddNumberToObject(root, "loadavg15", la15);
    cJSON_AddNumberToObject(root, "tasks_total", tot);
    cJSON_AddNumberToObject(root, "tasks_running", run);
    cJSON_AddNumberToObject(root, "cpu_util_percent", gs.total_cpu_utilization_percent);

    cJSON *mem = cJSON_CreateObject();
    cJSON_AddNumberToObject(mem, "total_MB", gs.memory.mem_total_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "free_MB", gs.memory.mem_free_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "available_MB", gs.memory.mem_available_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "buffers_MB", gs.memory.buffers_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "cached_MB", gs.memory.cached_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "swap_total_MB", gs.memory.swap_total_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "swap_free_MB", gs.memory.swap_free_kb / 1024.0);
    cJSON_AddItemToObject(root, "memory", mem);

    return send_json_response(conn, root);
}

int dispatch_stats_routes(struct MHD_Connection *conn,
                          const char *url,
                          const char *method) {
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/cpu"))
        return handle_cpu_stats(conn);
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/network"))
        return handle_network_stats(conn);
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/disk"))
        return handle_disk_stats(conn);
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/general"))
        return handle_general_stats(conn);
    return 0;
}
