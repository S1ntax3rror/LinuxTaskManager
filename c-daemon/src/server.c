#include "server.h"
#include "routes.h"
#include <microhttpd.h>
#include <string.h>

static int on_request(void *cls,
                      struct MHD_Connection *conn,
                      const char *url,
                      const char *method,
                      const char *version,
                      const char *upload_data,
                      size_t *upload_data_size,
                      void **con_cls)
{
    ConnContext *ctx = *con_cls;
    if (!ctx) {
        // first call: allocate context
        ctx = calloc(1, sizeof(*ctx));
        *con_cls = ctx;
        return MHD_YES;
    }

    if (*upload_data_size > 0) {
        // accumulate this chunk
        ctx->upload_data = realloc(ctx->upload_data,
                                   ctx->upload_len + *upload_data_size + 1);
        memcpy(ctx->upload_data + ctx->upload_len,
               upload_data, *upload_data_size);
        ctx->upload_len += *upload_data_size;
        ctx->upload_data[ctx->upload_len] = '\0';
        *upload_data_size = 0;
        return MHD_YES;
    }

    // all data received: dispatch
    const char *body = ctx->upload_data ? ctx->upload_data : "";
    int result = route_request(conn, url, method, body);

    // clean up
    free(ctx->upload_data);
    free(ctx);
    *con_cls = NULL;
    return result;
}

int start_http_server(unsigned port) {
    return MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY,
        port,
        NULL, NULL,
        &on_request, NULL,
        MHD_OPTION_END) != NULL;
}
