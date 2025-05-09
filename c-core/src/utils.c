#include "../include/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>


int is_number(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}


int count_folders(const char *path) {
    DIR *dp = opendir(path);
    struct dirent *entry;
    int count = 0;

    if (!dp) return -1;

    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                count++;
        }
    }
    closedir(dp);
    return count;
}