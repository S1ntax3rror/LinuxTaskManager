// server.h
#ifndef SERVER_H
#define SERVER_H

#include <microhttpd.h>
#include <cjson/cJSON.h>

/* Start the HTTP daemon on the given port.
 * Returns 0 on failure, nonzero on success. */
int start_http_server(unsigned port);

/* Helper: send a cJSON object as an HTTP 200 JSON response */
int send_json_response(struct MHD_Connection *conn, cJSON *obj);

#endif // SERVER_H
