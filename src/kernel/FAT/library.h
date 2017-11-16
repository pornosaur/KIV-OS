#ifndef FAT_LIBRARY_H
#define FAT_LIBRARY_H

#include <stdlib.h>
#include "fat_structure.h"

struct boot_record *get_boot_record(char *memory, size_t memory_size);

uint32_t *get_fat(char *memory, size_t memory_size, size_t start_of_fat, unsigned int fat_record_size, unsigned long cluster_count, unsigned long fat_size);

void delete_cluster(char *memory, size_t memory_size, unsigned int start_of_cluster, uint16_t cluster_size);

int *get_file_clusters(struct dir_file *file, uint32_t *clusters_size, uint16_t cluster_size, const uint32_t *fat, uint16_t dir_clusters);

struct dir_file *get_object_in_dir(char *memory, size_t memory_size, const char name[], unsigned int file_type, uint32_t start_position, unsigned int max_entries, uint32_t *object_dir_pos);

struct dir_file *get_all_in_dir(char *memory, size_t memory_size, uint32_t *number_of_objects, unsigned long * positions, unsigned long start_position, unsigned int max_entries);

int is_dir_empty(char *memory, size_t memory_size, uint32_t max_entries, uint32_t start_of_dir);

int find_empty_space_in_dir(char *memory, size_t memory_size, unsigned long *position, uint32_t max_entries, uint32_t start_of_dir);

int write_to_dir(char *memory, size_t memory_size, struct dir_file *file, uint32_t write_position);

int write_file_to_fat(char *memory, size_t memory_size, char *file, unsigned int file_size, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size);

size_t write_bytes_to_fat(char *memory, size_t memory_size, char *bytes, unsigned long bytes_size, unsigned long offset, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size);

int write_empty_dir_to_fat(char *memory, size_t memory_size, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size);

int find_empty_clusters(uint32_t usable_cluster_count, const uint32_t *fat, uint32_t *clusters, unsigned long number_of_clusters);

int rm_from_fat(uint32_t *fat, const uint32_t *indexes, uint32_t number_of_clusters);

int rm_from_all_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, uint32_t *indexes, uint32_t number_of_clusters, unsigned int fat_size, uint8_t number_of_fat);

int rm_from_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, const uint32_t *indexes, uint32_t number_of_clusters);

int change_fat(uint32_t *fat, const uint32_t *indexes, const uint32_t  *values, uint32_t number_of_clusters);

int change_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, const uint32_t *indexes, uint32_t *values, uint32_t number_of_clusters);

int change_all_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, uint32_t *indexes, uint32_t *values, uint32_t number_of_clusters, unsigned int fat_size, uint8_t number_of_fat);

void print_directory(char *memory, size_t memory_size, struct dir_file *files, unsigned int number_of_objects, uint16_t cluster_size, unsigned int start_of_data, unsigned int max_dir_entries, unsigned int level);

void remove_record_in_dir(char *memory, size_t memory_size, unsigned long position);

size_t read_file(char *memory, size_t memory_size, char *buffer, unsigned int buffer_size, unsigned long offset, uint32_t file_size, uint16_t cluster_size, uint32_t *clusters, unsigned int start_of_root_dir, uint32_t file_clusters_size);
#endif
