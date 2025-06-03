#ifndef LIMIT_ROUTES_H
#define LIMIT_ROUTES_H

#include <microhttpd.h>

/**
 * Read JSON body, pull out integer “nice” field, and call renice_process().
 * Returns MHD_HTTP_NO_CONTENT on success, otherwise 500.
 */
int handle_renice(struct MHD_Connection *conn,
                  int pid,
                  const char *body);

/**
 * Read JSON body, pull out integer “limit” field (seconds),
 * then call set_cpu_limit().
 */
int handle_cpu_limit(struct MHD_Connection *conn,
                     int pid,
                     const char *body);

/**
 * Read JSON body, pull out integer “limit” field (MB),
 * convert to bytes, then call set_ram_limit().
 */
int handle_ram_limit(struct MHD_Connection *conn,
                     int pid,
                     const char *body);

int handle_test_post(struct MHD_Connection *conn,
                     const char *body);

#endif // LIMIT_ROUTES_H
