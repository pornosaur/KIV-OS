#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fat_structure.h"
#include "library.h"
#include "fat.h"

long write_bigger_file(int32_t *old_clusters, long new_file_clusters_size, int32_t old_file_clusters_size, char *buffer, int buffer_size, long offset);

long write_smaller_file(int32_t *old_clusters, long new_file_clusters_size, int32_t old_file_clusters_size, char *buffer, int buffer_size, long offset);

long write_same_file(int32_t *old_clusters, long new_file_clusters_size, char *buffer, int buffer_size, long offset);

void create_values_from_clusters(const int32_t *clusters, int32_t *values, long size);

void init_object(struct dir_file *object,const char name[], int32_t file_size, int8_t file_type, int32_t first_cluster);

void print_all();



int main(){
    fat_init("../output.fat");

    fat_delete_file_by_name("pokus", 0);


    print_all();

//    long dir_position = 0;
//    struct dir_file* new_file;
//    long red = 0;
//
//    new_file = fat_create_file("pokus", 0, &dir_position);
//    printf("%s  %d  %d\n",new_file->file_name, new_file->file_size, new_file->file_type);
//
//    print_all();
//
//    fat_write_file(new_file, dir_position, "textik, ktery se ma zapsat aaaaaaaaaa bbbbbbbbbb ccccccccccc dddddddddd eeeeeeeeee ffffffffff gggggggggg hhhhhhhhhh jjjjjjjjjj kkkkkkkkkk llllllllll", 148, 0);
//
//    print_all();
//
//    char *buffer = malloc(sizeof(char) * 100);
//    red = fat_read_file(*new_file, buffer, 100, 0);
//    printf("%li - %.*s\n", red, (int) red, buffer);
//    red = fat_read_file(*new_file, buffer, 100, 100);
//    printf("%li - %.*s\n", red, (int) red, buffer);

    int data_size = 0;
    int ret_code = 0;
    printf("%d - %.*s\n",data_size,data_size, read_object(&ret_code,&data_size, "b.txt", 0));



//    int ret_code = 0;
//    int data_size = 0;
//
//    char * data = read_object(&ret_code, &data_size, "pokus", 0);
//    printf("%d\n", ret_code);
//    print_all();
//    printf("size: %d\n", data_size);
//    printf("data: %s\n", data);
//
//    result = delete_file("pokus",0);
//    print_all();
//    printf("%d\n", result);
//
//    result = fat_create_dir("dir_pokus", 0);
//    print_all();
//    printf("%d\n", result);
//
//    result = fat_delete_empty_dir("dir_pokus", 0);
//    print_all();
//    printf("%d\n", result);


    close_fat();
    //pause();
}

int fat_init(const char * fat_path){

	errno_t err;

    err = fopen_s(&p_file, fat_path, "r+");
    if (err != 0) {
        printf("%d", err);
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
    free(boot_record);
    free(fat1);
    free(fat2);

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
struct dir_file *fat_create_file(const char *file_name, int32_t act_fat_position, long *dir_position) {
    struct dir_file *file = NULL;
    struct dir_file *new_local_file;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    int32_t position = 0;
    long file_cluster_count = 0;
    int result = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        //return 1;
        return NULL;
    }

    if(strlen(file_name) > 12){
        //return 2;
        return NULL;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        //return 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    // check if file exists
    file = get_object_in_dir(p_file, file_name, position, max_dir_entries);
    if (file != NULL) {
        if(file->file_type == OBJECT_FILE) {
            result = fat_delete_file_by_file(file, ftell(p_file) - sizeof(struct dir_file));
            if (result) {
                //return result;
                return NULL;
            }
        } else {
            //return -1; /* Cant delete directory when i want create file */
            return NULL;
        }
    }

    // check empty space in dir
    *dir_position = find_empty_space_in_dir(p_file, max_dir_entries, position);
    if (*dir_position == -1) {
        //return 6;
        return NULL;
    }

    // number of clusters of new file
    file_cluster_count = 1;

    // search free space in fat
    clusters = malloc(sizeof(int32_t) * file_cluster_count);
    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, clusters, file_cluster_count) == -1){
        free(clusters);
        //return 7;
        return NULL;
    }

    new_local_file = malloc(sizeof(struct dir_file));
    init_object(new_local_file, file_name, 0, OBJECT_FILE, clusters[0]);

    write_to_dir(p_file, *new_local_file, (int32_t) *dir_position);

    values = malloc(sizeof(int32_t) * file_cluster_count);
    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(fat1, clusters, values, (int32_t)file_cluster_count);
    change_fat(fat2, clusters, values, (int32_t)file_cluster_count);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, values, (int32_t) file_cluster_count,
                          fat_size, boot_record->fat_copies);

    free(clusters);
    free(values);

    return new_local_file;
}

struct dir_file *fat_get_object_info_by_name(const char *file_name, int32_t act_fat_position, long *dir_position){
    int32_t position = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        //return 1;
        return NULL;
    }

    if(strlen(file_name) > 12){
        //return 2;
        return NULL;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        //return 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    // check if file exists
    struct dir_file *dirFile = get_object_in_dir(p_file, file_name, position, max_dir_entries);
    *dir_position = ftell(p_file) - sizeof(struct dir_file);

    return dirFile;
}

struct dir_file *get_file_info(char file_name[], int32_t act_fat_position){
    int32_t position = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
        //return 1;
        return NULL;
    }

    if(strlen(file_name) > 12){
        //return 2;
        return NULL;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        //return 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    return get_object_in_dir(p_file, file_name, position, max_dir_entries);
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
void init_object(struct dir_file *object,const char name[], int32_t file_size, int8_t file_type, int32_t first_cluster){
    memset(object->file_name, 0, 12);
    strcpy_s(object->file_name, 12, name);
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
int fat_delete_file_by_name(const char *file_name, int32_t act_fat_position) {

    struct dir_file *file = NULL;


    int32_t position = 0;

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

    return fat_delete_file_by_file(file, ftell(p_file) - sizeof(struct dir_file));
}

int fat_delete_file_by_file(struct dir_file *file, long position){
    int *clusters = NULL;
    int32_t file_clusters_size = 0;
    unsigned int i;
    unsigned int new_value = 0;

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

    fseek(p_file, position, SEEK_SET);
    for (i = 0; i < sizeof(struct dir_file); i++) {
        fwrite(&new_value, sizeof(char), 1, p_file);
    }

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
struct dir_file *fat_create_dir(const char *dir_name, int32_t act_fat_position, long *dir_position) {
    struct dir_file *file = NULL;
    struct dir_file *new_local_file;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    int32_t position = 0;
    long file_cluster_count = 0;

    if(boot_record == NULL || fat1 == NULL || fat2 == NULL){
//        return 1;
        return NULL;
    }

    if(strlen(dir_name) > 12){
//        return 2;
        return NULL;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
//        return 9;
        return NULL;
    }

    position = start_of_root_dir + (act_fat_position * boot_record->cluster_size);

    file = get_object_in_dir(p_file, dir_name, position, max_dir_entries);
    if (file != NULL) {
        free(file);
//        return 4;
        return NULL;
    }

    *dir_position = find_empty_space_in_dir(p_file, max_dir_entries, position);
    if (*dir_position == -1) {
//        return 6;
        return NULL;
    }

    file_cluster_count = boot_record->dir_clusters;
    clusters = malloc(sizeof(int32_t) * file_cluster_count);
    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, clusters, file_cluster_count) == -1){
        free(clusters);
//        return 7;
        return NULL;
    }


    new_local_file = malloc(sizeof(struct dir_file));
    init_object(new_local_file, dir_name, 0, OBJECT_DIRECTORY, clusters[0]);

    write_empty_dir_to_fat(p_file, clusters, (int32_t) file_cluster_count, start_of_data, boot_record->cluster_size);
    write_to_dir(p_file, *new_local_file, (int32_t) *dir_position);

    values = malloc(sizeof(int32_t) * file_cluster_count);
    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(fat1, clusters, values, (int32_t)file_cluster_count);
    change_fat(fat2, clusters, values, (int32_t)file_cluster_count);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, values, (int32_t) file_cluster_count,
                          fat_size, boot_record->fat_copies);

    free(clusters);
    free(values);

    return new_local_file;
}

/**
 * Smaze prazdny adresar ve FAT.
 *
 * @param dir_name jmeno hledaneho adresare
 * @param act_fat_position pozice adresare ve kterem hledame adresar
 * @return chybovy kod nebo 0 pri uspechu
 */
int fat_delete_empty_dir(const char *dir_name, int32_t act_fat_position) {
    long position = 0;
    int32_t parent_position = 0;
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    int result = 0;
    unsigned int i = 0;
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
char* read_object(int *ret_code, int *data_size,const char file_name[], int32_t act_fat_position) {
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

long fat_read_file(struct dir_file file, char *buffer, int buffer_size, long offset) {
    int32_t file_clusters_size = 0;
    int *clusters = NULL;
    char *cluster = NULL;
    int32_t read_size = 0;
    int32_t act_read_size = 0;
    int32_t writed_size = 0;
    int i = 0;
    int first_cluster = 0;
    int cluster_offset = 0;


    if(
            boot_record == NULL ||
            file.file_name == NULL ||
            buffer == NULL ||
            buffer_size <= 0 ||
            file.file_size < offset ||
            file.file_type == OBJECT_DIRECTORY ||
            file.first_cluster >= fat_size ||
            file.first_cluster < 0 ||
            file.file_size == 0
            ){

        return 0;
    }

    clusters = get_file_clusters(&file, &file_clusters_size, boot_record->cluster_size, fat1, boot_record->dir_clusters);
    if (clusters == NULL) {
        return 0;
    }


    cluster = malloc(sizeof(char) * boot_record->cluster_size);
    read_size = sizeof(char) * boot_record->cluster_size;

    first_cluster = (int)(offset / boot_record->cluster_size);
    cluster_offset = (int)(offset % boot_record->cluster_size);

    writed_size = 0;
    for (i = first_cluster; i < file_clusters_size; i++) {

        fseek(p_file, clusters[i] * boot_record->cluster_size + start_of_data + cluster_offset, SEEK_SET);

        act_read_size = read_size - cluster_offset;
        if(writed_size + act_read_size > buffer_size){
            act_read_size = buffer_size - writed_size;
        }
        if(offset + writed_size + act_read_size > file.file_size){
            act_read_size = (int32_t) (file.file_size - writed_size - offset);
        }

        if(act_read_size == 0){
            break;
        }

        fread(buffer + writed_size, (size_t) act_read_size, 1, p_file);

        writed_size += act_read_size;
        cluster_offset = 0;
    }

    free(clusters);
    free(cluster);

    return writed_size;
}

long fat_write_file(struct dir_file *file, long dir_position, char *buffer, int buffer_size, long offset){

    int32_t old_file_clusters_size = 0;
    int32_t *old_clusters = NULL;
    long new_file_clusters_size = 0;
    long writed_bytes = 0;

    if(
            boot_record == NULL ||
            fat1 == NULL ||
            fat2 == NULL ||
            file -> file_name == NULL ||
            buffer == NULL ||
            buffer_size <= 0 ||
            offset < 0 ||
            file -> file_type == OBJECT_DIRECTORY ||
            file -> first_cluster >= fat_size ||
            file -> first_cluster < 0
            ){

        return 0;
    }

    // number of clusters of new file
    new_file_clusters_size = (offset + buffer_size) / boot_record -> cluster_size;
    if((offset + buffer_size) % boot_record -> cluster_size > 0){
        new_file_clusters_size ++;
    }

    // get old clusters
    old_clusters = get_file_clusters(file, &old_file_clusters_size, boot_record->cluster_size, fat1, boot_record->dir_clusters);
    if (old_clusters == NULL) {
        return 0;
    }


    if (old_file_clusters_size > new_file_clusters_size){
        writed_bytes = write_smaller_file(old_clusters, new_file_clusters_size, old_file_clusters_size, buffer, buffer_size, offset);
    } else if (old_file_clusters_size < new_file_clusters_size){
        writed_bytes = write_bigger_file(old_clusters, new_file_clusters_size, old_file_clusters_size, buffer, buffer_size, offset);
    } else {
        writed_bytes = write_same_file(old_clusters, new_file_clusters_size, buffer, buffer_size, offset);
    }

    file -> file_size = (int32_t) (writed_bytes + offset);

    if(writed_bytes >= 0){
        write_to_dir(p_file, *file, (int32_t) dir_position);
    }

    free(old_clusters);
    return writed_bytes;
}

long write_bigger_file(int32_t *old_clusters, long new_file_clusters_size, int32_t old_file_clusters_size, char *buffer, int buffer_size, long offset){

    int32_t *new_clusters = NULL;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    long number_of_new_clusters = 0;
    long writed_bytes = 0;
    int i = 0;

    // search free space in fat ========================================================================================
    new_clusters = malloc(sizeof(int32_t) * (new_file_clusters_size - old_file_clusters_size));
    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, new_clusters, new_file_clusters_size - old_file_clusters_size) == -1){
        free(new_clusters);
        //return 7;
        return -1;
    }

    //write data =======================================================================================================
    clusters = malloc(sizeof(int32_t) * new_file_clusters_size);
    for(i = 0; i < new_file_clusters_size; i++){

        if(i < old_file_clusters_size) {
            clusters[i] = old_clusters[i];
        } else {
            clusters[i] = new_clusters[i - old_file_clusters_size];
        }
    }
    writed_bytes = write_bytes_to_fat(p_file, buffer, buffer_size, offset, clusters, (int32_t) new_file_clusters_size, start_of_data, boot_record->cluster_size);
    free(clusters);


    // change FAT table ================================================================================================
    number_of_new_clusters = new_file_clusters_size - old_file_clusters_size;

    values = malloc(sizeof(int32_t) * number_of_new_clusters);
    create_values_from_clusters(new_clusters, values, number_of_new_clusters);

    // write new clusters to fat
    change_fat(fat1, new_clusters, values, (int32_t)number_of_new_clusters);
    change_fat(fat2, new_clusters, values, (int32_t)number_of_new_clusters);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, new_clusters, values,
                          (int32_t) number_of_new_clusters, fat_size, boot_record->fat_copies);

    // redirect last old cluster to new clusters
    values[0] = new_clusters[0];
    change_fat(fat1, old_clusters + old_file_clusters_size - 1, values, 1);
    change_fat(fat2, old_clusters + old_file_clusters_size - 1, values, 1);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, old_clusters + old_file_clusters_size - 1,
                          values, 1, fat_size, boot_record->fat_copies);

    // free pointers ===================================================================================================
    free(new_clusters);
    free(values);

    return writed_bytes;
}

long write_smaller_file(int32_t *old_clusters, long new_file_clusters_size, int32_t old_file_clusters_size, char *buffer, int buffer_size, long offset){

    int32_t *values = NULL;
    long writed_bytes = 0;
    int clusters_to_remove = 0;

    writed_bytes = write_bytes_to_fat(p_file, buffer, buffer_size, offset, old_clusters, (int32_t) new_file_clusters_size, start_of_data, boot_record->cluster_size);

    // change FAT table
    clusters_to_remove = (int)(old_file_clusters_size - new_file_clusters_size);

    // free unused fat clusters
    rm_from_fat(fat1, old_clusters + new_file_clusters_size, clusters_to_remove);
    rm_from_fat(fat2, old_clusters + new_file_clusters_size, clusters_to_remove);
    rm_from_all_physic_fat(p_file, fat_record_size, start_of_fat, old_clusters + new_file_clusters_size,
                           clusters_to_remove, fat_size, boot_record->fat_copies);

    // mark last cluster as last
    values = malloc(sizeof(int32_t));
    values[0] = FAT_FILE_END;
    change_fat(fat1, old_clusters + new_file_clusters_size - 1, values, 1);
    change_fat(fat2, old_clusters + new_file_clusters_size - 1, values, 1);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, old_clusters + new_file_clusters_size - 1,
                          values, 1, fat_size, boot_record->fat_copies);

    free(values);
    return writed_bytes;
}

long write_same_file(int32_t *old_clusters, long new_file_clusters_size, char *buffer, int buffer_size, long offset){
    return write_bytes_to_fat(p_file, buffer, buffer_size, offset, old_clusters, (int32_t) new_file_clusters_size, start_of_data, boot_record->cluster_size);
}

struct dir_file *fat_read_dir(int32_t act_fat_position, int32_t *files) {
    if(boot_record == NULL){
        return NULL;
    }

    if(act_fat_position > 0 && act_fat_position < boot_record->usable_cluster_count){
        return NULL;
    }

    return get_all_in_dir(p_file, files, NULL, act_fat_position, max_dir_entries);
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