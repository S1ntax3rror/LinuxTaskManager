// stats_api.c
#include "../c-daemon/include/stats_api.h"
#include "../c-daemon/include/core_interface.h"
#include "../c-daemon/include/server.h"

#include <cjson/cJSON.h>

int handle_cpu_stats(struct MHD_Connection *conn) {
    general_stat gs = get_cpu_stats();

    cJSON *root = cJSON_CreateObject();
    cJSON *cpu  = cJSON_CreateObject();

    cJSON_AddStringToObject(cpu,  "name",       gs.cpu.name);
    cJSON_AddNumberToObject(cpu,  "nice",       gs.cpu.nice);
    cJSON_AddNumberToObject(cpu,  "system",     gs.cpu.system);
    cJSON_AddNumberToObject(cpu,  "idle",       gs.cpu.idle);
    cJSON_AddNumberToObject(cpu,  "iowait",     gs.cpu.iowait);
    cJSON_AddNumberToObject(cpu,  "irq",        gs.cpu.irq);
    cJSON_AddNumberToObject(cpu,  "softirq",    gs.cpu.softirq);
    cJSON_AddNumberToObject(cpu,  "steal",      gs.cpu.steal);
    cJSON_AddNumberToObject(cpu,  "guest",      gs.cpu.guest);
    cJSON_AddNumberToObject(cpu,  "guest_nice", gs.cpu.guest_nice);
    cJSON_AddItemToObject(root, "cpu", cpu);

    cJSON *cores = cJSON_CreateArray();
    for (int i = 0; i < gs.num_cpus; ++i) {
        cpu_stats *c = &gs.cores[i];
        cJSON *cobj = cJSON_CreateObject();
        cJSON_AddStringToObject(cobj, "name", c->name);
        cJSON_AddNumberToObject(cobj, "idle", c->idle);
        cJSON_AddItemToArray(cores, cobj);
    }
    cJSON_AddItemToObject(root, "cores", cores);

    cJSON_AddNumberToObject(root, "intr",         gs.intr_0);
    cJSON_AddNumberToObject(root, "ctxt",         gs.ctxt);
    cJSON_AddNumberToObject(root, "btime",        gs.btime);
    cJSON_AddNumberToObject(root, "processes",    gs.processes);
    cJSON_AddNumberToObject(root, "procs_running",gs.procs_running);
    cJSON_AddNumberToObject(root, "procs_blocked",gs.procs_blocked);
    cJSON_AddNumberToObject(root, "num_cpus",      gs.num_cpus);

    return send_json_response(conn, root);
}
