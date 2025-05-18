#include "server.h"
#include "routes.h"
#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>

/* we get ConnContext from include/server.h, so no redefinition here */

static enum MHD_Result
on_request(void *cls,
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
        ctx = calloc(1, sizeof(*ctx));
        *con_cls = ctx;
        return MHD_YES;
    }

    if (*upload_data_size > 0) {
        ctx->upload_data = realloc(ctx->upload_data,
                                   ctx->upload_len + *upload_data_size + 1);
        memcpy(ctx->upload_data + ctx->upload_len,
               upload_data, *upload_data_size);
        ctx->upload_len += *upload_data_size;
        ctx->upload_data[ctx->upload_len] = '\0';
        *upload_data_size = 0;
        return MHD_YES;
    }

    /* dispatch once body is all in */
    const char *body = ctx->upload_data ? ctx->upload_data : "";
    int result = route_request(conn, url, method, body);

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
        on_request, NULL,
        MHD_OPTION_END
    ) != NULL;
}

int send_json_response(struct MHD_Connection *conn, cJSON *obj) {
    char *s = cJSON_PrintUnformatted(obj);
    struct MHD_Response *resp =
      MHD_create_response_from_buffer(strlen(s),
                                      (void*)s,
                                      MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(resp, "Content-Type", "application/json");
    int rc = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    cJSON_Delete(obj);
    return rc;
}
