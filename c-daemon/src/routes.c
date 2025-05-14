// routes.c
#include "routes.h"
#include "process_api.h"
#include "stats_api.h"

#include <string.h>
#include <microhttpd.h>

int route_request(struct MHD_Connection *conn, const char *url, const char *method) {
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/api/processes") == 0)
            return handle_process_list(conn);
        if (strcmp(url, "/api/stats/cpu") == 0)
            return handle_cpu_stats(conn);
    }
    const char *msg = "Not Found";
    struct MHD_Response *r = MHD_create_response_from_buffer(
        strlen(msg), (void*)msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, r);
    MHD_destroy_response(r);
    return ret;
}
