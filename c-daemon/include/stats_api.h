#ifndef STATS_API_H
#define STATS_API_H

#include <microhttpd.h>

/* Handler for GET /api/stats/cpu
 * Serializes the result of get_cpu_stats() into JSON. */
 int handle_cpu_stats(struct MHD_Connection *conn);

 #endif // STATS_API_H