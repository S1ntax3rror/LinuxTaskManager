#ifndef SERVER_H
#define SERVER_H

#include <microhttpd.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

/**
 * Per-connection context to accumulate POST body.
 */
typedef struct {
    char   *upload_data;
    size_t  upload_len;
} ConnContext;

/* Start the HTTP daemon on the given port.
 * Returns nonzero on success, zero on failure. */
int start_http_server(unsigned port);

/* Helper: send a cJSON object as an HTTP 200 JSON response */
int send_json_response(struct MHD_Connection *conn, cJSON *obj);

#endif // SERVER_H
