// server.c
#include "server.h"
#include "routes.h"

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
    (void)cls; (void)version; (void)upload_data; (void)upload_data_size; (void)con_cls;
    return route_request(conn, url, method);
}

int start_http_server(unsigned port) {
    return MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY,
        port,
        NULL, NULL,
        &on_request, NULL,
        MHD_OPTION_END) != NULL;
}

int send_json_response(struct MHD_Connection *conn, cJSON *obj) {
    char *body = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);

    struct MHD_Response *resp = MHD_create_response_from_buffer(
        strlen(body), (void*)body, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(resp, "Content-Type", "application/json");
    int ret = MHD_queue_response(conn, MHD_HTTP_OK, resp);
    MHD_destroy_response(resp);
    return ret;
}
