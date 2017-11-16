#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fat_structure.h"
#include "library.h"
#include "fat.h"

size_t write_bigger_file(struct fat_data *f_data, uint32_t *old_clusters, uint32_t new_file_clusters_size, uint32_t old_file_clusters_size, char *buffer, unsigned int buffer_size, unsigned long offset);

size_t write_smaller_file(struct fat_data *f_data, uint32_t *old_clusters, uint32_t new_file_clusters_size, uint32_t old_file_clusters_size, char *buffer, unsigned int buffer_size, unsigned long offset);

size_t write_same_file(struct fat_data *f_data, uint32_t *old_clusters, uint32_t new_file_clusters_size, char *buffer, unsigned int buffer_size, unsigned long offset);

void create_values_from_clusters(const uint32_t *clusters, uint32_t *values, unsigned long size);

void init_object(struct dir_file *object,const char name[], uint32_t file_size, uint8_t file_type, uint32_t first_cluster);

void print_all(struct fat_data *f_data);

int remove_unused_clusters(struct fat_data *f_data, uint32_t *clusters, uint32_t clusters_old_size, unsigned long clusters_new_size);


int fat_init(struct fat_data *f_data) {

	if (f_data == NULL || f_data->memory == NULL) {
		return -1;
	}

	f_data->boot_record = get_boot_record(f_data->memory, f_data->memory_size);

    f_data->start_of_fat = f_data->boot_record->reserved_cluster_count * f_data->boot_record->cluster_size;
	f_data->fat_record_size = sizeof(uint32_t);
	f_data->fat_size = f_data->boot_record->usable_cluster_count * f_data->fat_record_size;
	f_data->start_of_root_dir = f_data->start_of_fat + (f_data->boot_record->fat_copies * f_data->fat_size);
	f_data->max_dir_entries = f_data->boot_record->cluster_size / sizeof(struct dir_file);

	f_data->fat1 = get_fat(f_data->memory, f_data->memory_size, f_data->start_of_fat, f_data->fat_record_size, f_data->boot_record->usable_cluster_count, f_data->fat_size);
	f_data->fat2 = get_fat(f_data->memory, f_data->memory_size, f_data->start_of_fat + f_data->fat_size, f_data->fat_record_size, f_data->boot_record->usable_cluster_count, f_data->fat_size);
	
	return 0;
}

void close_fat(struct fat_data *f_data){
    free(f_data->boot_record);
    free(f_data->fat1);
    free(f_data->fat2);
}

/**
 * Vlozi data do fat.
 *
 * @param data data, ktera se zapisi
 * @param data_size velikost zapisovanych dat
 * @param file_name jmeno vytvareneho souboru
 * @param act_fat_position pozice adresare ve kterem vytvarime soubor
 * @return chybovy kod nebo 0 pri uspechu
 */
int fat_create_file(struct fat_data *f_data, struct dir_file **new_file, const char *file_name, uint32_t act_fat_position, unsigned long *dir_position) {
    struct dir_file *file = NULL;
    uint32_t *clusters = NULL;
    uint32_t *values = NULL;
    uint32_t position = 0;
	uint32_t object_dir_pos = 0;
    unsigned long file_cluster_count = 0;
    int result = 0;

    if(f_data == NULL || f_data->boot_record == NULL || f_data->fat1 == NULL || f_data->fat2 == NULL){
        return 1; // NOT INITIALIZE
    }

    if(strlen(file_name) > NAME_SIZE){
        return 2; // NAME IS TOO LONG
    }

    if(act_fat_position < 0 || act_fat_position >= f_data->boot_record->usable_cluster_count){
        return 9; // POSITION OUT OF RANGE
    }

    position = f_data->start_of_root_dir + (act_fat_position * f_data->boot_record->cluster_size);

    // check if file exists
	file = get_object_in_dir(f_data->memory, f_data->memory_size, file_name, OBJECT_FILE, position, f_data->max_dir_entries, &object_dir_pos);
    if (file != NULL) {
        if(file->file_type == OBJECT_FILE) {
            result = fat_delete_file_by_file(f_data, file, object_dir_pos);
            if (result) {
                return result;
            }
        } else {
            return 8;
        }
    }

    // check empty space in dir
    result = find_empty_space_in_dir(f_data->memory, f_data->memory_size, dir_position, f_data->max_dir_entries, position);
    if (result == -1) {
        return 6; // DIRECTORY IS FULL
    }

    // number of clusters of new file
    file_cluster_count = 1;

    // search free space in fat
    clusters = malloc(sizeof(uint32_t) * file_cluster_count);
	if (!clusters) {
		return 12;
	}

    if(find_empty_clusters(f_data->boot_record->usable_cluster_count, f_data->fat1, clusters, file_cluster_count) == -1){
        free(clusters);
        return 7; // FAT IS FULL
    }

    *new_file = malloc(sizeof(struct dir_file));
	if (!(*new_file)) {
		free(clusters);
		return 12; // OUT OF MEMORY
	}

    init_object(*new_file, file_name, 1, OBJECT_FILE, clusters[0]);

    write_to_dir(f_data->memory, f_data->memory_size, *new_file, (uint32_t) *dir_position);

    values = malloc(sizeof(uint32_t) * file_cluster_count);
	if (!values) {
		free(clusters);
		free(*new_file);
		*new_file = NULL;
		return 12; // OUT OF MEMORY
	}

    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(f_data->fat1, clusters, values, (uint32_t)file_cluster_count);
    change_fat(f_data->fat2, clusters, values, (uint32_t)file_cluster_count);

    change_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, clusters, values, (uint32_t) file_cluster_count,
                          f_data->fat_size, f_data->boot_record->fat_copies);

    free(clusters);
    free(values);

    return 0; // SUCCESS
}



struct dir_file *fat_get_object_info_by_name(struct fat_data *f_data, const char *file_name, unsigned int file_type, uint32_t act_fat_position, unsigned long *dir_position){
    uint32_t position = 0;

    if(f_data == NULL || f_data->boot_record == NULL || f_data->fat1 == NULL || f_data->fat2 == NULL){
        //return 1;
        return NULL;
    }

    if(strlen(file_name) > NAME_SIZE){
        //return 2;
        return NULL;
    }

    if(act_fat_position < 0 || act_fat_position >= f_data->boot_record->usable_cluster_count){
        //return 9;
        return NULL;
    }

    position = f_data->start_of_root_dir + (act_fat_position * f_data->boot_record->cluster_size);

    // check if file exists
    struct dir_file *dir_file = get_object_in_dir(f_data->memory, f_data->memory_size, file_name, file_type, position, f_data->max_dir_entries, dir_position);

    return dir_file;
}

/*
struct dir_file *get_file_info(char file_name[], int32_t act_fat_position){
    int32_t position = 0;
	int32_t object_dir_pos = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        //return 1;
        return NULL;
    }

    if(strlen(file_name) > NAME_SIZE){
        //return 2;
        return NULL;
    }

    if(act_fat_position < 0 || act_fat_position >= boot_record->usable_cluster_count){
        //return 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    return get_object_in_dir(p_file, file_name, position, max_dir_entries, &object_dir_pos);
}*/
/**
 * Podle vstupniho pole clusters udavajici indexy do FAT pro jeden soubor, vytvori pole values predstavujici hodnoty na
 * techto indexech.
 *
 * @param clusters indexy do fat tabulky na kterych je ulozen jeden soubor
 * @param values prazdne pole ktere bude naplneno
 * @param size velikost techto poli
 */
void create_values_from_clusters(const uint32_t *clusters, uint32_t *values, unsigned long size){
    size_t i = 0;

    for(i = 1; i < size; i++){
        values[i-1] = clusters[i];
    }
    values[size-1] = FAT_FILE_END;
}

/**
 * Nastavi vstupni strukture objekt parametry, ktere jsou predany jako ostatni parametry.
 *
 * @param object struktura, ktera se bude nastavovat
 * @param name nazev objektu
 * @param file_size velikost objektu
 * @param file_type typ objektu
 * @param first_cluster index prvniho clusteru objektu
 */
void init_object(struct dir_file *object, const char name[], uint32_t file_size, uint8_t file_type, uint32_t first_cluster){
    memset(object->file_name, 0, NAME_SIZE + 1);
    strcpy_s(object->file_name, NAME_SIZE + 1, name);
    object->file_size = file_size;
    object->file_type = file_type;
    object->first_cluster = first_cluster;
}

/**
 * Smaze soubor ve FAT v danem adresari s danym jmenem.
 *
 * @param file_name jmeno hledaneho souboru
 * @param act_fat_position pozice adresare ve kterem hledame soubor
 * @return chybovy kod nebo 0 pri uspechu
 */
int fat_delete_file_by_name(struct fat_data *f_data, const char *file_name, uint32_t act_fat_position) {

    struct dir_file *file = NULL;
    uint32_t position = 0;
	uint32_t object_dir_pos = 0;

	if (f_data == NULL || f_data->boot_record == NULL || f_data->fat1 == NULL || f_data->fat2 == NULL) {
        return 1; // NOT INITIALIZE
    }

    if(strlen(file_name) > NAME_SIZE){
        return 2; // NAME IS TOO LONG
    }

    if(act_fat_position < 0 || act_fat_position >= f_data->boot_record->usable_cluster_count){
        return 9; // POSITION OUT OF RANGE
    }

    position = f_data->start_of_root_dir + (act_fat_position * f_data->boot_record->cluster_size);

    file = get_object_in_dir(f_data->memory, f_data->memory_size, file_name, OBJECT_FILE, position, f_data->max_dir_entries, &object_dir_pos);
    if (file == NULL || file->file_type != OBJECT_FILE) {
        free(file);
        return 3; // PATH NOT FOUND
    }

    return fat_delete_file_by_file(f_data, file, object_dir_pos);
}

int fat_delete_file_by_file(struct fat_data *f_data, struct dir_file *file, unsigned long position){
    uint32_t *clusters = NULL;
    uint32_t file_clusters_size = 0;

    clusters = get_file_clusters(file, &file_clusters_size, f_data->boot_record->cluster_size, f_data->fat1,
                                 f_data->boot_record->dir_clusters);
    if (clusters == NULL) {
        free(file);
        return 11; // CANT GET CLUSTERS
    }

	remove_record_in_dir(f_data->memory, f_data->memory_size, position);

    rm_from_fat(f_data->fat1, clusters, file_clusters_size);
    rm_from_fat(f_data->fat2, clusters, file_clusters_size);
    rm_from_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, clusters, file_clusters_size, f_data->fat_size,
                           f_data->boot_record->fat_copies);

    free(file);
    free(clusters);

    return 0; // SUCCESS
}

/**
 * Vytrovy novy prazdny adresar na zvolenem miste.
 *
 * @param dir_name jmeno hledaneho adresare
 * @param act_fat_position pozice adresare ve kterem hledame adresar
 * @return chybovy kod nebo 0 pri uspechu
 */
int fat_create_dir(struct fat_data *f_data, struct dir_file **new_dir, const char *dir_name, uint32_t act_fat_position, unsigned long *dir_position) {
    struct dir_file *file = NULL;
    uint32_t *clusters = NULL;
    uint32_t *values = NULL;
    uint32_t position = 0;
	uint32_t object_dir_pos = 0;
    unsigned long file_cluster_count = 0;
	int result = 0;

    if(f_data == NULL || f_data->boot_record == NULL || f_data->fat1 == NULL || f_data->fat2 == NULL){
        return 1; // NOT_INITIALIZE
    }

    if(strlen(dir_name) > NAME_SIZE){
        return 2; // NAME IS TOO LONG
    }

    if(act_fat_position < 0 || act_fat_position >= f_data->boot_record->usable_cluster_count){
        return 9; // POSITION OUT OF RANGE
    }

    position = f_data->start_of_root_dir + (act_fat_position * f_data->boot_record->cluster_size);

    file = get_object_in_dir(f_data->memory, f_data->memory_size, dir_name, OBJECT_DIRECTORY, position, f_data->max_dir_entries, &object_dir_pos);
    if (file != NULL) {
        free(file);
        return 4; // PATH EXISTS
    }

    result = find_empty_space_in_dir(f_data->memory, f_data->memory_size, dir_position, f_data->max_dir_entries, position);
    if (result == -1) {
        return 6; // DIRECTORY IS FULL
    }

    file_cluster_count = f_data->boot_record->dir_clusters;
    clusters = malloc(sizeof(uint32_t) * file_cluster_count);
	if (!clusters) {
		return 12;
	}

    if(find_empty_clusters(f_data->boot_record->usable_cluster_count, f_data->fat1, clusters, file_cluster_count) == -1){
        free(clusters);
        return 7; // FAT IS FULL
    }


    *new_dir = malloc(sizeof(struct dir_file));
	if (!(*new_dir)) {
		free(clusters);
		return 12; // OUT OF MEMORY
	}

    init_object(*new_dir, dir_name, f_data->boot_record->dir_clusters * f_data->boot_record->cluster_size, OBJECT_DIRECTORY, clusters[0]);

    write_empty_dir_to_fat(f_data->memory, f_data->memory_size, clusters, (uint32_t) file_cluster_count, f_data->start_of_root_dir, f_data->boot_record->cluster_size);
    write_to_dir(f_data->memory, f_data->memory_size, *new_dir, (uint32_t) *dir_position);

    values = malloc(sizeof(uint32_t) * file_cluster_count);
	if (!values) {
		free(clusters);
		free(*new_dir);
		*new_dir = NULL;
		return 12; // OUT OF MEMORY
	}
    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(f_data->fat1, clusters, values, (uint32_t)file_cluster_count);
    change_fat(f_data->fat2, clusters, values, (uint32_t)file_cluster_count);

    change_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, clusters, values, (uint32_t) file_cluster_count,
                          f_data->fat_size, f_data->boot_record->fat_copies);

    free(clusters);
    free(values);

    return 0;
}

/**
 * Smaze prazdny adresar ve FAT.
 *
 * @param dir_name jmeno hledaneho adresare
 * @param act_fat_position pozice adresare ve kterem hledame adresar
 * @return chybovy kod nebo 0 pri uspechu
 */
int fat_delete_empty_dir(struct fat_data *f_data, const char *dir_name, uint32_t act_fat_position) {
    unsigned long position = 0;
    uint32_t parent_position = 0;
    uint32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    uint32_t *clusters = NULL;
    int result = 0;

    if(f_data == NULL || f_data->boot_record == NULL || f_data->fat1 == NULL || f_data->fat2 == NULL){
        return 1; // NOT INITIALIZE
    }

    if(strlen(dir_name) > NAME_SIZE){
        return 2; // NAME IS TOO LONG
    }

    if(act_fat_position < 0 || act_fat_position >= f_data->boot_record->usable_cluster_count){
        return 9; // POSITION OUT OF RANGE
    }

    parent_position = f_data->start_of_root_dir + (act_fat_position * f_data->boot_record->cluster_size);

    file = get_object_in_dir(f_data->memory, f_data->memory_size, dir_name, OBJECT_DIRECTORY, parent_position, f_data->max_dir_entries, &position);
    if (file == NULL || file->file_type != OBJECT_DIRECTORY) {
        free(file);
        return 3; // PAHT NOT FOUND
    }

    result = is_dir_empty(f_data->memory, f_data->memory_size, f_data->max_dir_entries, f_data->start_of_root_dir + (file->first_cluster * f_data->boot_record->cluster_size));
    if (result == 0) {
        free(file);
        return 5; // DIR NOT EMPTY
    } else if (result == -1) {
        free(file);
        return 8; // ERROR
    }

    clusters = get_file_clusters(file, &file_clusters_size, f_data->boot_record->cluster_size, f_data->fat1,
                                 f_data->boot_record->dir_clusters);
    if (clusters == NULL) {
        free(file);
        return 11; // CANT GET CLUSTERS
    }

    rm_from_fat(f_data->fat1, clusters, file_clusters_size);
    rm_from_fat(f_data->fat2, clusters, file_clusters_size);

    rm_from_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, clusters, file_clusters_size, f_data->fat_size,
                           f_data->boot_record->fat_copies);

	remove_record_in_dir(f_data->memory, f_data->memory_size, position);

    free(file);
    free(clusters);

    return 0; // SUCCES
}

/**
 * Vrati soubor s nactenymi daty
 *
 * @param ret_code obsahuje chybovy kode nebo 0 pri uspechu
 * @param data_size velikost vracenych dat
 * @param file_name jmeno hledaneho souboru
 * @param act_fat_position pozice adresare ve kterem hledame soubor
 * @return nactena data nebo NULL pri neuspechu
 */
/*
char* read_object(int *ret_code, int *data_size,const char file_name[], int32_t act_fat_position) {
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    char *cluster = NULL;
    char *data = NULL;
    int32_t position = 0;
    int32_t read_size = 0;
    int32_t writed_size = 0;
	int32_t object_dir_pos = 0;
    int i = 0;

    if(boot_record == NULL || fat1 == NULL){
        *ret_code = 1;
        return NULL;
    }

    if(strlen(file_name) > NAME_SIZE){
        *ret_code = 2;
        return NULL;
    }

    if(act_fat_position < 0 || act_fat_position >= boot_record->usable_cluster_count){
        *ret_code = 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    file = get_object_in_dir(p_file, file_name, position, max_dir_entries, &object_dir_pos);
    if (file == NULL || file->file_type != OBJECT_FILE) {
        free(file);
        *ret_code = 3;
        return NULL;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat1,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        free(file);
        *ret_code = 3;
        return NULL;
    }

    *data_size = file->file_size;

    cluster = malloc(sizeof(char) * boot_record->cluster_size);
    data = malloc(sizeof(char) * *data_size);

    read_size = sizeof(char) * boot_record->cluster_size;

    for (i = 0; i < file_clusters_size; i++) {
        fseek(p_file, clusters[i] * boot_record->cluster_size + start_of_root_dir, SEEK_SET);
        writed_size = i * boot_record->cluster_size;
        if(writed_size + read_size > *data_size){
            read_size = *data_size - writed_size;
        }
        fread(data + writed_size, (size_t) read_size, 1, p_file);
    }

    free(file);
    free(clusters);
    free(cluster);

    *ret_code = 0;
    return data;
}*/

size_t fat_read_file(struct fat_data *f_data, struct dir_file *file, char *buffer, unsigned int buffer_size, unsigned long offset) {
    uint32_t file_clusters_size = 0;
    uint32_t *clusters = NULL;
	size_t writed_size = 0;

    if(
			f_data == NULL ||
            f_data->boot_record == NULL ||
            file->file_name == NULL ||
            buffer == NULL ||
            buffer_size <= 0 ||
            file->file_size < offset ||
            file->file_type == OBJECT_DIRECTORY ||
            file->first_cluster >= f_data->fat_size ||
            file->first_cluster < 0 ||
            file->file_size == 0
            ){

        return -1;
    }

    clusters = get_file_clusters(file, &file_clusters_size, f_data->boot_record->cluster_size, f_data->fat1, f_data->boot_record->dir_clusters);
    if (clusters == NULL) {
        return 0;
    }

	writed_size = read_file(f_data->memory, f_data->memory_size, buffer, buffer_size, offset, file->file_size, f_data->boot_record->cluster_size, clusters, f_data->start_of_root_dir, file_clusters_size);

    free(clusters);

    return writed_size;
}

size_t fat_write_file(struct fat_data *f_data, struct dir_file *file, unsigned long dir_position, char *buffer, unsigned int buffer_size, unsigned long offset){

    uint32_t old_file_clusters_size = 0;
    uint32_t *old_clusters = NULL;
    unsigned long new_file_clusters_size = 0;
    size_t writed_bytes = 0;

    if(
			f_data == NULL || 
            f_data->boot_record == NULL ||
            f_data->fat1 == NULL ||
            f_data->fat2 == NULL ||
            file -> file_name == NULL ||
            buffer == NULL ||
            buffer_size <= 0 ||
            offset < 0 ||
            file -> file_type == OBJECT_DIRECTORY ||
            file -> first_cluster >= f_data->fat_size ||
            file -> first_cluster < 0
            ){

        return 0;
    }

    // number of clusters of new file
    new_file_clusters_size = (offset + buffer_size) / f_data->boot_record -> cluster_size;
    if((offset + buffer_size) % f_data->boot_record -> cluster_size > 0){
        new_file_clusters_size ++;
    }

    // get old clusters
    old_clusters = get_file_clusters(file, &old_file_clusters_size, f_data->boot_record->cluster_size, f_data->fat1, f_data->boot_record->dir_clusters);
    if (old_clusters == NULL) {
        return 0;
    }


    if (old_file_clusters_size > new_file_clusters_size){
        writed_bytes = write_smaller_file(f_data, old_clusters, new_file_clusters_size, old_file_clusters_size, buffer, buffer_size, offset);
    } else if (old_file_clusters_size < new_file_clusters_size){
        writed_bytes = write_bigger_file(f_data, old_clusters, new_file_clusters_size, old_file_clusters_size, buffer, buffer_size, offset);
    } else {
        writed_bytes = write_same_file(f_data, old_clusters, new_file_clusters_size, buffer, buffer_size, offset);
    }

    file -> file_size = (uint32_t) (writed_bytes + offset);

    if(writed_bytes >= 0){
        write_to_dir(f_data->memory, f_data->memory_size, file, (uint32_t) dir_position);
    }

    free(old_clusters);
    return writed_bytes;
}

size_t write_bigger_file(struct fat_data *f_data, uint32_t *old_clusters, uint32_t new_file_clusters_size, uint32_t old_file_clusters_size, char *buffer, unsigned int buffer_size, unsigned long offset){

    uint32_t *new_clusters = NULL;
    uint32_t *clusters = NULL;
    uint32_t *values = NULL;
    unsigned long number_of_new_clusters = 0;
    size_t writed_bytes = 0;
    size_t i = 0;

    // search free space in fat ========================================================================================
    new_clusters = malloc(sizeof(uint32_t) * (new_file_clusters_size - old_file_clusters_size));
	if (!new_clusters) {
		return 0;
	}

    if(find_empty_clusters(f_data->boot_record->usable_cluster_count, f_data->fat1, new_clusters, new_file_clusters_size - old_file_clusters_size) == -1){
        free(new_clusters);
        return 0;
    }

    //write data =======================================================================================================
    clusters = malloc(sizeof(uint32_t) * new_file_clusters_size);
	if (!clusters) {
		free(new_clusters);
		return 0;
	}

    for(i = 0; i < new_file_clusters_size; i++){

        if(i < old_file_clusters_size) {
            clusters[i] = old_clusters[i];
        } else {
            clusters[i] = new_clusters[i - old_file_clusters_size];
        }
    }
    writed_bytes = write_bytes_to_fat(f_data->memory, f_data->memory_size, buffer, buffer_size, offset, clusters, new_file_clusters_size, f_data->start_of_root_dir, f_data->boot_record->cluster_size);
    free(clusters);


    // change FAT table ================================================================================================
    number_of_new_clusters = new_file_clusters_size - old_file_clusters_size;

    values = malloc(sizeof(uint32_t) * number_of_new_clusters);
	if (!values) {
		free(new_clusters);
		return 0;
	}
    create_values_from_clusters(new_clusters, values, number_of_new_clusters);

    // write new clusters to fat
    change_fat(f_data->fat1, new_clusters, values, (uint32_t)number_of_new_clusters);
    change_fat(f_data->fat2, new_clusters, values, (uint32_t)number_of_new_clusters);

    change_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, new_clusters, values,
                          (uint32_t) number_of_new_clusters, f_data->fat_size, f_data->boot_record->fat_copies);

    // redirect last old cluster to new clusters
    values[0] = new_clusters[0];
    change_fat(f_data->fat1, old_clusters + old_file_clusters_size - 1, values, 1);
    change_fat(f_data->fat2, old_clusters + old_file_clusters_size - 1, values, 1);

    change_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, old_clusters + old_file_clusters_size - 1,
                          values, 1, f_data->fat_size, f_data->boot_record->fat_copies);

    // free pointers ===================================================================================================
    free(new_clusters);
    free(values);

    return writed_bytes;
}

size_t write_smaller_file(struct fat_data *f_data, uint32_t *old_clusters, uint32_t new_file_clusters_size, uint32_t old_file_clusters_size, char *buffer, unsigned int buffer_size, unsigned long offset){

	size_t writed_bytes = 0;
	int result = 0;

    writed_bytes = write_bytes_to_fat(f_data->memory, f_data->memory_size, buffer, buffer_size, offset, old_clusters, new_file_clusters_size, f_data->start_of_root_dir, f_data->boot_record->cluster_size);

 	result = remove_unused_clusters(f_data, old_clusters, old_file_clusters_size, new_file_clusters_size);

	if (result) return 0;
    return writed_bytes;
}

size_t write_same_file(struct fat_data *f_data, uint32_t *old_clusters, uint32_t new_file_clusters_size, char *buffer, unsigned int buffer_size, unsigned long offset){
    return write_bytes_to_fat(f_data->memory, f_data->memory_size, buffer, buffer_size, offset, old_clusters,  new_file_clusters_size, f_data->start_of_root_dir, f_data->boot_record->cluster_size);
}

struct dir_file *fat_read_dir(struct fat_data *f_data, uint32_t act_fat_position, uint32_t *files, unsigned long * positions) {
	if (f_data == NULL || f_data->boot_record == NULL) {
        return NULL;
    }

    if(act_fat_position < 0 || act_fat_position >= f_data->boot_record->usable_cluster_count){
        return NULL;
    }

    return get_all_in_dir(f_data->memory, f_data->memory_size, files, positions, 
		f_data->start_of_root_dir + (act_fat_position * f_data->boot_record->cluster_size), f_data->max_dir_entries);
}

int fat_set_file_size(struct fat_data *f_data, struct dir_file * file, size_t file_size, unsigned long dir_position)
{
	uint32_t *clusters = NULL;
	uint32_t clusters_old_size = 0;
	unsigned long clusters_new_size = 0;
	int result = 0;

	size_t new_file_size = file_size ? file_size : 1;

	if (f_data == NULL || f_data->boot_record == NULL || f_data->fat1 == NULL || f_data->fat2 == NULL) {
		return 1; // NOT INITIALIZE
	}
	
	if (file->file_name == NULL ||
		file->file_type == OBJECT_DIRECTORY ||
		file->first_cluster >= f_data->fat_size ||
		file->first_cluster < 0
		) {
		return 8; // ERROR
	}

	// number of clusters with new size
	clusters_new_size = (unsigned long)new_file_size / f_data->boot_record->cluster_size;
	if (new_file_size % f_data->boot_record->cluster_size > 0) {
		clusters_new_size++;
	}

	// get clusters
	clusters = get_file_clusters(file, &clusters_old_size, f_data->boot_record->cluster_size, f_data->fat1, f_data->boot_record->dir_clusters);
	if (clusters == NULL) {
		return 11; // CANT GET CLUSTERS
	}

	result = remove_unused_clusters(f_data, clusters, clusters_old_size, clusters_new_size);
	free(clusters);
	
	if (result) return 12; // OUT OF MEMORY

	file->file_size = (uint32_t)(new_file_size);
	write_to_dir(f_data->memory, f_data->memory_size, file, (uint32_t)dir_position);

	return 0; // SUCCESS
}

/**
 * Vypise obsah adresare ve formatu:
 * +ROOT
 *    +ADRESAR
 *       -FILE 1 2
 *    --
 *    -FILE 3 1
 * --
 * Cislice za nazvem souboru znaci prvni cluster a pocet clusteru.
 * Neobsahuje-li FAT zadne soubory, vypise se na obrazovku:
 * EMPTY
 *
 * @param boot_record zavadec nacteny z FAT
 */
void print_all(struct fat_data *f_data) {
    struct dir_file *files = NULL;
	unsigned int number_of_objects = 0;

    if(f_data == NULL || f_data->boot_record == NULL){
        return;
    }

    files = get_all_in_dir(f_data->memory, f_data->memory_size, &number_of_objects, NULL , f_data->start_of_root_dir, f_data->max_dir_entries);

    if (number_of_objects == 0) {
        printf("EMPTY\n");
        free(files);
        return;
    }

    printf("+ROOT\n");

    print_directory(f_data->memory, f_data->memory_size, files, number_of_objects, f_data->boot_record->cluster_size, f_data->start_of_root_dir, f_data->max_dir_entries, 1);
}

int remove_unused_clusters(struct fat_data *f_data, uint32_t *clusters, uint32_t clusters_old_size, unsigned long clusters_new_size) {
	
	uint32_t *values = NULL;

	unsigned int clusters_to_remove = (unsigned int)(clusters_old_size - clusters_new_size);

	if (clusters_to_remove) {
		// free unused fat clusters
		rm_from_fat(f_data->fat1, clusters + clusters_new_size, clusters_to_remove);
		rm_from_fat(f_data->fat2, clusters + clusters_new_size, clusters_to_remove);
		rm_from_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, clusters + clusters_new_size,
			clusters_to_remove, f_data->fat_size, f_data->boot_record->fat_copies);

		// mark last cluster as last
		values = malloc(sizeof(uint32_t));
		if (!values) {
			return 12; // OUT OF MEMORY
		}
		values[0] = FAT_FILE_END;
		change_fat(f_data->fat1, clusters + clusters_new_size - 1, values, 1);
		change_fat(f_data->fat2, clusters + clusters_new_size - 1, values, 1);

		change_all_physic_fat(f_data->memory, f_data->memory_size, f_data->fat_record_size, f_data->start_of_fat, clusters + clusters_new_size - 1,
			values, 1, f_data->fat_size, f_data->boot_record->fat_copies);

		free(values);
	}

	return 0;
}

struct boot_record *fat_get_boot_record(char *memory, size_t memory_size) {
	return get_boot_record(memory, memory_size);
}