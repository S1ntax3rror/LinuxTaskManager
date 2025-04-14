#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <ctype.h>

int is_number(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(*str)) return 0;
    }
    return 1;
}

int main() {
    struct dirent *entry;
    printf("running Systemproclist\n");

    DIR *dp = opendir("/proc");
    if (dp == NULL) {
        printf("/proc open error");
        return 1;
    }
    while ((entry = readdir(dp))) {
        if (is_number(entry->d_name)) {
            char path[256];
            snprintf(path,sizeof(path), "/proc/%s/comm",entry->d_name);

            FILE *fp = fopen(path, "r"); 
            if (fp) {
                char name[256];
                if (fgets(name, sizeof(name), fp)) {
                    printf("PID: %s\tName: %s", entry->d_name, name);
                }
                fclose(fp);
            }

        }
    }
closedir(dp);
return 0;
}
