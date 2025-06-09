#include "../include/routes.h"
#include "../include/process_routes.h"   // process list & signal handlers
#include "../include/limit_routes.h"     // renice & resource limit handlers
#include "../include/stats_routes.h"     // CPU/memory/disk/network stats handlers
#include "../include/server.h"
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

/** ---------------------------------------------------------------------
 * route_request: Top-level dispatcher for all incoming HTTP requests.
 -----------------------------------------------------------------------*/

/**
 * - conn:    the MHD connection object
 * - url:     requested URL path
 * - method:  HTTP method ("GET", "POST", etc.)
 * - body:    full request body, if any (else empty string)
 *
 * Returns an MHD_HTTP_* status code indicating the result.
 */

int route_request(struct MHD_Connection *conn,
                  const char *url,
                  const char *method,
                  const char *body)
{
    // Log each incoming request URL for debugging
    printf("Received request for URL: %s\n", url);

    // ── GET /api/processes ────────────────────────────────────────────────
    // Return the full list of processes in JSON
    if (!strcmp(method, "GET") &&
        !strcmp(url, "/api/processes") &&
        strstr(url, "/api/processes"))
    {
        return handle_process_list(conn);
    }

    // ── GET /api/cpu_mem ─────────────────────────────────────────────────
    // Return per-core CPU usage + memory and swap stats
    if (!strcmp(method, "GET") &&
        !strcmp(url, "/api/cpu_mem") &&
        strstr(url, "/api/cpu_mem"))
    {
        return handle_cores_and_memory(conn);
    }

    // ── POST /api/processes/{pid}/signal ─────────────────────────────────
    // Send a signal (KILL/TERM/custom) to a process
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/signal", &pid) == 1 &&
            strstr(url, "/signal"))
        {
            return handle_signal(conn, pid, body);
        }
    }

    // ── POST /api/processes/{pid}/renice ─────────────────────────────────
    // Change the nice value of a process
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/renice", &pid) == 1 &&
            strstr(url, "/renice"))
        {
            return handle_renice(conn, pid, body);
        }
    }

    // ── POST /api/processes/test_post ────────────────────────────────────
    // Simple test endpoint to verify POST handling
    {
        if (!strcmp(method, "POST") &&
            strstr(url, "/test_post"))
        {
            return handle_test_post(conn, body);
        }
    }

    // ── POST /api/processes/{pid}/limit/cpu ──────────────────────────────
    // Limit CPU time for a process
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/limit/cpu", &pid) == 1 &&
            strstr(url, "/limit/cpu"))
        {
            return handle_cpu_limit(conn, pid, body);
        }
    }

    // ── POST /api/processes/{pid}/limit/ram ──────────────────────────────
    // Limit RAM usage for a process
    {
        int pid;
        if (!strcmp(method, "POST") &&
            sscanf(url, "/api/processes/%d/limit/ram", &pid) == 1 &&
            strstr(url, "/limit/ram"))
        {
            return handle_ram_limit(conn, pid, body);
        }
    }

    // ── GET /api/stats/* ─────────────────────────────────────────────────
    // Let stats_routes decide if this URL matches one of its endpoints
    if (dispatch_stats_routes(conn, url, method)) {
        return MHD_HTTP_OK;
    }

    // ── Not Found ────────────────────────────────────────────────────────
    // If no handler matched, return a 404 response
    const char *not_found = "Not Found";
    struct MHD_Response *resp =
        MHD_create_response_from_buffer(strlen(not_found),
                                        (void*)not_found,
                                        MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(resp, "Content-Type", "text/plain");
    int rc = MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, resp);
    MHD_destroy_response(resp);
    return rc;
}
