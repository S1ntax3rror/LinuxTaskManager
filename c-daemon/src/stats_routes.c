#include "../include/stats_routes.h"
#include "../../c-core/include/core_interface.h"
#include "../include/server.h"
#include <string.h>
#include <cjson/cJSON.h>
#include <sys/time.h>
#include <stdint.h>



static int handle_cpu_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();

    cJSON *root = cJSON_CreateObject();
    cJSON *cpu  = cJSON_CreateObject();

    cJSON_AddStringToObject(cpu,  "name",       gs.cpu.name);
    cJSON_AddNumberToObject(cpu,  "nice",       gs.cpu.nice);
    cJSON_AddNumberToObject(cpu,  "system",     gs.cpu.system);
    cJSON_AddNumberToObject(cpu,  "idle",       gs.cpu.idle);
    cJSON_AddNumberToObject(cpu,  "iowait",     gs.cpu.iowait);
    cJSON_AddNumberToObject(cpu,  "irq",        gs.cpu.irq);
    cJSON_AddNumberToObject(cpu,  "softirq",    gs.cpu.softirq);
    cJSON_AddNumberToObject(cpu,  "steal",      gs.cpu.steal);
    cJSON_AddNumberToObject(cpu,  "guest",      gs.cpu.guest);
    cJSON_AddNumberToObject(cpu,  "guest_nice", gs.cpu.guest_nice);
    cJSON_AddItemToObject(root, "cpu", cpu);

    cJSON *cores = cJSON_CreateArray();
    for (int i = 0; i < gs.num_cpus; ++i) {
        cpu_stats *c = &gs.cores[i];
        cJSON *cobj = cJSON_CreateObject();
        cJSON_AddStringToObject(cobj, "name", c->name);
        cJSON_AddNumberToObject(cobj, "idle", c->idle);
        cJSON_AddItemToArray(cores, cobj);
    }
    cJSON_AddItemToObject(root, "cores", cores);

    cJSON_AddNumberToObject(root, "intr",         gs.intr_0);
    cJSON_AddNumberToObject(root, "ctxt",         gs.ctxt);
    cJSON_AddNumberToObject(root, "btime",        gs.btime);
    cJSON_AddNumberToObject(root, "processes",    gs.processes);
    cJSON_AddNumberToObject(root, "procs_running",gs.procs_running);
    cJSON_AddNumberToObject(root, "procs_blocked",gs.procs_blocked);
    cJSON_AddNumberToObject(root, "num_cpus",      gs.num_cpus);

    return send_json_response(conn, root);
}

static int handle_network_stats(struct MHD_Connection *conn) {
    // 1) Fetch your general_stat (which contains net.total_download_MB, net.total_upload_MB, etc.)
    general_stat gs = get_cpu_stats();

    // 2) Build a cJSON object
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "total_download_MB", gs.net.total_download_MB);
    cJSON_AddNumberToObject(root, "total_upload_MB",   gs.net.total_upload_MB);

    // 3) Compute a real timestamp (ms since epoch)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ts_ms = (uint64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
    cJSON_AddNumberToObject(root, "timestamp_ms", ts_ms);

    // 4) Serialize the JSON to a string
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 5) Create an MHD response, add CORS, and queue it
    struct MHD_Response *resp = MHD_create_response_from_buffer(
        strlen(json_str),
        (void *)json_str,
        MHD_RESPMEM_MUST_FREE);

    MHD_add_response_header(resp, "Content-Type", "application/json");
    MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");

    int ret = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    return ret;
}

static int handle_disk_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();    
    cJSON *root = cJSON_CreateObject();

    // “total_read_MB” and “total_write_MB” are the cumulative MB values from /proc/diskstats
    cJSON_AddNumberToObject(root, "total_read_MB",  gs.disk.read_MB);
    cJSON_AddNumberToObject(root, "total_write_MB", gs.disk.write_MB);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    struct MHD_Response *resp = MHD_create_response_from_buffer(
        strlen(json_str),
        (void *)json_str,
        MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(resp, "Content-Type", "application/json");
    MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");
    int ret = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    return ret;
}



int dispatch_stats_routes(struct MHD_Connection *conn,
                          const char *url,
                          const char *method)
{
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/cpu")) {
        return handle_cpu_stats(conn);
    }
    // NEW: If someone does GET /api/stats/network → handle network stats
    if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/network")) {
        return handle_network_stats(conn);
    }
     // NEW: GET /api/stats/disk 
     if (!strcmp(method, "GET") && !strcmp(url, "/api/stats/disk")) {
        return handle_disk_stats(conn);
    }
    
    return 0;
}


