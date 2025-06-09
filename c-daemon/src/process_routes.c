// c-daemon/src/process_routes.c

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

/**
 * Helper: extract a string field from JSON; caller must free().
 */
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

/**
 * Handle GET /api/processes
 * Return a JSON array of all processes (trimmed_info).
 */
int handle_process_list(struct MHD_Connection *conn) {
    int count;
    trimmed_info *list = get_process_list(&count);
    printf("[process_routes] fetched %d processes\n", count);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; ++i) {
        cJSON *o = cJSON_CreateObject();

        cJSON_AddNumberToObject(o, "pid",        list[i].pid);

        // ← NEW: expose the full command‐line (empty string for kernel threads)
        cJSON_AddStringToObject(o, "cmd",        list[i].cmd);

        cJSON_AddStringToObject(o, "comm",       list[i].comm);
        char state_str[2] = { list[i].state, '\0' };
        cJSON_AddStringToObject(o, "state",      state_str);

        // CPU / RAM / nice
        cJSON_AddNumberToObject(o, "cpuPercent", list[i].cpu_percent);
        cJSON_AddNumberToObject(o, "ramPercent", list[i].ram_percent);
        cJSON_AddNumberToObject(o, "nice",       list[i].nice);

        // ← NEW: expose the username (owner) and priority
        cJSON_AddStringToObject(o, "username", list[i].username);
        cJSON_AddNumberToObject(o, "prio",     list[i].prio);

        // ← NEW: expose VIRT, RES, SHARED (all in KiB)
        cJSON_AddNumberToObject(o, "virt",   list[i].virt_kb);
        cJSON_AddNumberToObject(o, "res",    list[i].res_kb);
        cJSON_AddNumberToObject(o, "shared", list[i].shared_kb);

        // ← NEW: expose up‐time in seconds
        cJSON_AddNumberToObject(o, "upTime", list[i].up_time_seconds);

        // Append this object to the array
        cJSON_AddNumberToObject(o, "is_sleeper",       list[i].is_sleeper);
        cJSON_AddItemToArray(arr, o);
    }

    free(list);
    return send_json_response(conn, arr);
}

/**
 * Handle GET /api/cpu_mem
 * Return a JSON array of all processes (trimmed_info).
 */
int handle_cores_and_memory(struct MHD_Connection *conn) {
    general_stat list = get_cpu_stats();
    memory_stats memory = list.memory; 
    cJSON *o = cJSON_CreateObject();
    cJSON *core_percent = cJSON_CreateObject();
    cJSON *memory_stats = cJSON_CreateObject();
    cJSON *swap_stats = cJSON_CreateObject();
    
    for (int i=0; i<list.num_cpus; i++) {
        char key[16];
        snprintf(key, sizeof(key), "cpu%d", i);
        cJSON_AddNumberToObject(core_percent, key, list.cores[i].core_percent);
    }
    cJSON_AddItemToObject(o, "core_percent", core_percent);
   
    cJSON_AddNumberToObject(memory_stats, "total_kb", memory.mem_total_kb);
    int used_mem = memory.mem_total_kb - memory.mem_available_kb;
    cJSON_AddNumberToObject(memory_stats, "used_kb", used_mem);
    cJSON_AddNumberToObject(memory_stats, "free_kb", memory.mem_free_kb);
    cJSON_AddNumberToObject(memory_stats, "cached_kb", memory.cached_kb);
    cJSON_AddNumberToObject(memory_stats, "buffered_kb", memory.buffers_kb);
    cJSON_AddItemToObject(o, "memory_stats", memory_stats);
    
    cJSON_AddNumberToObject(swap_stats, "total_kb", memory.swap_total_kb);
    cJSON_AddNumberToObject(swap_stats, "used_kb", memory.swap_used_kb);
    cJSON_AddNumberToObject(swap_stats, "free_kb", memory.swap_free_kb);
    cJSON_AddItemToObject(o, "swap_stats", swap_stats);
    return send_json_response(conn, o);
}



/**
 * Handle POST /api/processes/{pid}/signal
 */
int handle_signal(struct MHD_Connection *conn, int pid, const char *body) {
    char *cmd = json_get_string(body, "cmd");
    printf("[process_routes] body = %s\n", body);

    if (!cmd) {
        fprintf(stderr, "[process_routes] missing cmd field\n");
        const char *response_str = "Bad Request";
        struct MHD_Response *resp = MHD_create_response_from_buffer(
            strlen(response_str),
            (void *)response_str,
            MHD_RESPMEM_PERSISTENT
        );
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        return ret;
    }

    int signo = -1;
    if (!strcasecmp(cmd, "KILL")   || !strcasecmp(cmd, "SIGKILL")) {
        signo = SIGKILL;
    }
    else if (!strcasecmp(cmd, "TERM") || !strcasecmp(cmd, "SIGTERM")) {
        signo = SIGTERM;
    }
    else if (isdigit((unsigned char)cmd[0])) {
        signo = atoi(cmd);
    }
    free(cmd);

    if (signo <= 0) {
        fprintf(stderr, "[process_routes] invalid cmd → signo=%d\n", signo);
        return MHD_HTTP_BAD_REQUEST;
    }

    printf("[process_routes] kill(pid=%d, sig=%d)\n", pid, signo);
    if (send_signal(pid, signo) == 0) {
        printf("[process_routes] send_signal succeeded\n");
        return MHD_HTTP_NO_CONTENT;
    } else {
        fprintf(stderr, "[process_routes] send_signal failed: %s\n", strerror(errno));
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}
