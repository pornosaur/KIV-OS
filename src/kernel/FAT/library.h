#ifndef FAT_LIBRARY_H
#define FAT_LIBRARY_H

#include <stdlib.h>
#include "fat_structure.h"

struct boot_record *get_boot_record(FILE *p_file);

int32_t *get_fat(FILE *p_file, unsigned int fat_record_size, int cluster_count, unsigned int fat_size);

void delete_cluster(FILE *p_file, unsigned int start_of_cluster, int16_t cluster_size);

int *get_file_clusters(struct dir_file *file, int32_t *clusters_size, int16_t cluster_size, const int32_t *fat, int16_t dir_clusters);

struct dir_file *find_file(FILE *p_file, struct boot_record *boot_record, char file_path[], unsigned int start_of_root_dir, unsigned int start_of_data, unsigned int max_dir_entries);

struct dir_file *get_object_in_dir(FILE * p_file,const char name[], int32_t start_position, unsigned int max_entries, int32_t *object_dir_pos);

struct dir_file *get_all_in_dir(FILE *p_file, int32_t *number_of_objects, long * positions, long start_position, unsigned int max_entries);

int is_dir_empty(FILE *p_file, int32_t max_entries, int32_t start_of_dir);

long find_empty_space_in_dir(FILE *p_file, int32_t max_entries, int32_t start_of_dir);

int write_to_dir(FILE *p_file, struct dir_file *file, int32_t write_position);

int write_file_to_fat(FILE *fat_file, char *file, int file_size, const int32_t *clusters, int32_t clusters_size, unsigned int start_of_data, int16_t cluster_size);

int write_bytes_to_fat(FILE *fat_file, char *bytes, int bytes_size, long offset, const int32_t *clusters, int32_t clusters_size, unsigned int start_of_data, int16_t cluster_size);

int write_empty_dir_to_fat(FILE *fat_file, const int32_t *clusters, int32_t clusters_size, unsigned int start_of_data, int16_t cluster_size);

int find_empty_clusters(int32_t usable_cluster_count, const int32_t *fat, int32_t *clusters, long number_of_clusters);

int rm_from_fat(int32_t *fat, const int32_t *indexes, int32_t number_of_clusters);

int rm_from_all_physic_fat(FILE *p_file, unsigned int fat_record_size, int32_t fat_start, int32_t *indexes, int32_t number_of_clusters, unsigned int fat_size, int8_t number_of_fat);

int rm_from_physic_fat(FILE *p_file, unsigned int fat_record_size, int32_t fat_start, const int32_t *indexes, int32_t number_of_clusters);

int change_fat(int32_t *fat, const int32_t *indexes, const int32_t  *values, int32_t number_of_clusters);

int change_physic_fat(FILE *p_file, unsigned int fat_record_size, int32_t fat_start, const int32_t *indexes, int32_t *values, int32_t number_of_clusters);

int change_all_physic_fat(FILE *p_file, unsigned int fat_record_size, int32_t fat_start, int32_t *indexes, int32_t *values, int32_t number_of_clusters, unsigned int fat_size, int8_t number_of_fat);

void print_directory(FILE *p_file, struct dir_file *files, int number_of_objects, int16_t cluster_size, unsigned int start_of_data, unsigned int max_dir_entries, int level);
#endif
