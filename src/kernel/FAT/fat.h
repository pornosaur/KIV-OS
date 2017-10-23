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

int fat_init(char *fat_name);
int close_fat();

struct dir_file *fat_create_file(char *file_name, int32_t act_fat_position, long *dir_position);
struct dir_file *fat_get_object_info_by_name(const char *file_name, int32_t act_fat_position, long *dir_position);

int fat_delete_file_by_name(char *file_name, int32_t act_fat_position);
int fat_delete_file_by_file(struct dir_file *file, long position);

long fat_read_file(struct dir_file file, char *buffer, int buffer_size, long offset);
long fat_write_file(struct dir_file *file, long dir_position, char *buffer, int buffer_size, long offset);

struct dir_file *fat_create_dir(char *dir_name, int32_t act_fat_position, long *dir_position);
int fat_delete_empty_dir(char *dir_name, int32_t act_fat_position);
struct dir_file *fat_read_dir(int32_t act_fat_position, int32_t *files);


char *read_object(int *ret_code, int *data_size, char file_name[], int32_t act_fat_position);



#endif