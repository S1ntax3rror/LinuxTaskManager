#ifndef STATS_ROUTES_H
#define STATS_ROUTES_H

#include <microhttpd.h>

/* Dispatches CPU‐stats endpoint.
 * Returns 1 if it handled the request, 0 otherwise. */
int dispatch_stats_routes(struct MHD_Connection *conn,
                          const char *url,
                          const char *method);

#endif // STATS_ROUTES_H
