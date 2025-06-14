#include "../include/header.h"
#include <string.h>

void init_header(header* h) {
    h->latest_index = 0;
    h->count = 0;
    memset(h->entries, 0, sizeof(h->entries));
}

void push_general_stat(header* h, general_stat* new_stat, float down_time_sec) {
    int index = h->latest_index;
    
    h->entries[index] = *new_stat;

    h->latest_index = (index + 1) % MAX_HEADER_ENTRIES;
    if (h->count < MAX_HEADER_ENTRIES) {
        h->count++;
    }

    if (h->count >= 2) {
        float total_delta_write= 0.0;
        float total_delta_read = 0.0;
        float total_disk_read = 0.0;
        float total_disk_write = 0.0;
        int oldest_index = (h->count < MAX_HEADER_ENTRIES) ? 0 : h->latest_index;
        int prev_index = (index-1 + MAX_HEADER_ENTRIES) %MAX_HEADER_ENTRIES;
        int N = (h->count < MAX_HEADER_ENTRIES) ? h->count : MAX_HEADER_ENTRIES;
        float seconds = N * down_time_sec;
        for (int i=0; i < MAX_DISK;i++){
            float delta_write = h->entries[index].disk[i].write_MB - h->entries[oldest_index].disk[i].write_MB;
            float delta_read  = h->entries[index].disk[i].read_MB  - h->entries[oldest_index].disk[1].read_MB;
            total_delta_read += delta_read;
            total_delta_write += delta_write;
            total_disk_read += h->entries[index].disk[i].read_MB;
            total_disk_write += h->entries[index].disk[i].write_MB;
        }
        h->entries[index].avg_disk_write_MB = total_delta_write / seconds;
        h->entries[index].avg_disk_read_MB  = total_delta_read  / seconds;

        h->entries[index].total_disk_write_MB = total_disk_write;
        h->entries[index].total_disk_read_MB  = total_disk_read;
        
        float total_delta_upload   = h->entries[index].net.total_upload_MB   - h->entries[oldest_index].net.total_upload_MB;
        float total_delta_download = h->entries[index].net.total_download_MB - h->entries[oldest_index].net.total_download_MB;
        float delta_upload   = h->entries[index].net.total_upload_MB   - h->entries[prev_index].net.total_upload_MB;
        float delta_download = h->entries[index].net.total_download_MB - h->entries[prev_index].net.total_download_MB;
        
        h->entries[index].network_avg_upload_speed   = total_delta_upload / seconds;
        h->entries[index].network_avg_download_speed = total_delta_download / seconds;
        h->entries[index].network_upload_speed   = delta_upload / down_time_sec;
        h->entries[index].network_download_speed = delta_download / down_time_sec;
    }
}

general_stat* get_latest_stat(header* h) {
    if (h->count == 0) return NULL;
    int index = (h->latest_index - 1 + MAX_HEADER_ENTRIES) % MAX_HEADER_ENTRIES;
    return &h->entries[index];
}