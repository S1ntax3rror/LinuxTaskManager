#include "../include/process_routes.h"
#include "../../c-core/include/core_interface.h"
#include "../include/server.h"
#include "../../c-core/include/general_stat_query.h"
#include "memory_stats.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>   // for SIGKILL, SIGTERM
#include <ctype.h>    // for isdigit()
#include <errno.h>    // for errno
#include <unistd.h>   // for close()

/* helper to pull out a string field from JSON; caller must free() */
static char *json_get_string(const char *body, const char *field) {
    cJSON *root = cJSON_Parse(body);
    if (!root) return NULL;
    cJSON *item = cJSON_GetObjectItem(root, field);
    char *out = NULL;
    if (item && cJSON_IsString(item)) {
        out = strdup(item->valuestring);
    }
    cJSON_Delete(root);
    return out;
}

/* 
 * GET /api/processes 
 * Return a JSON array of trimmed_info 
 */
int handle_process_list(struct MHD_Connection *conn) {
    int count;
    trimmed_info *list = get_process_list(&count);
    printf("[process_routes] fetched %d processes\n", count);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; ++i) {
        cJSON *o = cJSON_CreateObject();
        // pid, cmd, comm, state…
        cJSON_AddNumberToObject(o, "pid", list[i].pid);
        cJSON_AddStringToObject(o, "cmd", list[i].cmd);
        cJSON_AddStringToObject(o, "comm", list[i].comm);
        char s[2] = { list[i].state, '\0' };
        cJSON_AddStringToObject(o, "state", s);
        // cpu/ram stats, user/prio, memory pages
        cJSON_AddNumberToObject(o, "cpuPercent", list[i].cpu_percent);
        cJSON_AddNumberToObject(o, "ramPercent", list[i].ram_percent);
        cJSON_AddNumberToObject(o, "nice", list[i].nice);
        cJSON_AddStringToObject(o, "username", list[i].username);
        cJSON_AddNumberToObject(o, "prio", list[i].prio);
        cJSON_AddNumberToObject(o, "virt", list[i].virt_kb);
        cJSON_AddNumberToObject(o, "res", list[i].res_kb);
        cJSON_AddNumberToObject(o, "shared", list[i].shared_kb);
        cJSON_AddNumberToObject(o, "upTime", list[i].up_time_seconds);
        cJSON_AddNumberToObject(o, "is_sleeper", list[i].is_sleeper);
        cJSON_AddItemToArray(arr, o);
    }
    free(list);
    // send_json_response already sets Content-Type; no CORS here
    return send_json_response(conn, arr);
}

/*
 * GET /api/cpu_mem
 * Return per-core % and memory+swap stats
 */
int handle_cores_and_memory(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();
    memory_stats mem = gs.memory;

    cJSON *root = cJSON_CreateObject();
    cJSON *cpujs = cJSON_CreateObject();
    // build core_percent map
    for (int i = 0; i < gs.num_cpus; i++) {
        char key[16];
        snprintf(key, sizeof(key), "cpu%d", i);
        cJSON_AddNumberToObject(cpujs, key, gs.cores[i].core_percent);
    }
    cJSON_AddItemToObject(root, "core_percent", cpujs);

    cJSON *memjs = cJSON_CreateObject();
    int used_kb = mem.mem_total_kb - mem.mem_available_kb;
    cJSON_AddNumberToObject(memjs, "total_kb", mem.mem_total_kb);
    cJSON_AddNumberToObject(memjs, "used_kb", used_kb);
    cJSON_AddNumberToObject(memjs, "free_kb", mem.mem_free_kb);
    cJSON_AddNumberToObject(memjs, "cached_kb", mem.cached_kb);
    cJSON_AddNumberToObject(memjs, "buffered_kb", mem.buffers_kb);
    cJSON_AddItemToObject(root, "memory_stats", memjs);

    cJSON *swapjs = cJSON_CreateObject();
    cJSON_AddNumberToObject(swapjs, "total_kb", mem.swap_total_kb);
    cJSON_AddNumberToObject(swapjs, "used_kb", mem.swap_used_kb);
    cJSON_AddNumberToObject(swapjs, "free_kb", mem.swap_free_kb);
    cJSON_AddItemToObject(root, "swap_stats", swapjs);

    // serialize + send; no CORS header any more
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    struct MHD_Response *resp = 
      MHD_create_response_from_buffer(strlen(json), (void*)json, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(resp, "Content-Type", "application/json");
    int ret = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    return ret;
}

/*
 * POST /api/processes/{pid}/signal
 */
int handle_signal(struct MHD_Connection *conn, int pid, const char *body) {
    char *cmd = json_get_string(body, "cmd");
    if (!cmd) {
        struct MHD_Response *r = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        int rc = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, r);
        MHD_destroy_response(r);
        return rc;
    }
    int signo = -1;
    if (!strcasecmp(cmd, "KILL") || !strcasecmp(cmd, "SIGKILL")) signo = SIGKILL;
    else if (!strcasecmp(cmd, "TERM") || !strcasecmp(cmd, "SIGTERM")) signo = SIGTERM;
    else if (isdigit((unsigned char)cmd[0])) signo = atoi(cmd);
    free(cmd);
    if (signo <= 0) return MHD_HTTP_BAD_REQUEST;
    return send_signal(pid, signo) == 0
         ? MHD_HTTP_NO_CONTENT
         : MHD_HTTP_INTERNAL_SERVER_ERROR;
}
