#include "../include/limit_routes.h"
#include "../include/core_interface.h"
#include "../include/server.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/**
 * Parse a JSON integer field out of the POST body.
 */
static long limit_routes_json_get_long(const char *body, const char *field) {
    cJSON *req = cJSON_Parse(body);
    if (!req) return -1;
    cJSON *item = cJSON_GetObjectItem(req, field);
    long val = (item && cJSON_IsNumber(item)) ? item->valuedouble : -1;
    cJSON_Delete(req);
    return val;
}

int handle_renice(struct MHD_Connection *conn, int pid, const char *body) {
    long nice_val = limit_routes_json_get_long(body, "nice");
    printf("[limit_routes] renice pid=%d to nice=%ld\n", pid, nice_val);
    if (nice_val < -20 || nice_val > 19) {
        fprintf(stderr, "[limit_routes] bad nice %ld\n", nice_val);

        const char *response_str = "Bad nice value";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void *)response_str,
                                                                    MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");

        int ret = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        return ret;
        //return MHD_HTTP_BAD_REQUEST;
    }
    if (renice_process(pid, (int)nice_val) == 0) {
        printf("[limit_routes] renice succeeded\n");

        const char *response_str = "Created";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void *)response_str,
                                                                    MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");

        int ret = MHD_queue_response(conn, MHD_HTTP_CREATED, resp);
        MHD_destroy_response(resp);
        return ret;
        //return MHD_HTTP_NO_CONTENT;
    } else {
        fprintf(stderr,
                "[limit_routes] renice_process failed: %s\n",
                strerror(errno));
        const char *response_str = "Renice failed (internal server error)";
        struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void *)response_str,
                                                                    MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");

        int ret = MHD_queue_response(conn, MHD_HTTP_INTERNAL_SERVER_ERROR, resp);
        MHD_destroy_response(resp);
        return ret;
        //return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}

int handle_test_post(struct MHD_Connection *conn, const char *body) {
    printf("test successfully entered handle\n");
    
    // TERMINATE CONNECTION AFTER ONE PING
    const char *response_str = "Created";
    struct MHD_Response *resp = MHD_create_response_from_buffer(strlen(response_str),
                                                                (void *)response_str,
                                                                MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(resp, "Connection", "close");

    int ret = MHD_queue_response(conn, MHD_HTTP_CREATED, resp);
    MHD_destroy_response(resp);
    return ret;
}

int handle_cpu_limit(struct MHD_Connection *conn, int pid, const char *body) {
    long secs = limit_routes_json_get_long(body, "limit");
    printf("[limit_routes] cpu‐limit pid=%d to %lds\n", pid, secs);
    if (secs < 0) {
        fprintf(stderr, "[limit_routes] bad cpu limit %ld\n", secs);
        return MHD_HTTP_BAD_REQUEST;
    }
    if (set_cpu_limit(pid, (rlim_t)secs) == 0) {
        printf("[limit_routes] set_cpu_limit succeeded\n");
        return MHD_HTTP_NO_CONTENT;
    } else {
        fprintf(stderr,
                "[limit_routes] set_cpu_limit failed: %s\n",
                strerror(errno));
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}

int handle_ram_limit(struct MHD_Connection *conn, int pid, const char *body) {
    long mb = limit_routes_json_get_long(body, "limit");
    printf("[limit_routes] ram‐limit pid=%d to %ldMB\n", pid, mb);
    if (mb < 0) {
        fprintf(stderr, "[limit_routes] bad ram limit %ld\n", mb);
        return MHD_HTTP_BAD_REQUEST;
    }
    rlim_t bytes = (rlim_t)mb * 1024 * 1024;
    if (set_ram_limit(pid, bytes) == 0) {
        printf("[limit_routes] set_ram_limit succeeded\n");
        return MHD_HTTP_NO_CONTENT;
    } else {
        fprintf(stderr,
                "[limit_routes] set_ram_limit failed: %s\n",
                strerror(errno));
        return MHD_HTTP_INTERNAL_SERVER_ERROR;
    }
}
