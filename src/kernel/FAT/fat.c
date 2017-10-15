#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "fat_structure.h"
#include "library.h"

FILE *p_file;

uint start_of_fat = 0;
uint fat_record_size = 0;
uint fat_size = 0;
uint start_of_root_dir = 0;
uint max_dir_entries = 0;
uint start_of_data = 0;

struct boot_record *boot_record = NULL;
int32_t *fat1 = NULL;
int32_t *fat2 = NULL;

int init(char * fat_name);

int close_fat();

int create_file(char *data, int data_size, char file_name[], int32_t act_fat_position);

void create_values_from_clusters(const int32_t *clusters, int32_t *values, long size);

void init_object(struct dir_file *object, char name[], int32_t file_size, int8_t file_type, int32_t first_cluster);

int delete_file(char file_name[], int32_t act_fat_position);

int create_dir(char dir_name[], int32_t act_fat_position);

int delete_empty_dir(char dir_name[], int32_t act_fat_position);

char* read_object(int *ret_code, int *data_size, char file_name[], int32_t act_fat_position);

void print_all();

int main(){
    init("../output.fat");

    print_all();

    int result = 0;

    result = create_file("pokus", 6, "pokus", 0);
    printf("%d\n", result);
    print_all();

    int ret_code = 0;
    int data_size = 0;

    char * data = read_object(&ret_code, &data_size, "pokus", 0);
    printf("%d\n", ret_code);
    print_all();
    printf("size: %d\n", data_size);
    printf("data: %s\n", data);

    result = delete_file("pokus",0);
    print_all();
    printf("%d\n", result);

    result = create_dir("dir_pokus", 0);
    print_all();
    printf("%d\n", result);

    result = delete_empty_dir("dir_pokus", 0);
    print_all();
    printf("%d\n", result);


    close_fat();
    pause();
}

int init(char * fat_name){

    p_file = fopen(fat_name, "r+");
    if (p_file == NULL) {
        printf("%d", errno);
        printf("Can't open file\n");
        return -1;
    }
    fseek(p_file, 0, SEEK_SET);

    boot_record = get_boot_record(p_file);

    start_of_fat = boot_record->reserved_cluster_count * boot_record->cluster_size;
    fat_record_size = sizeof(int32_t);
    fat_size = boot_record->usable_cluster_count * fat_record_size;
    start_of_root_dir = start_of_fat + (boot_record->fat_copies * fat_size);
    max_dir_entries = boot_record->cluster_size / sizeof(struct dir_file);
    start_of_data = start_of_root_dir + (boot_record->dir_clusters * boot_record->cluster_size);


    fseek(p_file, start_of_fat, SEEK_SET);
    fat1 = get_fat(p_file, fat_record_size, boot_record->usable_cluster_count, fat_size);
    fat2 = get_fat(p_file, fat_record_size, boot_record->usable_cluster_count, fat_size);
}

int close_fat(){
    // TODO free memory
    fclose(p_file);
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
int create_file(char *data, int data_size, char file_name[], int32_t act_fat_position) {
    struct dir_file *file = NULL;
    struct dir_file new_local_file;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    int32_t position = 0;
    long dir_position = 0;
    long file_cluster_count = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        return 1;
    }

    if(data == NULL || data_size < 0){
        return 10;
    }

    if(strlen(file_name) > 12){
        return 2;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        return 9;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    // check if file exists
    file = get_object_in_dir(p_file, file_name, position, max_dir_entries);
    if (file != NULL) {
        free(file);
        return 4;
    }

    // check empty space in dir
    dir_position = find_empty_space_in_dir(p_file, max_dir_entries, position);
    if (dir_position == -1) {
        return 6;
    }

    // count number of clusters of new file
    file_cluster_count = data_size / boot_record->cluster_size;
    if (data_size % boot_record->cluster_size > 0) {
        file_cluster_count++;
    }

    // search free space in fat
    clusters = malloc(sizeof(int32_t) * file_cluster_count);
    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, clusters, file_cluster_count) == -1){
        free(clusters);
        return 7;
    }


    init_object(&new_local_file, file_name, (int32_t) data_size, OBJECT_FILE, clusters[0]);

    write_file_to_fat(p_file,data, data_size, clusters, (int32_t) file_cluster_count, start_of_data, boot_record->cluster_size);
    write_to_dir(p_file, new_local_file, (int32_t) dir_position);

    values = malloc(sizeof(int32_t) * file_cluster_count);
    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(fat1, clusters, values, (int32_t)file_cluster_count);
    change_fat(fat2, clusters, values, (int32_t)file_cluster_count);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, values, (int32_t) file_cluster_count,
                          fat_size, boot_record->fat_copies);

    free(clusters);
    free(values);

    return 0;
}

/**
 * Podle vstupniho pole clusters udavajici indexy do FAT pro jeden soubor, vytvori pole values predstavujici hodnoty na
 * techto indexech.
 *
 * @param clusters indexy do fat tabulky na kterych je ulozen jeden soubor
 * @param values prazdne pole ktere bude naplneno
 * @param size velikost techto poli
 */
void create_values_from_clusters(const int32_t *clusters, int32_t *values, long size){
    int i = 0;

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
void init_object(struct dir_file *object, char name[], int32_t file_size, int8_t file_type, int32_t first_cluster){
    memset(object->file_name, 0, 12);
    strcpy(object->file_name, name);
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
int delete_file(char file_name[], int32_t act_fat_position) {
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    unsigned int new_value = 0;
    int *clusters = NULL;
    int32_t position = 0;
    uint i = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        return 1;
    }

    if(strlen(file_name) > 12){
        return 2;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        return 9;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    file = get_object_in_dir(p_file, file_name, position, max_dir_entries);
    if (file == NULL || file->file_type != OBJECT_FILE) {
        free(file);
        return 3;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat1,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        free(file);
        return 11;
    }

    fseek(p_file, ftell(p_file) - sizeof(struct dir_file), SEEK_SET);
    for (i = 0; i < sizeof(struct dir_file); i++) {
        fwrite(&new_value, sizeof(char), 1, p_file);
    }

    rm_from_fat(fat1, clusters, file_clusters_size);
    rm_from_fat(fat2, clusters, file_clusters_size);
    rm_from_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, file_clusters_size, fat_size,
                           boot_record->fat_copies);

    free(file);
    free(clusters);

    return 0;
}

/**
 * Vytrovy novy prazdny adresar na zvolenem miste.
 *
 * @param dir_name jmeno hledaneho adresare
 * @param act_fat_position pozice adresare ve kterem hledame adresar
 * @return chybovy kod nebo 0 pri uspechu
 */
int create_dir(char dir_name[], int32_t act_fat_position) {
    struct dir_file *file = NULL;
    struct dir_file new_local_file;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    int32_t position = 0;
    long dir_position = 0;
    long file_cluster_count = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        return 1;
    }

    if(strlen(dir_name) > 12){
        return 2;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        return 9;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    file = get_object_in_dir(p_file, dir_name, position, max_dir_entries);
    if (file != NULL) {
        free(file);
        return 4;
    }

    dir_position = find_empty_space_in_dir(p_file, max_dir_entries, position);
    if (dir_position == -1) {
        return 6;
    }

    file_cluster_count = boot_record->dir_clusters;
    clusters = malloc(sizeof(int32_t) * file_cluster_count);
    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, clusters, file_cluster_count) == -1){
        free(clusters);
        return 7;
    }


    init_object(&new_local_file, dir_name, 0, OBJECT_DIRECTORY, clusters[0]);

    write_empty_dir_to_fat(p_file, clusters, (int32_t) file_cluster_count, start_of_data, boot_record->cluster_size);
    write_to_dir(p_file, new_local_file, (int32_t) dir_position);

    values = malloc(sizeof(int32_t) * file_cluster_count);
    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(fat1, clusters, values, (int32_t)file_cluster_count);
    change_fat(fat2, clusters, values, (int32_t)file_cluster_count);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, values, (int32_t) file_cluster_count,
                          fat_size, boot_record->fat_copies);

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
int delete_empty_dir(char dir_name[], int32_t act_fat_position) {
    long position = 0;
    int32_t parent_position = 0;
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    int result = 0;
    uint i = 0;
    unsigned int new_value = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        return 1;
    }

    if(strlen(dir_name) > 12){
        return 2;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        return 9;
    }

    parent_position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    file = get_object_in_dir(p_file, dir_name, parent_position, max_dir_entries);
    if (file == NULL || file->file_type != OBJECT_DIRECTORY) {
        free(file);
        return 3;
    }
    position = ftell(p_file);

    result = is_dir_empty(p_file, max_dir_entries, start_of_data + (file->first_cluster * boot_record->cluster_size));
    if (result == 0) {
        free(file);
        return 5;
    } else if (result == -1) {
        free(file);
        return 8;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat1,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        free(file);
        return 11;
    }

    rm_from_fat(fat1, clusters, file_clusters_size);
    rm_from_fat(fat2, clusters, file_clusters_size);

    rm_from_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, file_clusters_size, fat_size,
                           boot_record->fat_copies);

    fseek(p_file, position - sizeof(struct dir_file), SEEK_SET);
    for (i = 0; i < sizeof(struct dir_file); i++) {
        fwrite(&new_value, sizeof(char), 1, p_file);
    }

    free(file);
    free(clusters);

    return 0;
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
char* read_object(int *ret_code, int *data_size, char file_name[], int32_t act_fat_position) {
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    char *cluster = NULL;
    char *data = NULL;
    int32_t position = 0;
    int32_t read_size = 0;
    int32_t writed_size = 0;
    int i = 0;

    if(boot_record == NULL || fat1 == NULL){
        *ret_code = 1;
        return NULL;
    }

    if(strlen(file_name) > 12){
        *ret_code = 2;
        return NULL;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        *ret_code = 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    file = get_object_in_dir(p_file, file_name, position, max_dir_entries);
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
        fseek(p_file, clusters[i] * boot_record->cluster_size + start_of_data, SEEK_SET);
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
void print_all() {
    struct dir_file *files = NULL;
    int number_of_objects = 0;

    if(boot_record == NULL){
        return;
    }

    files = get_all_in_dir(p_file, &number_of_objects, NULL , start_of_root_dir, max_dir_entries);

    if (number_of_objects == 0) {
        printf("EMPTY\n");
        free(files);
        return;
    }

    printf("+ROOT\n");

    print_directory(p_file, files, number_of_objects, boot_record->cluster_size, start_of_data, max_dir_entries, 1);
}