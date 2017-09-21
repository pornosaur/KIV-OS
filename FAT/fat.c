#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fat_structure.h"
#include "library.h"

FILE *p_file;

uint start_of_fat = 0;
uint fat_record_size = 0;
uint fat_size = 0;
uint start_of_root_dir = 0;
uint max_dir_entries = 0;
uint start_of_data = 0;


void create_file(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char file_name[], char file_path[]);

int32_t get_directory_position(struct boot_record *boot_record, char file_path[]);

void create_values_from_clusters(int32_t *clusters, int32_t *values, long size);

void init_object(struct dir_file *object, char name[], int32_t file_size, int8_t file_type, int32_t first_cluster);

void delete_file(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char file_path[]);

void print_clusters(struct boot_record *boot_record, int32_t *fat, char file_path[]);

void create_dir(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char dir_name[], char dir_path[]);

void delete_empty_dir(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char dir_path[]);

void print_file(struct boot_record *boot_record, int32_t *fat, char file_path[]);

void print_all(struct boot_record *boot_record);

int check_arguments(int argc, int number_of_arguments);

void crossroad(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, int argc, char *argv[]);

/**
 * Nacte potrebna data z FAT, initializuje potrabne hodnoty.
 *
 * @param argc pocet vstupnich argumentu
 * @param argv vstupni argumenty
 * @return -1 pri chybe, jinak vraci 0
 */
int main(int argc, char *argv[]) {
    struct boot_record *boot_record = NULL;
    int32_t *fat1 = NULL;
    int32_t *fat2 = NULL;

    if (argc < 3) {
        printf("Invalid number of parameters\n");
        return -1;
    }

    if (argv[2][0] != '-') {
        printf("Invalid parameter\n");
        return -1;
    }

    p_file = fopen(argv[1], "r+");
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

    crossroad(boot_record, fat1, fat2, argc, argv);

    fclose(p_file);
}

/**
 * Zkontroluje, zda se vstpuni hodnoty rovnaji. Pokud se nerovnaji, vypise se hlaska a vrati 1.
 *
 * @param argc pocet vstupnich argumentu programu.
 * @param number_of_arguments pocet pozadovanuch argumentu
 * @return 0 rovnaji-li se, jinak vraci 1
 */
int check_arguments(int argc, int number_of_arguments) {
    if (argc == number_of_arguments) {
        return 0;
    } else {
        printf("Invalid number of parameters\n");
        return 1;
    }
}

/**
 * Rozhoduje, dle tretiho vstupniho argumentu programu, ktera operace se vykona.
 * @param boot_record zavadec nacten z FAT
 *
 * @param fat1 prvni kopie FAT tabulky
 * @param fat2 druha kopie FAT tabulky
 * @param argc pocet vstupnich argumentu programu
 * @param argv vstupni argumenty programu
 */
void crossroad(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, int argc, char *argv[]) {
    switch (argv[2][1]) {
        case 'a':
            if (!check_arguments(argc, 5))
                create_file(boot_record, fat1, fat2, argv[3], argv[4]);
            break;
        case 'f':
            if (!check_arguments(argc, 4))
                delete_file(boot_record, fat1, fat2, argv[3]);
            break;
        case 'c':
            if (!check_arguments(argc, 4))
                print_clusters(boot_record, fat1, argv[3]);
            break;
        case 'm':
            if (!check_arguments(argc, 5))
                create_dir(boot_record, fat1, fat2, argv[3], argv[4]);
            break;
        case 'r':
            if (!check_arguments(argc, 4))
                delete_empty_dir(boot_record, fat1, fat2, argv[3]);
            break;
        case 'l':
            if (!check_arguments(argc, 4))
                print_file(boot_record, fat1, argv[3]);
            break;
        case 'p':
            if (!check_arguments(argc, 3))
                print_all(boot_record);
            break;
        default:
            printf("Invalid parameter\n");
            break;
    }
}

/**
 * Vlozi soubor do fat. Funkce muze vypsat tato hlaseni.
 *
 * NAME IS TOO LONG - jmeno noveho souboru je prilis dlouhe
 * PATH NOT FOUND - umisteni adresare, kam ma byt soubor vlozeni, nebylo nalezeno
 * PATH EXISTS - soubor se stejnym nazvem jiz na zvolenem miste existuje
 * DIRECTORY IS FULL - aktualni slozka jiz obsahuje maximalni pocet prvku
 * FAT IS FULL - neni dostatek volnych cluteru pro vlozeni souboru
 * FILE NOT FOUND - nebyl nalezen soubor, ktery se ma nahravat
 * OK - vse probehlo v poradku
 *
 * @param boot_record zavadec nacten z FAT
 * @param fat1 prvni kopie FAT tabulky
 * @param fat2 druha kopie FAT tabulky
 * @param file_name nazev nahravaneho souboru
 * @param file_path cesta ve fat, kde bude soubor ulozen
 */
void create_file(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char file_name[], char file_path[]) {
    struct dir_file *file = NULL;
    struct dir_file new_local_file;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    int32_t position = 0;
    long dir_position = 0;
    long file_size = 0;
    long file_cluster_count = 0;
    FILE *new_file = NULL;

    if(strlen(file_name) > 12){
        printf("NAME IS TOO LONG\n");
        return;
    }

    position = get_directory_position(boot_record, file_path);
    if(position < 0){
        printf("PATH NOT FOUND\n");
        return;
    }

    file = get_object_in_dir(p_file, file_name, position, max_dir_entries);
    if (file != NULL) {
        printf("PATH EXISTS\n");
        free(file);
        return;
    }

    dir_position = find_empty_space_in_dir(p_file, max_dir_entries, position);
    if (dir_position == -1) {
        printf("DIRECTORY IS FULL\n");
        return;
    }

    new_file = fopen(file_name, "r");
    if (!new_file) {
        printf("FILE NOT FOUND\n");
        return;
    }

    fseek(new_file, 0, SEEK_END);
    file_size = ftell(new_file);
    rewind(new_file);
    file_cluster_count = file_size / boot_record->cluster_size;
    if (file_size % boot_record->cluster_size > 0) {
        file_cluster_count++;
    }


    clusters = malloc(sizeof(int32_t) * file_cluster_count);

    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, clusters, file_cluster_count) == -1){
        printf("FAT IS FULL\n");
        free(clusters);
        fclose(new_file);
        return;
    }


    init_object(&new_local_file, file_name, (int32_t) file_size, OBJECT_FILE, clusters[0]);


    write_file_to_fat(p_file, new_file, clusters, (int32_t) file_cluster_count, start_of_data, boot_record->cluster_size);
    write_to_dir(p_file, new_local_file, (int32_t) dir_position);

    values = malloc(sizeof(int32_t) * file_cluster_count);
    create_values_from_clusters(clusters, values, file_cluster_count);

    change_fat(fat1, clusters, values, (int32_t)file_cluster_count);
    change_fat(fat2, clusters, values, (int32_t)file_cluster_count);

    change_all_physic_fat(p_file, fat_record_size, start_of_fat, clusters, values, (int32_t) file_cluster_count,
                          fat_size, boot_record->fat_copies);

    free(clusters);
    free(values);
    fclose(new_file);

    printf("OK\n");
}

/**
 * Nalezne pozici v souboru kde zacina zaznam slozky danou vstupni cestou.
 *
 * @param boot_record zavadec nacten z FAT
 * @param file_path cesta ke slozce
 * @return pozici slozky v souboru (od zacatku souboru), -1 pri chybe
 */
int32_t get_directory_position(struct boot_record *boot_record, char file_path[]){
    struct dir_file *directory;
    int32_t position = 0;

    if(strcmp(file_path, "/")) {
        directory = find_file(p_file, boot_record, file_path, start_of_root_dir, start_of_data, max_dir_entries);
        if (directory == NULL || directory->file_type != OBJECT_DIRECTORY) {
            return -1;
        }
        position = start_of_data + (directory->first_cluster * boot_record->cluster_size);
        free(directory);
    }else{
        position = start_of_root_dir;
    }

    return position;
}

/**
 * Podle vstupniho pole clusters udavajici indexy do FAT pro jeden soubor, vytvori pole values predstavujici hodnoty na
 * techto indexech.
 *
 * @param clusters indexy do fat tabulky na kterych je ulozen jeden soubor
 * @param values prazdne pole ktere bude naplneno
 * @param size velikost techto poli
 */
void create_values_from_clusters(int32_t *clusters, int32_t *values, long size){
    int i = 0;

    for(i = 1; i < size; i++){
        values[i-1] = clusters[i];
    }
    values[size-1] = FAT_FILE_END;
}

/**
 * Nastavi vstupni strukture objekt parametry, ktere jsou predany jako ostatni parametry.
 *
 * @param objekt struktura, ktera se bude nastavovat
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
 * Smaze soubor ve FAT dany vstupni cestou. Neni-li soubor nalezen vypise PATH NOT FOUND.
 *
 * @param boot_record zavadec nacteny z FAT
 * @param fat1 prvni kopie FAT tabulky
 * @param fat2 druha kopie FAT tabulky
 * @param file_path cesta k souboru, ktery se ma smazat
 */
void delete_file(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char file_path[]) {
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    unsigned int new_value = 0;
    int *clusters = NULL;
    uint i = 0;

    file = find_file(p_file, boot_record, file_path, start_of_root_dir, start_of_data, max_dir_entries);
    if (file == NULL || file->file_type != OBJECT_FILE) {
        printf("PATH NOT FOUND\n");
        return;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat1,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        return;
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

    printf("OK\n");
}

/**
 * Vypise veskere clustery souboru v poradi, v jakem jdou za sebou. Neni-li soubor nalezen vypise PATH NOT FOUND.
 *
 * @param boot_record zavadec nacteny z FAT
 * @param fat FAT tabulka
 * @param file_path cesta k souboru, ktery se bude zpracovavat
 */
void print_clusters(struct boot_record *boot_record, int32_t *fat, char file_path[]) {
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    int i = 0;

    file = find_file(p_file, boot_record, file_path, start_of_root_dir, start_of_data, max_dir_entries);
    if (file == NULL) {
        printf("PATH NOT FOUND\n");
        return;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        return;
    }

    free(file);

    printf("%s ", file_path);
    for (i = 0; i < file_clusters_size; i++) {
        printf("%d", clusters[i]);
        printf("%c", i + 1 == file_clusters_size ? '\n' : ':');
    }

    free(clusters);
}

/**
 * Vytrovy novy prazdny adresar na zvolenem miste. Funkce muze vypsat tato hlaseni:
 *
 * NAME IS TOO LONG - jmeno noveho adresare je prilis dlouhe
 * PATH NOT FOUND - umisteni adresare nebylo nalezeno
 * PATH EXISTS - adresar jiz na zvolenem miste existuje
 * DIRECTORY IS FULL - aktualni slozka jiz obsahuje maximalni pocet prvku
 * FAT IS FULL - neni dostatek volnych cluteru pro vytvoreni slozky
 * OK - vse probehlo v poradku
 *
 * @param boot_record zavadec nacteny z FAT
 * @param fat1 prvni kopie fat tabulky
 * @param fat2 druha kopie fat tabulky
 * @param dir_name nazev nove slozky
 * @param dir_path cesta, kde bude slozka vytvorena
 */
void create_dir(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char dir_name[], char dir_path[]) {
    struct dir_file *file = NULL;
    struct dir_file new_local_file;
    int32_t *clusters = NULL;
    int32_t *values = NULL;
    int32_t position = 0;
    long dir_position = 0;
    long file_cluster_count = 0;

    if(strlen(dir_name) > 12){
        printf("NAME IS TOO LONG\n");
        return;
    }

    position = get_directory_position(boot_record, dir_path);
    if(position < 0){
        printf("PATH NOT FOUND\n");
        return;
    }

    file = get_object_in_dir(p_file, dir_name, position, max_dir_entries);
    if (file != NULL) {
        printf("PATH EXISTS\n");
        free(file);
        return;
    }

    dir_position = find_empty_space_in_dir(p_file, max_dir_entries, position);
    if (dir_position == -1) {
        printf("DIRECTORY IS FULL\n");
        return;
    }


    file_cluster_count = boot_record->dir_clusters;

    clusters = malloc(sizeof(int32_t) * file_cluster_count);

    if(find_empty_clusters(boot_record->usable_cluster_count, fat1, clusters, file_cluster_count) == -1){
        printf("FAT IS FULL\n");
        free(clusters);
        return;
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

    printf("OK\n");
}

/**
 * Smaze prazdny adresar ve FAT. Neni-li adresar nalezen je vypsana hlaska PATH NOT FOUND. Obsahuje-li slozka
 * soubory, nebo jine adresare je vypsana hlaska PATH NOT EMPTY. Pri uspechu je vypsano OK.
 *
 * @param boot_record zavadec nacteny z FAT
 * @param fat1 prvni kopie fat tabulky
 * @param fat2 druha kopie fat tabulky
 * @param dir_path cesta k adresari
 */
void delete_empty_dir(struct boot_record *boot_record, int32_t *fat1, int32_t *fat2, char dir_path[]) {
    long position = 0;
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    int result = 0;
    uint i = 0;
    unsigned int new_value = 0;

    file = find_file(p_file, boot_record, dir_path, start_of_root_dir, start_of_data, max_dir_entries);
    if (file == NULL || file->file_type != OBJECT_DIRECTORY) {
        printf("PATH NOT FOUND\n");
        free(file);
        return;
    }
    position = ftell(p_file);

    result = is_dir_empty(p_file, max_dir_entries, start_of_data + (file->first_cluster * boot_record->cluster_size));
    if (result == 0) {
        printf("PATH NOT EMPTY\n");
        free(file);
        return;
    } else if (result == -1) {
        printf("ERROR\n");
        free(file);
        return;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat1,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        return;
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

    printf("OK\n");
}

/**
 * Vypise na obrazovku obsah souboru daneho cestou. Vystup je ve tvaru:
 * nazev_souboru: text souboru
 *
 * Neni-li soubor nalezen vypise se:
 * PATH NOT FOUND
 *
 * @param boot_record zavadec nacteny z FAT
 * @param fat FAT tabulka
 * @param file_path cesta k souboru, ktery se ma vytisknout
 */
void print_file(struct boot_record *boot_record, int32_t *fat, char file_path[]) {
    int32_t file_clusters_size = 0;
    struct dir_file *file = NULL;
    int *clusters = NULL;
    char *cluster = NULL;
    int i = 0;


    file = find_file(p_file, boot_record, file_path, start_of_root_dir, start_of_data, max_dir_entries);
    if (file == NULL || file->file_type != OBJECT_FILE) {
        printf("PATH NOT FOUND\n");
        free(file);
        return;
    }

    clusters = get_file_clusters(file, &file_clusters_size, boot_record->cluster_size, fat,
                                 boot_record->dir_clusters);
    if (clusters == NULL) {
        free(file);
        return;
    }

    cluster = malloc(sizeof(char) * boot_record->cluster_size);
    printf("%s: ", file_path);
    for (i = 0; i < file_clusters_size; i++) {
        fseek(p_file, clusters[i] * boot_record->cluster_size + start_of_data, SEEK_SET);
        fread(cluster, sizeof(char) * boot_record->cluster_size, 1, p_file);
        if (cluster[0] != '\0')
            printf("%s", cluster);
    }
    printf("\n");

    free(file);
    free(clusters);
    free(cluster);
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
void print_all(struct boot_record *boot_record) {
    struct dir_file *files = NULL;
    int number_of_objects = 0;

    files = get_all_in_dir(p_file, &number_of_objects, NULL , start_of_root_dir, max_dir_entries);

    if (number_of_objects == 0) {
        printf("EMPTY\n");
        free(files);
        return;
    }

    printf("+ROOT\n");

    print_directory(p_file, files, number_of_objects, boot_record->cluster_size, start_of_data, max_dir_entries, 1);
}