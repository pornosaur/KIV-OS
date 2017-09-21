#ifndef FAT_BADBLOCK_RECOVERY_H
#define FAT_BADBLOCK_RECOVERY_H

#include <pthread.h>
#include <stdlib.h>

struct treat_directory_struct{
    pthread_mutex_t *file_mutex;
    pthread_mutex_t *fat_mutex;
    struct boot_record *boot_record;
    int32_t *fat;
    long start_position;
};

struct check_unused_clusters_struct{
    struct boot_record *boot_record;
    int32_t *index;
    pthread_mutex_t *file_mutex;
    pthread_mutex_t *fat_mutex;
    int32_t *fat;
};

#endif
