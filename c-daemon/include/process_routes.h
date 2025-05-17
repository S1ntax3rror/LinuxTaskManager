#ifndef PROCESS_ROUTES_H
#define PROCESS_ROUTES_H

#include <microhttpd.h>

int  handle_process_list(struct MHD_Connection *conn);
int  handle_signal      (struct MHD_Connection *conn,
                         int pid,
                         const char *body);
/* Returns nonzero if this module handled the URL+method */
int dispatch_process_routes(struct MHD_Connection *conn,
                            const char *url, const char *method);

#endif // PROCESS_ROUTES_H
