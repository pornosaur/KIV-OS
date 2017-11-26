#include <stdio.h>
#include <string.h>
#include "library.h"

struct boot_record *get_boot_record(char *memory, size_t memory_size) {

	if (memory_size < sizeof(struct boot_record)) {
		return NULL;
	}

	struct boot_record *boot_record = (struct boot_record *) malloc(sizeof(struct boot_record));
	if (!boot_record) {
		return NULL;
	}

	memcpy(boot_record, memory, sizeof(struct boot_record));

	return boot_record;
}


uint32_t *get_fat(char *memory, size_t memory_size, size_t start_of_fat, unsigned int fat_record_size, unsigned long cluster_count, unsigned long fat_size) {
	uint32_t *fat = NULL;

	if (memory_size < start_of_fat + fat_size) {
		return NULL;
	}

	fat = (uint32_t *)malloc(fat_size);
	if (!fat) {
		return NULL;
	}

	size_t i = 0;
	size_t memory_shift = start_of_fat;
	for (i = 0; i < cluster_count; i++) {
		memcpy(fat + i, memory + memory_shift, fat_record_size);
		memory_shift += fat_record_size;
	}
	return fat;
}


void delete_cluster(char *memory, size_t memory_size, unsigned int start_of_cluster, uint16_t cluster_size) {
	size_t i = 0;
	char new_value = 0;
	size_t cpy_size = sizeof(new_value);

	if (memory_size < start_of_cluster + cluster_size) {
		return;
	}

	for (i = 0; i < cluster_size; i++) {
		memcpy(memory + start_of_cluster + (i * cpy_size), &new_value, cpy_size);
	}
}


uint32_t *get_file_clusters(struct dir_file *file, uint32_t *clusters_size, uint16_t cluster_size, const uint32_t *fat, uint16_t dir_clusters) {
	unsigned long cluster_position = 0;
	unsigned int counter = 0;
	size_t i = 0;
	uint32_t *clusters;

	if (file->file_size == 0) {
		return NULL;
	}


	if (file->file_type == OBJECT_DIRECTORY) {
		*clusters_size = dir_clusters;
	}
	else {
		*clusters_size = file->file_size / cluster_size;
	}

	if (file->file_size % cluster_size) {
		*clusters_size += 1;
	}

	if (*clusters_size == 0) {
		*clusters_size = 1;
	}

	clusters = malloc(sizeof(uint32_t) * (*clusters_size) + 1);
	if (!clusters) {
		return NULL;
	}

	cluster_position = file->first_cluster;

	while (cluster_position != FAT_FILE_END) {
		counter++;
		if (*clusters_size < counter || cluster_position == FAT_BAD_CLUSTER || cluster_position == FAT_UNUSED) {
			free(clusters);
			clusters = NULL;
			break;
		}
		clusters[i] = cluster_position;
		i++;

		cluster_position = fat[cluster_position];
	}

	return clusters;
}


struct dir_file *get_object_in_dir(char *memory, size_t memory_size, const char name[], unsigned int file_type, uint32_t start_position, unsigned int max_entries, uint32_t *object_dir_pos) {
	size_t i = 0;
	unsigned int read_size;
	struct dir_file *object = NULL;
	read_size = sizeof(struct dir_file);

	if (memory_size < start_position + (max_entries * read_size)) {
		return NULL;
	}

	object = (struct dir_file *) malloc(sizeof(struct dir_file));
	if (!object) {
		return NULL;
	}

	*object_dir_pos = start_position;


	for (i = 0; i < max_entries; i++) {
		memcpy(object, memory + start_position + (i * read_size), read_size);

		if (strcmp(object->file_name, name) == 0 && object->file_type == file_type) {
			return object;
		}
		*object_dir_pos += read_size;
	}

	free(object);
	return NULL;
}


struct dir_file *get_all_in_dir(char *memory, size_t memory_size, uint32_t *number_of_objects, unsigned long * positions, unsigned long start_position, unsigned int max_entries) {
	size_t i = 0;
	struct dir_file *files = NULL;
	struct dir_file file;
	unsigned long position = 0;
	unsigned long read_size = sizeof(struct dir_file);

	if (memory == NULL || memory_size < start_position + (max_entries * read_size)) {
		return NULL;
	}

	*number_of_objects = 0;

	files = malloc(sizeof(struct dir_file) * max_entries);
	if (!files) {
		return NULL;
	}

	position = start_position;

	for (i = 0; i < max_entries; i++) {
		memcpy(&file, memory + start_position + (i * read_size), read_size);

		if (file.file_name[0] != '\0') {
			files[*number_of_objects] = file;
			if (positions != NULL) {
				positions[*number_of_objects] = position;
			}
			*number_of_objects += 1;
		}
		position += read_size;
	}

	return files;
}


int is_dir_empty(char *memory, size_t memory_size, uint32_t max_entries, uint32_t start_of_dir) {
	size_t i = 0;
	struct dir_file *file = NULL;
	size_t read_size = sizeof(struct dir_file);

	if (memory == NULL || memory_size < start_of_dir + (max_entries * read_size)) {
		return -1;
	}

	file = (struct dir_file *) malloc(sizeof(struct dir_file));
	if (!file) {
		return -1;
	}

	for (i = 0; i < max_entries; i++) {
		memcpy(file, memory + start_of_dir + (i * read_size), read_size);

		if (file != NULL && file->file_name[0] != '\0') {
			free(file);
			return 0;
		}
	}
	free(file);
	return 1;
}


int find_empty_space_in_dir(char *memory, size_t memory_size, unsigned long *position, uint32_t max_entries, uint32_t start_of_dir) {
	size_t i = 0;
	struct dir_file *file = NULL;
	unsigned long read_size = sizeof(struct dir_file);

	if (memory == NULL || memory_size < start_of_dir + (max_entries * read_size)) {
		return -1;
	}

	file = (struct dir_file *) malloc(sizeof(struct dir_file));
	if (!file) {
		return -1;
	}

	*position = start_of_dir;
	for (i = 0; i < max_entries; i++) {
		memcpy(file, memory + *position, read_size);

		if (file != NULL && file->file_name[0] == '\0') {
			free(file);
			return 0;
		}
		*position += read_size;
	}
	free(file);
	return -1;
}


int write_to_dir(char *memory, size_t memory_size, struct dir_file *file, uint32_t write_position) {
	if (memory == NULL || memory_size < write_position + sizeof(struct dir_file)) {
		return -1;
	}

	memcpy(memory + write_position, file, sizeof(struct dir_file));

	return 0;
}


size_t write_bytes_to_fat(char *memory, size_t memory_size, char *bytes, unsigned long bytes_size, unsigned long offset, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size) {
	uint16_t first_cluster = 0;
	unsigned int cluster_offset = 0;
	size_t i = 0;
	size_t writed_bytes = 0;
	size_t write_size = 0;
	unsigned long write_position = 0;

	if (memory == NULL || memory_size < start_of_data + (clusters_size * cluster_size) || bytes == NULL || clusters == NULL || cluster_size < 0) {
		return 0;
	}

	first_cluster = (uint16_t)(offset / cluster_size);
	cluster_offset = offset % cluster_size;

	writed_bytes = 0;
	for (i = first_cluster; i < clusters_size; i++) {

		write_size = cluster_size - cluster_offset;
		write_position = start_of_data + (clusters[i] * cluster_size) + cluster_offset;

		if (writed_bytes + write_size > bytes_size) {
			write_size = bytes_size - writed_bytes;
		}

		memcpy(memory + write_position, &bytes[writed_bytes], write_size);

		writed_bytes += write_size;
		cluster_offset = 0;
	}

	return writed_bytes;
}


int write_empty_dir_to_fat(char *memory, size_t memory_size, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size) {
	size_t i = 0;
	unsigned long cluster_position = 0;
	char *data = NULL;
	uint16_t u_cluster_size = 0;

	if (memory == NULL || memory_size < start_of_data + (clusters_size * cluster_size) || clusters == NULL) {
		return -1;
	}

	u_cluster_size = cluster_size;

	data = malloc(u_cluster_size);
	if (!data) {
		return -1;
	}

	memset(data, 0, u_cluster_size);

	for (i = 0; i < clusters_size; i++) {
		cluster_position = start_of_data + (clusters[i] * cluster_size);

		memcpy(memory + cluster_position, data, u_cluster_size);
	}

	free(data);
	return 0;
}


int find_empty_clusters(uint32_t usable_cluster_count, const uint32_t *fat, uint32_t *clusters, unsigned long number_of_clusters) {
	size_t i = 0, j = 0;

	for (i = 0; i < usable_cluster_count; i++) {
		if (fat[i] == FAT_UNUSED) {
			clusters[j] = (uint32_t)i;
			j++;
			if (j == number_of_clusters) {
				return 0;
			}
		}
	}
	return -1;
}


int rm_from_fat(uint32_t *fat, const uint32_t *indexes, uint32_t number_of_clusters) {
	size_t i = 0;

	if (number_of_clusters == 0 || indexes == NULL || fat == NULL) {
		return -1;
	}

	for (i = 0; i < number_of_clusters; i++) {
		if (fat[indexes[i]] != FAT_BAD_CLUSTER) {
			fat[indexes[i]] = FAT_UNUSED;
		}
	}

	return 0;
}


int rm_from_all_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, uint32_t *indexes,
	uint32_t number_of_clusters, unsigned int fat_size, uint8_t number_of_fat) {
	size_t i = 0;
	int result = 0;

	for (i = 0; i < number_of_fat; i++) {
		result = rm_from_physic_fat(memory, memory_size, fat_record_size, fat_start + ((uint32_t)i * fat_size), indexes, number_of_clusters);
		if (result == -1) {
			return -1;
		}
	}

	return 0;
}


int rm_from_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, const uint32_t *indexes,
	uint32_t number_of_clusters) {
	size_t i = 0;
	uint32_t index = 0;
	uint32_t new_value = 0;
	unsigned long position = 0;

	if (memory == NULL || memory_size < fat_start + (number_of_clusters * fat_record_size) || indexes == NULL || number_of_clusters == 0) {
		return -1;
	}

	new_value = FAT_UNUSED;
	for (i = 0; i < number_of_clusters; i++) {
		position = fat_start + (fat_record_size * indexes[i]);

		memcpy(&index, memory + position, fat_record_size);
		if (index != FAT_BAD_CLUSTER) {
			memcpy(memory + position, &new_value, fat_record_size);
		}
	}

	return 0;
}


int change_fat(uint32_t *fat, const uint32_t *indexes, const uint32_t *values, uint32_t number_of_clusters) {
	size_t i = 0;

	if (number_of_clusters == 0 || indexes == NULL || values == NULL || fat == NULL) {
		return -1;
	}

	for (i = 0; i < number_of_clusters; i++) {
		fat[indexes[i]] = values[i];
	}

	return 0;
}


int change_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, const uint32_t *indexes, uint32_t *values,
	uint32_t number_of_clusters) {
	size_t i = 0;

	if (memory == NULL || memory_size < fat_start + (number_of_clusters * fat_record_size) || indexes == NULL || values == NULL || number_of_clusters == 0) {
		return -1;
	}

	for (i = 0; i < number_of_clusters; i++) {
		memcpy(memory + fat_start + (fat_record_size * indexes[i]), &values[i], fat_record_size);
	}

	return 0;
}


int change_all_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, uint32_t *indexes, uint32_t *values,
	uint32_t number_of_clusters, unsigned int fat_size, uint8_t number_of_fat) {
	size_t i = 0;
	int result = 0;

	for (i = 0; i < number_of_fat; i++) {
		result = change_physic_fat(memory, memory_size, fat_record_size, fat_start + ((uint32_t)i * fat_size), indexes, values,
			number_of_clusters);
		if (result == -1) {
			return -1;
		}
	}

	return 0;
}


void remove_record_in_dir(char *memory, size_t memory_size, unsigned long position) {
	size_t i = 0;
	size_t write_size = sizeof(char);

	if (memory == NULL || memory_size < position + sizeof(struct dir_file)) {
		return;
	}

	for (i = 0; i < sizeof(struct dir_file); i++) {
		memset(memory + position + (i * write_size), '\0', write_size);
	}
}


size_t read_file(char *memory, size_t memory_size, char *buffer, unsigned int buffer_size, unsigned long offset, uint32_t file_size, uint16_t cluster_size, uint32_t *clusters, unsigned int start_of_root_dir, uint32_t file_clusters_size) {
	char *cluster = NULL;
	uint32_t read_size = 0;
	size_t act_read_size = 0;
	size_t writed_size = 0;
	size_t i = 0;
	unsigned long first_cluster = 0;
	unsigned int cluster_offset = 0;


	if (memory == NULL || memory_size < start_of_root_dir + (file_clusters_size * cluster_size) || file_size == 0) {
		return writed_size;
	}

	cluster = malloc(sizeof(char) * cluster_size);
	read_size = sizeof(char) * cluster_size;

	first_cluster = offset / cluster_size;
	cluster_offset = offset % cluster_size;

	writed_size = 0;
	for (i = first_cluster; i < file_clusters_size; i++) {

		act_read_size = read_size - cluster_offset;
		if (writed_size + act_read_size > buffer_size) {
			act_read_size = buffer_size - writed_size;
		}
		if (offset + writed_size + act_read_size > file_size) {
			act_read_size = file_size - writed_size - offset;
		}

		if (act_read_size == 0) {
			break;
		}

		memcpy(buffer + writed_size, memory + clusters[i] * cluster_size + start_of_root_dir + cluster_offset, act_read_size);

		writed_size += act_read_size;
		cluster_offset = 0;
	}

	free(cluster);

	return writed_size;
}
