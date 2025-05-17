// c-daemon/src/routes.c

#include "routes.h"
#include "process_routes.h"  // handle_process_list(), handle_signal()
#include "limit_routes.h"    // handle_renice(), handle_cpu_limit(), handle_ram_limit()
#include "stats_routes.h"    // dispatch_stats_routes()
#include "server.h"
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

int route_request(struct MHD_Connection *conn,
                  const char       *url,
                  const char       *method,
                  const char       *body)
{
    // GET /api/processes
    if (!strcmp(method, "GET") && !strcmp(url, "/api/processes")) {
        return handle_process_list(conn);
    }

    // POST /api/processes/{pid}/signal
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/signal", &pid) == 1)
        {
            return handle_signal(conn, pid, body);
        }
    }

    // POST /api/processes/{pid}/renice
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/renice", &pid) == 1)
        {
            return handle_renice(conn, pid, body);
        }
    }

    // POST /api/processes/{pid}/limit/cpu
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/limit/cpu", &pid) == 1)
        {
            return handle_cpu_limit(conn, pid, body);
        }
    }

    // POST /api/processes/{pid}/limit/ram
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/limit/ram", &pid) == 1)
        {
            return handle_ram_limit(conn, pid, body);
        }
    }

    // GET /api/stats/cpu
    if (dispatch_stats_routes(conn, url, method)) {
        // dispatch_stats_routes already sent the 200 response
        return MHD_HTTP_OK;
    }

    // Not found → 404
    const char *notf = "Not Found";
    struct MHD_Response *resp =
      MHD_create_response_from_buffer(strlen(notf),
                                      (void*)notf,
                                      MHD_RESPMEM_PERSISTENT);
    int rc = MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, resp);
    MHD_destroy_response(resp);
    return rc;
}
