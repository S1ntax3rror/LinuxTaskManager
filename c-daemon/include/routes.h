#ifndef ROUTES_H
#define ROUTES_H

#include <microhttpd.h>

/* Dispatches URL+method to the correct handler */
int route_request(struct MHD_Connection *conn, const char *url, const char *method);

#endif // ROUTES_H
