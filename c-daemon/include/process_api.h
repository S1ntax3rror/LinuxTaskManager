#ifndef PROCESS_API_H
#define PROCESS_API_H

#include <microhttpd.h>

/* Handler for GET /api/processes */
int handle_process_list(struct MHD_Connection *conn);

#endif // PROCESS_API_H
