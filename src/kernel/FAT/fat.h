#pragma once
#ifndef FAT_H
#define FAT_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "fat_structure.h"

struct fat_data {
	char *memory;			/* pole predstavuje fat disk*/
	size_t memory_size;

	struct boot_record *boot_record;
	uint32_t *fat1;
	uint32_t *fat2;
	unsigned int start_of_fat;
	unsigned int fat_record_size;
	unsigned int fat_size;
	unsigned int start_of_root_dir;
	unsigned int max_dir_entries;
};

int fat_init(struct fat_data *f_data);
void close_fat(struct fat_data *f_data);

int fat_create_file(struct fat_data *f_data, struct dir_file **new_file, const char *file_name, uint32_t act_fat_position, unsigned long *dir_position);
struct dir_file *fat_get_object_info_by_name(struct fat_data *f_data, const char *file_name, unsigned int file_type, uint32_t act_fat_position, unsigned long *dir_position);

int fat_delete_file_by_name(struct fat_data *f_data, const char *file_name, uint32_t act_fat_position);
int fat_delete_file_by_file(struct fat_data *f_data, struct dir_file *file, unsigned long position);

int fat_read_file(struct fat_data *f_data, struct dir_file *file, char *buffer, unsigned int buffer_size, size_t *read, unsigned long offset);
int fat_write_file(struct fat_data *f_data, struct dir_file *file, unsigned long dir_position, char *buffer, unsigned int buffer_size, size_t *writed, unsigned long offset);

int fat_create_dir(struct fat_data *f_data, struct dir_file **new_dir, const char *dir_name, uint32_t act_fat_position, unsigned long *dir_position);
int fat_delete_empty_dir(struct fat_data *f_data, const char *dir_name, uint32_t act_fat_position);
struct dir_file *fat_read_dir(struct fat_data *f_data, uint32_t act_fat_position, uint32_t *files, unsigned long * positions);

int fat_set_file_size(struct fat_data *f_data, struct dir_file * file, size_t file_size, unsigned long dir_position);

#endif