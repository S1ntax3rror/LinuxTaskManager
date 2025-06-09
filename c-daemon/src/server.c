#include "../include/server.h"   // definitions for server API and ConnContext
#include "../include/routes.h"   // route_request dispatcher
#include <microhttpd.h>            // HTTP daemon library
#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>            // JSON serialization (used in send_json_response)

/** ---------------------------------------------------------------------
 * Starts the HTTP daemon, gathers each request’s body, hands it off to 
 * route_request(), and provides a helper to send back JSON responses
 -----------------------------------------------------------------------*/

/*
 * on_request: callback for each HTTP transaction.
 * - cls:   unused user-supplied pointer
 * - conn:  the connection handle
 * - url:   request path
 * - method: HTTP verb ("GET", "POST", etc.)
 * - version: HTTP version string
 * - upload_data: chunk of POST body
 * - upload_data_size: size of this chunk
 * - con_cls: pointer to our per-connection context pointer.
 */
static enum MHD_Result
on_request(void *cls,
           struct MHD_Connection *conn,
           const char *url,
           const char *method,
           const char *version,
           const char *upload_data,
           size_t *upload_data_size,
           void **con_cls)
{
    ConnContext *ctx = *con_cls;

    // First call for this connection: allocate our context
    if (!ctx) {
        ctx = calloc(1, sizeof(*ctx));  // zeroes upload_data and upload_len
        *con_cls = ctx;
        return MHD_YES;                 // Tell libmicrohttpd we want more data
    }

    // If there's still upload data arriving, append it
    if (*upload_data_size > 0) {
        // Expand buffer to hold new chunk + null terminator
        ctx->upload_data = realloc(ctx->upload_data,
                                   ctx->upload_len + *upload_data_size + 1);
        memcpy(ctx->upload_data + ctx->upload_len,
               upload_data,
               *upload_data_size);
        ctx->upload_len += *upload_data_size;
        ctx->upload_data[ctx->upload_len] = '\0';  // null-terminate
        *upload_data_size = 0;  // tell libmicrohttpd it's consumed
        return MHD_YES;         // request more data
    }

    // All upload data collected: dispatch to router
    const char *body = ctx->upload_data ? ctx->upload_data : "";
    int result = route_request(conn, url, method, body);

    // Clean up context now that request is handled
    free(ctx->upload_data);
    free(ctx);
    *con_cls = NULL;
    return result;
}

/**
 * start_http_server: initializes and starts the HTTP daemon.
 * - port: TCP port to listen on
 *
 * Returns non-zero on success, zero on failure.
 */
int start_http_server(unsigned port) {
    return MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, // use built-in select() loop
        port,                      // listening TCP port
        NULL, NULL,                // accept policy callbacks (none)
        on_request, NULL,          // request handler callback
        MHD_OPTION_END             // end of options
    ) != NULL;
}

/**
 * send_json_response: helper to send a cJSON object as HTTP 200 JSON.
 * - conn: microhttpd connection
 * - obj:  cJSON object to serialize (will be deleted)
 *
 * Constructs a JSON string, wraps in HTTP response, sets Content-Type,
 * and queues it. Frees cJSON and internal string.
 */
int send_json_response(struct MHD_Connection *conn, cJSON *obj) {
    // Serialize JSON object (no pretty-print)
    char *s = cJSON_PrintUnformatted(obj);
    // Create HTTP response from buffer; server will free 's'
    struct MHD_Response *resp =
      MHD_create_response_from_buffer(strlen(s),
                                      (void*)s,
                                      MHD_RESPMEM_MUST_FREE);
    // Set header for JSON MIME type
    MHD_add_response_header(resp, "Content-Type", "application/json");
    // Queue response with status 200
    int rc = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    cJSON_Delete(obj);  // free the cJSON tree
    return rc;
}
