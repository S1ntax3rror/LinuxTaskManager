// process_api.c
#include "../c-daemon/include/process_api.h"
#include "../c-daemon/include/core_interface.h"
#include "../c-daemon/include/server.h"

#include <cjson/cJSON.h>
#include <stdlib.h>

int handle_process_list(struct MHD_Connection *conn) {
    int count = 0;
    trimmed_info *list = get_process_list(&count);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; ++i) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "pid",             list[i].pid);
        cJSON_AddStringToObject(obj, "comm",            list[i].comm);
        char state_str[2] = { list[i].state, '\0' };
        cJSON_AddStringToObject(obj, "state",           state_str);
        cJSON_AddNumberToObject(obj, "cpuPercent",      list[i].cpu_percent);
        cJSON_AddNumberToObject(obj, "ramPercent",      list[i].ram_percent);
        cJSON_AddNumberToObject(obj, "timestampMs",     list[i].timestamp_ms);
        cJSON_AddNumberToObject(obj, "avgCpuPercent",   list[i].avg_cpu_percent);
        cJSON_AddNumberToObject(obj, "peakRamPercent",  list[i].peak_ram_percent);
        cJSON_AddItemToArray(arr, obj);
    }
    free(list);

    return send_json_response(conn, arr);
}
