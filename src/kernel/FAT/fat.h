#pragma once
#ifndef FAT_H
#define FAT_H

#include <stdio.h>
#include <stdint.h>

#include "fat_structure.h"

static FILE *p_file;

static unsigned int start_of_fat = 0;
static unsigned int fat_record_size = 0;
static unsigned int fat_size = 0;
static unsigned int start_of_root_dir = 0;
static unsigned int max_dir_entries = 0;

static struct boot_record *boot_record = NULL;
static int32_t *fat1 = NULL;
static int32_t *fat2 = NULL;

int fat_init(const char *fat_name);
void close_fat();

struct dir_file *fat_create_file(const char *file_name, int32_t act_fat_position, long *dir_position);
struct dir_file *fat_get_object_info_by_name(const char *file_name, int32_t act_fat_position, long *dir_position);

int fat_delete_file_by_name(const char *file_name, int32_t act_fat_position);
int fat_delete_file_by_file(struct dir_file *file, long position);

long fat_read_file(struct dir_file file, char *buffer, int buffer_size, long offset);
long fat_write_file(struct dir_file *file, long dir_position, char *buffer, int buffer_size, long offset);

struct dir_file *fat_create_dir(const char *dir_name, int32_t act_fat_position, long *dir_position);
int fat_delete_empty_dir(const char *dir_name, int32_t act_fat_position);
struct dir_file *fat_read_dir(int32_t act_fat_position, int32_t *files);


char *read_object(int *ret_code, int *data_size, const char file_name[], int32_t act_fat_position);

int is_boot_record_init();

int16_t get_cluster_size();

int32_t get_fat_size_in_bytes();

int16_t get_dir_clusters();

int32_t get_dir_size_in_bytes();

unsigned int get_start_of_root_dir();



#endif