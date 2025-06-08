#include "../include/routes.h"
#include "../include/process_routes.h"   // handle_process_list, handle_signal
#include "../include/limit_routes.h"     // handle_renice, handle_cpu_limit, handle_ram_limit
#include "../include/stats_routes.h"     // dispatch_stats_routes
#include "../include/server.h"
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

int route_request(struct MHD_Connection *conn,
                  const char *url,
                  const char *method,
                  const char *body)
{
    printf("%s\n", url);
    //int pid;
    //printf("WE WANT TO KILL %i\n", sscanf(url, "/api/processes/%d/signal", &pid));
    //printf("signal in url: %i\n", strstr(url, "/signal"));
    //printf("renice in url: %i\n", strstr(url, "/renice"));
    //printf("other in url: %i\n", strstr(url, "/other"));
    
    // GET /api/processes
    if (!strcmp(method, "GET") && !strcmp(url, "/api/processes") && strstr(url, "/api/processes"))
        return handle_process_list(conn);

    if (!strcmp(method, "GET") && !strcmp(url, "/api/cpu_mem") && strstr(url, "/api/cpu_mem"))
        return handle_cores_and_memory(conn);

    // POST /api/processes/{pid}/signal
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/signal", &pid) == 1 &&
            strstr(url, "/signal"))
        {
            return handle_signal(conn, pid, body);
        }
    }

    // POST /api/processes/{pid}/renice
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/renice", &pid) == 1 &&
            strstr(url, "/renice"))
        {
            return handle_renice(conn, pid, body);
        }
    }

    // POST /api/processes/test_post
    {
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/test_post") == 0 &&
            strstr(url, "/test_post"))
        {
            return handle_test_post(conn, body);
        }
    }

    // POST /api/processes/{pid}/limit/cpu
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/limit/cpu", &pid) == 1 &&
            strstr(url, "/limit/cpu"))
        {
            return handle_cpu_limit(conn, pid, body);
        }
    }

    // POST /api/processes/{pid}/limit/ram
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/limit/ram", &pid) == 1 &&
            strstr(url, "/limit/ram"))
        {
            return handle_ram_limit(conn, pid, body);
        }
    }

    // GET /api/stats/cpu
    if (dispatch_stats_routes(conn, url, method))
        return MHD_HTTP_OK;

    // Not found
    const char *notf = "Not Found";
    struct MHD_Response *resp =
      MHD_create_response_from_buffer(strlen(notf),
                                      (void*)notf,
                                      MHD_RESPMEM_PERSISTENT);
    int rc = MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, resp);
    MHD_destroy_response(resp);
    return rc;
}
