#include "process_routes.h"
#include "core_interface.h"
#include "server.h"
#include <cjson/cJSON.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Helper: extract a string field from JSON, caller must free() */
static char *json_get_string(const char *body, const char *field) {
    cJSON *root = cJSON_Parse(body);
    if (!root) return NULL;
    cJSON *item = cJSON_GetObjectItem(root, field);
    char *out = NULL;
    if (item && cJSON_IsString(item))
        out = strdup(item->valuestring);
    cJSON_Delete(root);
    return out;
}

int handle_process_list(struct MHD_Connection *conn) {
    int count;
    trimmed_info *list = get_process_list(&count);
    printf("[process_routes] fetched %d processes\n", count);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; ++i) {
        cJSON *o = cJSON_CreateObject();
        cJSON_AddNumberToObject(o, "pid",        list[i].pid);
        cJSON_AddStringToObject(o, "comm",       list[i].comm);
        char s[2] = { list[i].state, '\0' };
        cJSON_AddStringToObject(o, "state",      s);
        cJSON_AddNumberToObject(o, "cpuPercent", list[i].cpu_percent);
        cJSON_AddNumberToObject(o, "ramPercent", list[i].ram_percent);
        cJSON_AddNumberToObject(o, "nice",       list[i].nice);
        cJSON_AddItemToArray(arr, o);
    }
    free(list);
    return send_json_response(conn, arr);
}

int handle_signal(struct MHD_Connection *conn, int pid, const char *body) {
    char *cmd = json_get_string(body, "cmd");
    if (!cmd) {
        fprintf(stderr, "[process_routes] missing cmd field\n");
        return MHD_HTTP_BAD_REQUEST;
    }

    int signo = -1;
    if (!strcasecmp(cmd, "KILL") || !strcasecmp(cmd, "SIGKILL")) {
        signo = SIGKILL;
    } else if (!strcasecmp(cmd, "TERM") || !strcasecmp(cmd, "SIGTERM")) {
        signo = SIGTERM;
    } else if (isdigit((unsigned char)cmd[0])) {
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
        fprintf(stderr,
                "[process_routes] send_signal failed: %s\n",
                strerror(errno));
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}
