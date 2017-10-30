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
static uint32_t *fat1 = NULL;
static uint32_t *fat2 = NULL;

int fat_init(const char *fat_name);
void close_fat();

int fat_create_file(struct dir_file **new_file, const char *file_name, uint32_t act_fat_position, unsigned long *dir_position);
struct dir_file *fat_get_object_info_by_name(const char *file_name, unsigned int file_type, uint32_t act_fat_position, unsigned long *dir_position);

int fat_delete_file_by_name(const char *file_name, uint32_t act_fat_position);
int fat_delete_file_by_file(struct dir_file *file, unsigned long position);

size_t fat_read_file(struct dir_file file, char *buffer, unsigned int buffer_size, unsigned long offset);
size_t fat_write_file(struct dir_file *file, unsigned long dir_position, char *buffer, unsigned int buffer_size, unsigned long offset);

int fat_create_dir(struct dir_file **new_dir, const char *dir_name, uint32_t act_fat_position, unsigned long *dir_position);
int fat_delete_empty_dir(const char *dir_name, uint32_t act_fat_position);
struct dir_file *fat_read_dir(uint32_t act_fat_position, uint32_t *files);


int is_boot_record_init();

uint16_t get_cluster_size();

uint32_t get_fat_size_in_bytes();

uint16_t get_dir_clusters();

uint32_t get_dir_size_in_bytes();

unsigned int get_start_of_root_dir();



#endif