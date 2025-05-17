#include "limit_routes.h"
#include "core_interface.h"
#include "server.h"          // send_json_response()
#include <cjson/cJSON.h>
#include <stdlib.h>

static long json_get_long(const char *body, const char *field) {
    cJSON *req = cJSON_Parse(body);
    if (!req) return -1;
    cJSON *item = cJSON_GetObjectItem(req, field);
    long val = (item && cJSON_IsNumber(item)) ? item->valuedouble : -1;
    cJSON_Delete(req);
    return val;
}

int handle_renice(struct MHD_Connection *conn, int pid, const char *body) {
    long nice_val = json_get_long(body, "nice");
    if (nice_val < -20 || nice_val > 19) {
        // invalid nice
        return MHD_HTTP_BAD_REQUEST;
    }
    if (renice_process(pid, (int)nice_val) == 0) {
        return MHD_HTTP_NO_CONTENT;
    } else {
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}

int handle_cpu_limit(struct MHD_Connection *conn, int pid, const char *body) {
    long secs = json_get_long(body, "limit");
    if (secs < 0) return MHD_HTTP_BAD_REQUEST;
    if (set_cpu_limit(pid, (rlim_t)secs) == 0) {
        return MHD_HTTP_NO_CONTENT;
    } else {
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}

int handle_ram_limit(struct MHD_Connection *conn, int pid, const char *body) {
    long mb = json_get_long(body, "limit");
    if (mb < 0) return MHD_HTTP_BAD_REQUEST;
    rlim_t bytes = (rlim_t)mb * 1024 * 1024;
    if (set_ram_limit(pid, bytes) == 0) {
        return MHD_HTTP_NO_CONTENT;
    } else {
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}
