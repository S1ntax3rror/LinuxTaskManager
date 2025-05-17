#include "core_interface.h"
#include "process_routes.h"
#include "server.h"
#include <cjson/cJSON.h>
#include <stdlib.h>

int handle_process_list(struct MHD_Connection *conn) {
    int count;
    trimmed_info *list = get_process_list(&count);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; ++i) {
        cJSON *o = cJSON_CreateObject();
        cJSON_AddNumberToObject(o, "pid",           list[i].pid);
        cJSON_AddStringToObject(o, "comm",          list[i].comm);
        char s[2] = { list[i].state, '\0' };
        cJSON_AddStringToObject(o, "state",         s);
        cJSON_AddNumberToObject(o, "cpuPercent",    list[i].cpu_percent);
        cJSON_AddNumberToObject(o, "ramPercent",    list[i].ram_percent);
        cJSON_AddNumberToObject(o, "nice",          list[i].avg_cpu_percent); // placeholder
        // actually avg_cpu_percent is different—replace with a real field if desired
        cJSON_AddItemToArray(arr, o);
    }
    free(list);
    return send_json_response(conn, arr);
}
