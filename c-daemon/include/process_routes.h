#ifndef PROCESS_ROUTES_H
#define PROCESS_ROUTES_H

#include <microhttpd.h>

/* Return 200+JSON list of processes */
int  handle_process_list(struct MHD_Connection *conn);

/* POST /api/processes/{pid}/signal
 * body: {"cmd":"KILL"} or {"cmd":"TERM"} or {"cmd":"9"}
 */
int  handle_signal      (struct MHD_Connection *conn,
                         int pid,
                         const char *body);

#endif // PROCESS_ROUTES_H
