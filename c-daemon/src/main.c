// main.c
#include "../include/server.h"
#include <stdio.h>

#define PORT 9000

int main(void) {
    if (!start_http_server(PORT)) {
        fprintf(stderr, "ERROR: failed to start HTTP server on port %d\n", PORT);
        return 1;
    }
    printf("C-daemon listening on http://localhost:%d/ (Enter to quit)\n", PORT);
    getchar();
    return 0;
}
