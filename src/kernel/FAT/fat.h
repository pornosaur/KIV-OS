#ifndef FAT_H
#define FAT_H

#include <stdio.h>
#include <stdint.h>

#include "fat_structure.h"

FILE *p_file;

unsigned int start_of_fat = 0;
unsigned int fat_record_size = 0;
unsigned int fat_size = 0;
unsigned int start_of_root_dir = 0;
unsigned int max_dir_entries = 0;
unsigned int start_of_data = 0;

struct boot_record *boot_record = NULL;
int32_t *fat1 = NULL;
int32_t *fat2 = NULL;

int init(char *fat_name);
int close_fat();

struct dir_file *create_file(char file_name[], int32_t act_fat_position);
struct dir_file *get_file_info_by_name(char file_name[], int32_t act_fat_position);

int delete_file_by_name(char file_name[], int32_t act_fat_position);
int delete_file_by_file(struct dir_file *file, long position);

long read_file(struct dir_file file, char *buffer, int buffer_size, long offset);
long write_file(struct dir_file file, int32_t dir_position, char *buffer, int buffer_size, long offset);

struct dir_file *create_dir(char dir_name[], int32_t act_fat_position);
int delete_empty_dir(char dir_name[], int32_t act_fat_position);
struct dir_file *read_dir(int32_t act_fat_position, int32_t *files);



void create_values_from_clusters(const int32_t *clusters, int32_t *values, long size);
void init_object(struct dir_file *object, char name[], int32_t file_size, int8_t file_type, int32_t first_cluster);



char *read_object(int *ret_code, int *data_size, char file_name[], int32_t act_fat_position);


void print_all();

#endif