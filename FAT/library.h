#ifndef FAT_LIBRARY_H
#define FAT_LIBRARY_H

#include <stdlib.h>
#include "fat_structure.h"

struct boot_record *get_boot_record(FILE *p_file);

int32_t *get_fat(FILE *p_file, uint fat_record_size, int cluster_count, uint fat_size);

void delete_cluster(FILE *p_file, uint start_of_cluster, int16_t cluster_size);

int *get_file_clusters(struct dir_file *file, int32_t *clusters_size, int16_t cluster_size, int32_t *fat, int16_t dir_clusters);

struct dir_file *find_file(FILE *p_file, struct boot_record *boot_record, char file_path[], uint start_of_root_dir, uint start_of_data, uint max_dir_entries);

struct dir_file *get_object_in_dir(FILE * p_file, char name[], int32_t start_position, uint max_entries);

struct dir_file *get_all_in_dir(FILE *p_file, int32_t *number_of_objects, long * positions, long start_position, uint max_entries);

int is_dir_empty(FILE *p_file, int32_t max_entries, int32_t start_of_dir);

long find_empty_space_in_dir(FILE *p_file, int32_t max_entries, int32_t start_of_dir);

int write_to_dir(FILE *p_file, struct dir_file file, int32_t write_position);

int write_file_to_fat(FILE *fat_file, FILE *file, int32_t *clusters, int32_t clusters_size, uint start_of_data, int16_t cluster_size);

int write_empty_dir_to_fat(FILE *fat_file, int32_t *clusters, int32_t clusters_size, uint start_of_data, int16_t cluster_size);

int find_empty_clusters(int32_t usable_cluster_count, int32_t *fat, int32_t *clusters, long number_of_clusters);

int rm_from_fat(int32_t *fat, int32_t *indexes, int32_t number_of_clusters);

int rm_from_all_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes, int32_t number_of_clusters, uint fat_size, int8_t number_of_fat);

int rm_from_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes, int32_t number_of_clusters);

int change_fat(int32_t *fat, int32_t *indexes, int32_t  *values, int32_t number_of_clusters);

int change_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes, int32_t *values, int32_t number_of_clusters);

int change_all_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes, int32_t *values, int32_t number_of_clusters, uint fat_size, int8_t number_of_fat);

void print_directory(FILE *p_file, struct dir_file *files, int number_of_objects, int16_t cluster_size, uint start_of_data, uint max_dir_entries, int level);
#endif
