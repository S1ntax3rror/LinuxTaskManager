#include "../include/limit_routes.h"
#include "../include/core_interface.h"
#include "../include/server.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/** ---------------------------------------------------------------------
 * Helps to enforce the limits on Linux - Renicing, CPU and RAM Limits
 -----------------------------------------------------------------------*/

/**
 * Parse a JSON integer field out of the POST body.
 * @param body  Raw JSON payload as string
 * @param field Name of the integer field to extract
 * @return Parsed long value or -1 on error
 */
static long limit_routes_json_get_long(const char *body, const char *field) {
    cJSON *req = cJSON_Parse(body);
    if (!req) {
        return -1;  // JSON parsing failed
    }
    
    cJSON *item = cJSON_GetObjectItem(req, field);
    long val = (item && cJSON_IsNumber(item))
        ? (long)item->valuedouble    // valid number, cast to long
        : -1;                       // missing/invalid field
    
    cJSON_Delete(req);
    return val;
}

/**
 * Handle POST /api/processes/{pid}/renice
 * Reads "nice" from JSON body and calls renice_process()
 */
int handle_renice(struct MHD_Connection *conn, int pid, const char *body) {
    long nice_val = limit_routes_json_get_long(body, "nice");
    printf("[limit_routes] renice pid=%d to nice=%ld\n", pid, nice_val);

    // Validate nice range
    if (nice_val < -20 || nice_val > 19) {
        const char *resp_str = "Bad nice value";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        return ret;
    }

    // Attempt to change priority
    if (renice_process(pid, (int)nice_val) == 0) {
        const char *resp_str = "Renice succeeded";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_CREATED, resp);
        MHD_destroy_response(resp);
        return ret;
    } else {
        // System error while renicing
        const char *resp_str = "Renice failed (internal error)";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_INTERNAL_SERVER_ERROR, resp);
        MHD_destroy_response(resp);
        return ret;
    }
}

/**
 * Simple test POST handler to verify connectivity.
 */
int handle_test_post(struct MHD_Connection *conn, const char *body) {
    printf("[limit_routes] test endpoint hit\n");
    const char *resp_str = "Test OK";
    struct MHD_Response *resp =
        MHD_create_response_from_buffer(strlen(resp_str),
                                        (void*)resp_str,
                                        MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(resp, "Connection", "close");
    int ret = MHD_queue_response(conn, MHD_HTTP_CREATED, resp);
    MHD_destroy_response(resp);
    return ret;
}

/**
 * Handle POST /api/processes/{pid}/limit/cpu
 * Reads "limit" seconds from JSON and sets CPU time limit
 */
int handle_cpu_limit(struct MHD_Connection *conn, int pid, const char *body) {
    long secs = limit_routes_json_get_long(body, "limit");
    printf("[limit_routes] cpu limit pid=%d to %lds\n", pid, secs);

    // Validate non-negative
    if (secs < 0) {
        const char *resp_str = "Bad cpu limit";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        return ret;
    }

    // Apply CPU limit
    if (set_cpu_limit(pid, (rlim_t)secs) == 0) {
        const char *resp_str = "CPU limit set";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(conn, MHD_HTTP_NO_CONTENT, resp);
        MHD_destroy_response(resp);
        return ret;
    } else {
        const char *resp_str = "set_cpu_limit failed";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_INTERNAL_SERVER_ERROR, resp);
        MHD_destroy_response(resp);
        return ret;
    }
}

/**
 * Handle POST /api/processes/{pid}/limit/ram
 * Reads "limit" MB from JSON and sets RAM (address-space) limit
 */
int handle_ram_limit(struct MHD_Connection *conn, int pid, const char *body) {
    long mb = limit_routes_json_get_long(body, "limit");
    printf("[limit_routes] ram limit pid=%d to %ldMB\n", pid, mb);

    if (mb < 0) {
        const char *resp_str = "Bad ram limit";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, resp);
        MHD_destroy_response(resp);
        return ret;
    }

    // Convert MB to bytes and apply
    rlim_t bytes = (rlim_t)mb * 1024 * 1024;
    if (set_ram_limit(pid, bytes) == 0) {
        const char *resp_str = "RAM limit set";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(conn, MHD_HTTP_NO_CONTENT, resp);
        MHD_destroy_response(resp);
        return ret;
    } else {
        const char *resp_str = "set_ram_limit failed";
        struct MHD_Response *resp =
            MHD_create_response_from_buffer(strlen(resp_str),
                                            (void*)resp_str,
                                            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Connection", "close");
        int ret = MHD_queue_response(conn, MHD_HTTP_INTERNAL_SERVER_ERROR, resp);
        MHD_destroy_response(resp);
        return ret;
    }
}
