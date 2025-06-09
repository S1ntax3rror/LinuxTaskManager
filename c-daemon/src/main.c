// main.c

#include "../include/server.h"
#include <stdio.h>

/** ---------------------------------------------------------------------
 * defines port & initiates our Server in server.c
 -----------------------------------------------------------------------*/

#define PORT 9000  // Default listening port for the HTTP server on the c-daemon side

// start_http_server() returns non-zero on success
int main(void) {
    if (!start_http_server(PORT)) {
        fprintf(stderr, "ERROR: failed to start HTTP server on port %d\n", PORT);
        return 1;  // Exit with error code
    }

    // success
    printf("C-daemon listening on http://localhost:%d/ (Enter to quit)\n", PORT);
    getchar();  // Wait for user to press Enter before shutting down

    return 0;  // Clean exit
}
