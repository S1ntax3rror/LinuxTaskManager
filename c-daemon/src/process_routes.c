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

/** ---------------------------------------------------------------------
 * Implements HTTP Handlers for our process endpoints
 * 1. processlist, 2. cpu cores and memory, 3. signal (kill)
 -----------------------------------------------------------------------*/
 

// Helper: get a string value from JSON
// - body: raw JSON text
// - field: name of the string property to read
// Returns a newly malloc'd C string or NULL if missing/invalid
static char *json_get_string(const char *body, const char *field) {
    // Parse the JSON text into an object tree
    cJSON *root = cJSON_Parse(body);
    if (!root) {
        // Invalid JSON input
        return NULL;
    }

    // Look for the named field in the JSON object
    cJSON *item = cJSON_GetObjectItem(root, field);
    char *out = NULL;
    if (item && cJSON_IsString(item)) {
        // Duplicate the string so caller owns the memory
        out = strdup(item->valuestring);
    }

    // Clean up the parsed JSON tree
    cJSON_Delete(root);
    return out;
}

/**
 * GET /api/processes
 * Respond with a JSON array of trimmed_info objects for each process.
 */
int handle_process_list(struct MHD_Connection *conn) {
    int count;
    // Fetch processes and build C array
    trimmed_info *list = get_process_list(&count);
    printf("[process_routes] fetched %d processes\n", count);

    // Create JSON array
    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; ++i) {
        cJSON *obj = cJSON_CreateObject();
        // Basic fields
        cJSON_AddNumberToObject(obj, "pid", list[i].pid);
        cJSON_AddStringToObject(obj, "cmd", list[i].cmd);
        cJSON_AddStringToObject(obj, "comm", list[i].comm);
        char state_str[2] = { list[i].state, '\0' };
        cJSON_AddStringToObject(obj, "state", state_str);
        // Usage and metadata
        cJSON_AddNumberToObject(obj, "cpuPercent",  list[i].cpu_percent);
        cJSON_AddNumberToObject(obj, "ramPercent",  list[i].ram_percent);
        cJSON_AddNumberToObject(obj, "nice",        list[i].nice);
        cJSON_AddStringToObject(obj, "username",   list[i].username);
        cJSON_AddNumberToObject(obj, "prio",        list[i].prio);
        cJSON_AddNumberToObject(obj, "virt",        list[i].virt_kb);
        cJSON_AddNumberToObject(obj, "res",         list[i].res_kb);
        cJSON_AddNumberToObject(obj, "shared",      list[i].shared_kb);
        cJSON_AddNumberToObject(obj, "upTime",      list[i].up_time_seconds);
        cJSON_AddNumberToObject(obj, "is_sleeper",  list[i].is_sleeper);

        cJSON_AddItemToArray(arr, obj);
    }
    free(list);

    // Send JSON response (sends HTTP 200 and Content-Type header)
    return send_json_response(conn, arr);
}

/**
 * GET /api/cpu_mem
 * Return per-core CPU % plus memory & swap stats.
 */
int handle_cores_and_memory(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();        // Read /proc/stat etc.
    memory_stats mem = gs.memory;             // Extract memory stats

    cJSON *root = cJSON_CreateObject();

    // Build "core_percent" map: { "cpu0": 5.3, "cpu1": 2.1, ... }
    cJSON *cpujs = cJSON_CreateObject();
    for (int i = 0; i < gs.num_cpus; i++) {
        char key[16];
        snprintf(key, sizeof(key), "cpu%d", i);
        cJSON_AddNumberToObject(cpujs, key, gs.cores[i].core_percent);
    }
    cJSON_AddItemToObject(root, "core_percent", cpujs);

    // Memory stats
    cJSON *memjs = cJSON_CreateObject();
    int used_kb = mem.mem_total_kb - mem.mem_available_kb;
    cJSON_AddNumberToObject(memjs, "total_kb",    mem.mem_total_kb);
    cJSON_AddNumberToObject(memjs, "used_kb",     used_kb);
    cJSON_AddNumberToObject(memjs, "free_kb",     mem.mem_free_kb);
    cJSON_AddNumberToObject(memjs, "cached_kb",   mem.cached_kb);
    cJSON_AddNumberToObject(memjs, "buffered_kb", mem.buffers_kb);
    cJSON_AddItemToObject(root, "memory_stats", memjs);

    // Swap stats
    cJSON *swapjs = cJSON_CreateObject();
    cJSON_AddNumberToObject(swapjs, "total_kb", mem.swap_total_kb);
    cJSON_AddNumberToObject(swapjs, "used_kb",  mem.swap_used_kb);
    cJSON_AddNumberToObject(swapjs, "free_kb",  mem.swap_free_kb);
    cJSON_AddItemToObject(root, "swap_stats", swapjs);

    // Serialize and send (we build buffer manually here)
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    struct MHD_Response *resp =
        MHD_create_response_from_buffer(strlen(json), json, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(resp, "Content-Type", "application/json");
    int ret = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    return ret;
}

/**
 * POST /api/processes/{pid}/signal
 * Parse "cmd" from JSON and send signal (KILL/TERM/number).
 */
int handle_signal(struct MHD_Connection *conn, int pid, const char *body) {
    char *cmd = json_get_string(body, "cmd");
    if (!cmd) {
        // Bad request if no cmd field
        struct MHD_Response *r = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        int rc = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, r);
        MHD_destroy_response(r);
        return rc;
    }

    int signo = -1;
    // Map common strings to signals
    if (!strcasecmp(cmd, "KILL") || !strcasecmp(cmd, "SIGKILL")) signo = SIGKILL;
    else if (!strcasecmp(cmd, "TERM") || !strcasecmp(cmd, "SIGTERM")) signo = SIGTERM;
    else if (isdigit((unsigned char)cmd[0])) signo = atoi(cmd);
    free(cmd);

    if (signo <= 0) {
        return MHD_HTTP_BAD_REQUEST;
    }

    // Send the signal via core interface
    return send_signal(pid, signo) == 0
         ? MHD_HTTP_NO_CONTENT
         : MHD_HTTP_INTERNAL_SERVER_ERROR;
}
