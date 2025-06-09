#include "../include/stats_routes.h"
#include "../../c-core/include/core_interface.h"
#include "../include/server.h"
#include <cjson/cJSON.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

/** ---------------------------------------------------------------------
 * Implements HTTP Handlers for our stats ("/api/stats/...") endpoints.
 * 1. cpu, 2. network, 3. disk, 4. general stats plus dispatcher 
 -----------------------------------------------------------------------*/

/*
 * Handle GET /api/stats/cpu
 * -------------------------
 * Returns a JSON object with aggregate CPU fields and per-core idle times.
 */
static int handle_cpu_stats(struct MHD_Connection *conn) {
    // Fetch latest CPU stats (reads /proc/stat, computes percentages)
    general_stat gs = get_cpu_stats();

    // Build root JSON object
    cJSON *root = cJSON_CreateObject();
    // Create nested "cpu" object for aggregate CPU line
    cJSON *cpu  = cJSON_CreateObject();

    // Populate aggregate CPU metrics
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

    // Attach "cpu" object to root
    cJSON_AddItemToObject(root, "cpu", cpu);

    // Build per-core idle times for front-end calculations
    cJSON *cores = cJSON_CreateArray();
    for (int i = 0; i < gs.num_cpus; ++i) {
        cJSON *o = cJSON_CreateObject();
        // Label each core by name (e.g. "cpu0", "cpu1")
        cJSON_AddStringToObject(o, "name", gs.cores[i].name);
        // Report idle ticks for front-end to compute usage if needed
        cJSON_AddNumberToObject(o, "idle", gs.cores[i].idle);
        cJSON_AddItemToArray(cores, o);
    }
    // Attach cores array to root
    cJSON_AddItemToObject(root, "cores", cores);

    // Add other system counters: interrupts, context switches, etc.
    cJSON_AddNumberToObject(root, "intr", gs.intr_0);
    cJSON_AddNumberToObject(root, "ctxt", gs.ctxt);
    cJSON_AddNumberToObject(root, "btime", gs.btime);
    cJSON_AddNumberToObject(root, "processes", gs.processes);
    cJSON_AddNumberToObject(root, "procs_running", gs.procs_running);
    cJSON_AddNumberToObject(root, "procs_blocked", gs.procs_blocked);
    cJSON_AddNumberToObject(root, "num_cpus", gs.num_cpus);

    // Send the JSON response with HTTP 200
    return send_json_response(conn, root);
}


/*
 * Handle GET /api/stats/network
 * ------------------------------
 * Returns cumulative download/upload MB and a timestamp.
 */
static int handle_network_stats(struct MHD_Connection *conn) {
    // Gather network totals from core interface
    general_stat gs = get_cpu_stats();
    cJSON *root = cJSON_CreateObject();

    // Report total bytes (converted to MB by core code)
    cJSON_AddNumberToObject(root, "total_download_MB", gs.net.total_download_MB);
    cJSON_AddNumberToObject(root, "total_upload_MB", gs.net.total_upload_MB);

    // Add a timestamp (ms since epoch) for chart X-axis
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    cJSON_AddNumberToObject(root, "timestamp_ms", ts);

    return send_json_response(conn, root);
}


/*
 * Handle GET /api/stats/disk
 * ---------------------------
 * Returns cumulative disk read/write MB.
 */
static int handle_disk_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "total_read_MB", gs.total_disk_read_MB);
    cJSON_AddNumberToObject(root, "total_write_MB", gs.total_disk_write_MB);
    return send_json_response(conn, root);
}


/*
 * Handle GET /api/stats/general
 * ------------------------------
 * Combines load averages, task counts, CPU%, and memory info.
 */
static int handle_general_stats(struct MHD_Connection *conn) {
    // Get CPU and memory stats in one call
    general_stat gs = get_cpu_stats();

    // Read load average and task counts from /proc/loadavg
    double la1, la5, la15;
    int run, tot;
    FILE *f = fopen("/proc/loadavg", "r");
    if (f) {
        fscanf(f, "%lf %lf %lf %d/%d %*d", &la1, &la5, &la15, &run, &tot);
        fclose(f);
    } else {
        // Fallback if file open fails
        la1 = la5 = la15 = 0;
        run = tot = 0;
    }

    cJSON *root = cJSON_CreateObject();
    // Attach load averages
    cJSON_AddNumberToObject(root, "loadavg1", la1);
    cJSON_AddNumberToObject(root, "loadavg5", la5);
    cJSON_AddNumberToObject(root, "loadavg15", la15);
    // Attach task totals and running count
    cJSON_AddNumberToObject(root, "tasks_total", tot);
    cJSON_AddNumberToObject(root, "tasks_running", run);
    // Attach overall CPU utilization percentage
    cJSON_AddNumberToObject(root, "cpu_util_percent", gs.total_cpu_utilization_percent);

    // Build a nested memory object reporting MB (converted in core code)
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


static int handle_all_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    cJSON *root = cJSON_CreateObject();
    
    // ─── CPU section ────────────────────────────────────────────────────────
    cJSON *cpu = cJSON_CreateObject();
    cJSON_AddStringToObject(cpu, "name",       gs.cpu.name);
    cJSON_AddNumberToObject(cpu, "nice",       gs.cpu.nice);
    cJSON_AddNumberToObject(cpu, "system",     gs.cpu.system);
    cJSON_AddNumberToObject(cpu, "idle",       gs.cpu.idle);
    cJSON_AddNumberToObject(cpu, "iowait",     gs.cpu.iowait);
    cJSON_AddNumberToObject(cpu, "irq",        gs.cpu.irq);
    cJSON_AddNumberToObject(cpu, "softirq",    gs.cpu.softirq);
    cJSON_AddNumberToObject(cpu, "steal",      gs.cpu.steal);
    cJSON_AddNumberToObject(cpu, "guest",      gs.cpu.guest);
    cJSON_AddNumberToObject(cpu, "guest_nice", gs.cpu.guest_nice);
    cJSON_AddItemToObject(root, "cpu", cpu);

    // ─── proc_stats section ─────────────────────────────────────────────────
    cJSON *proc_stats = cJSON_CreateObject();
    cJSON_AddNumberToObject(proc_stats, "intr",          gs.intr_0);
    cJSON_AddNumberToObject(proc_stats, "ctxt",          gs.ctxt);
    cJSON_AddNumberToObject(proc_stats, "btime",         gs.btime);
    cJSON_AddNumberToObject(proc_stats, "processes",     gs.processes);
    cJSON_AddNumberToObject(proc_stats, "procs_running", gs.procs_running);
    cJSON_AddNumberToObject(proc_stats, "procs_blocked", gs.procs_blocked);
    cJSON_AddNumberToObject(proc_stats, "num_cpus",      gs.num_cpus);
    cJSON_AddItemToObject(root, "proc_stats", proc_stats);

    // ─── per-core section ────────────────────────────────────────────────────
    cJSON *cores = cJSON_CreateArray();
    for (int i = 0; i < gs.num_cpus; ++i) {
        cJSON *o = cJSON_CreateObject();
        char key[16];
        snprintf(key, sizeof(key), "cpu%d", i);
        cJSON_AddNumberToObject(o, key, gs.cores[i].core_percent);
        cJSON_AddStringToObject(o, "name", gs.cores[i].name);
        cJSON_AddNumberToObject(o, "idle", gs.cores[i].idle);
        cJSON_AddItemToArray(cores, o);
    }
    cJSON_AddItemToObject(root, "cores", cores);

    // ─── network section ─────────────────────────────────────────────────────
    cJSON *net_stats = cJSON_CreateObject();
    cJSON_AddNumberToObject(net_stats, "total_download_MB", gs.net.total_download_MB);
    cJSON_AddNumberToObject(net_stats, "total_upload_MB",   gs.net.total_upload_MB);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    cJSON_AddNumberToObject(net_stats, "timestamp_ms", ts);
    cJSON_AddItemToObject(root, "net_stats", net_stats);

    // ─── disk section ────────────────────────────────────────────────────────
    // Build an OBJECT, keyed by disk name
    cJSON *disc_stats = cJSON_CreateObject();
    gs.total_disk_read_MB  = 0;
    gs.total_disk_write_MB = 0;
    for (int i = 0; i < gs.num_disks; ++i) {
        // create one object per disk
        cJSON *disk = cJSON_CreateObject();
        cJSON_AddStringToObject(disk, "name",     gs.disk[i].name);
        cJSON_AddNumberToObject(disk, "read_MB",  gs.disk[i].read_MB);
        cJSON_AddNumberToObject(disk, "write_MB", gs.disk[i].write_MB);

        // attach it under its own name as the key
        cJSON_AddItemToObject(disc_stats, gs.disk[i].name, disk);

        // accumulate totals
        gs.total_disk_read_MB  += gs.disk[i].read_MB;
        gs.total_disk_write_MB += gs.disk[i].write_MB;
    }
    // Finally add the total fields
    cJSON_AddNumberToObject(disc_stats, "total_read_MB",  gs.total_disk_read_MB);
    cJSON_AddNumberToObject(disc_stats, "total_write_MB", gs.total_disk_write_MB);

    cJSON_AddItemToObject(root, "disc_stats", disc_stats);

    // ─── load + memory section ───────────────────────────────────────────────
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
    cJSON *load_stats = cJSON_CreateObject();
    cJSON_AddNumberToObject(load_stats, "loadavg1",        la1);
    cJSON_AddNumberToObject(load_stats, "loadavg5",        la5);
    cJSON_AddNumberToObject(load_stats, "loadavg15",       la15);
    cJSON_AddNumberToObject(load_stats, "tasks_total",     tot);
    cJSON_AddNumberToObject(load_stats, "tasks_running",   run);
    cJSON_AddNumberToObject(load_stats, "cpu_util_percent", gs.total_cpu_utilization_percent);
    cJSON_AddItemToObject(root, "load_stats", load_stats);

    cJSON *mem = cJSON_CreateObject();
    cJSON_AddNumberToObject(mem, "total_MB",      gs.memory.mem_total_kb   / 1024.0);
    cJSON_AddNumberToObject(mem, "free_MB",       gs.memory.mem_free_kb    / 1024.0);
    cJSON_AddNumberToObject(mem, "available_MB",  gs.memory.mem_available_kb / 1024.0);
    cJSON_AddNumberToObject(mem, "buffers_MB",    gs.memory.buffers_kb     / 1024.0);
    cJSON_AddNumberToObject(mem, "cached_MB",     gs.memory.cached_kb      / 1024.0);
    cJSON_AddNumberToObject(mem, "swap_total_MB", gs.memory.swap_total_kb  / 1024.0);
    cJSON_AddNumberToObject(mem, "swap_free_MB",  gs.memory.swap_free_kb   / 1024.0);
    cJSON_AddItemToObject(root, "memory", mem);

    // ─── send it off ─────────────────────────────────────────────────────────
    return send_json_response(conn, root);
}



/*
 * Dispatch incoming /api/stats/* requests to their handlers.
 * Returns non-zero if a handler was invoked, or 0 if URL didn't match.
 */
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
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/all"))
        return handle_all_stats(conn);
    return 0;
}
