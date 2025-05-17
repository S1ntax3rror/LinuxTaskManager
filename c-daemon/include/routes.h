#ifndef ROUTES_H
#define ROUTES_H

#include <microhttpd.h>

/**
 * Top‐level dispatcher.  ‘body’ is the entire POST payload (or "" if none).
 * Returns an MHD_HTTP_* status (200, 204, 404, 500, etc.).
 */
int route_request(struct MHD_Connection *conn,
                  const char *url,
                  const char *method,
                  const char *body);

#endif // ROUTES_H
