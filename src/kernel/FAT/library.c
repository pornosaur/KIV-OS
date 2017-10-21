#include <stdio.h>
#include <string.h>
#include "library.h"


/**
 * Nacte strukturu boot_record ze vstupiho souboru. Soubor musi mit ukazatel nastaven na zacatek boot_record.
 * @param p_file soubor obsahujici fat
 * @return vyplnenou strukturu boot_record
 */
struct boot_record *get_boot_record(FILE *p_file) {

    struct boot_record *boot_record = (struct boot_record *) malloc(sizeof(struct boot_record));
    fread(boot_record, sizeof(struct boot_record), 1, p_file);

    return boot_record;
}

/**
 * Nacte fat tabulku ze vstupniho souboru. Soubor musi mit ukazatel nastaven na zacatek fat tabulky v souboru.
 * @param p_file soubor obashujici fat
 * @param fat_record_size velikost jednoho zaznamu ve fat tabulce
 * @param cluster_count pocet zaznamu ve fat tabulce
 * @param fat_size velikost fat tabulky v bitech
 * @return pole obsahujici fat tabulku
 */
int32_t *get_fat(FILE *p_file, uint fat_record_size, int cluster_count, uint fat_size) {
    int32_t *fat = (int32_t *) malloc(fat_size);

    int i = 0;
    for (i = 0; i < cluster_count; i++) {
        fread(fat + i, fat_record_size, 1, p_file);
    }
    return fat;
}

/**
 * Odstrani veskery obsah clusteru ze souboru na dane pozici.
 * @param p_file soubor, ze ktereho se maze
 * @param start_of_cluster zacatek clusteru, pocitan on zacatku souboru
 * @param cluster_size velikost jednoho clusteru
 */
void delete_cluster(FILE *p_file, uint start_of_cluster, int16_t cluster_size) {
    int i = 0;
    char new_value = 0;

    fseek(p_file, start_of_cluster, SEEK_SET);
    for (i = 0; i < cluster_size; i++) {
        fwrite(&new_value, sizeof(new_value), 1, p_file);
    }
}

/**
 * Najde indexy clusteru ve fat tabulce, na kterych je soubor ulozen.
 * @param file soubor u ktereho se hledaji clutery
 * @param clusters_size pointer na pocet clusteru v souboru, hodnota je nastavovana ve teto funkci
 * @param cluster_size velikost jednoho clusteru
 * @param fat fat tabulka
 * @return NULL pri chybe, jinak indexy clusteru ve fat tabulce.
 */
int *get_file_clusters(struct dir_file *file, int32_t *clusters_size, int16_t cluster_size, int32_t *fat, int16_t dir_clusters) {
    int cluster_position = 0;
    int counter = 0;
    int i = 0;
    int *clusters;


    if(file->file_type == OBJECT_DIRECTORY){
        *clusters_size = dir_clusters;
    }else{
        *clusters_size = file->file_size / cluster_size;
    }

    if (file->file_size % cluster_size) {
        *clusters_size += 1;
    }

    clusters = malloc(sizeof(int) * (*clusters_size) + 1);

    cluster_position = file->first_cluster;

    while (cluster_position != FAT_FILE_END) {
        counter++;
        if (*clusters_size < counter || cluster_position == FAT_BAD_CLUSTER  || cluster_position == FAT_UNUSED) {
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

/**
 * Hleda soubor v dany cestou. Neni-li soubor nalezen, je vracena hodnota NULL.
 *
 * @param p_file soubor obsahujici fat
 * @param boot_record zavadec nacteny z FAT
 * @param file_path cesta k souboru
 * @param start_of_root_dir zacatek root adresare v souboru
 * @param start_of_data zacatek datoveho segmentu v souboru
 * @param max_dir_entries maximalni pocet polozek v adresari
 * @return nalezeny soubor, neni-li soubor nalezen je vracena hodnota NULL
 */
struct dir_file *find_file(FILE *p_file, struct boot_record *boot_record, char file_path[], uint start_of_root_dir,
                            uint start_of_data, uint max_dir_entries) {
    char *path = NULL;
    char *files = NULL;
    char *next = NULL;
    struct dir_file *object = NULL;
    uint position = start_of_root_dir;
    uint max_entries = max_dir_entries;

    path = malloc(strlen(file_path)*sizeof(char));
    strcpy(path, file_path);

    files = strtok(path, "/");
    while (files != NULL) {
        free(object);
        object = get_object_in_dir(p_file, files, position, max_entries);
        if (object == NULL) {
            break;
        }

        if (object->file_type == OBJECT_DIRECTORY) {
            position = object->first_cluster * boot_record->cluster_size + start_of_data;
            next = strtok(NULL, "/");
        } else {
            if (strtok(NULL, "/") == NULL) {
                free(path);
                return object;
            } else {
                free(object);
                free(path);
                return NULL;
            }
        }
        files = next;
    }

    free(path);
    return object;
}

/**
 * Hleda soubor nebo slozku ve slozce zacinajici v souboru na indexu danem hodnotu start_position.
 * @param p_file soubor, ve kterem se hleda
 * @param name nazev hledaneho souboru, slozky
 * @param start_position pocatecni pozice adresare v souboru, ve kterem se hleda
 * @param max_entries maximalni pocet polozek ve slozce
 * @return
 */
struct dir_file *get_object_in_dir(FILE *p_file, char name[], int32_t start_position, uint max_entries) {
    uint i = 0;
    struct dir_file *object = (struct dir_file *) malloc(sizeof(struct dir_file));

    fseek(p_file, start_position, SEEK_SET);

    for (i = 0; i < max_entries; i++) {
        fread(object, sizeof(struct dir_file), 1, p_file);
        if (strcmp(object->file_name, name) == 0) {
            return object;
        }
    }

    free(object);
    return NULL;
}

/**
 * Vrati veskere polozky v adresari.
 *
 * @param p_file soubor obsahujici fat
 * @param number_of_objects navratova hodnoto poctu polozek v adresari
 * @param positions navratova hodnota pozic polozek v souboru. Muze by NULL
 * @param start_position zacatek prohledavane slozky v souboru
 * @param max_entries maximalni pocet polozek v adresari
 * @return pole obsahujici polozky adresare, nebo NULL pri chybe
 */
struct dir_file *get_all_in_dir(FILE *p_file, int32_t *number_of_objects, long * positions, long start_position, uint max_entries) {
    uint i = 0;
    struct dir_file *files = NULL;
    struct dir_file file;
    long position = 0;

    if (p_file == NULL) {
        return NULL;
    }

    *number_of_objects = 0;

    files = malloc(sizeof(struct dir_file) * max_entries);

    fseek(p_file, start_position, SEEK_SET);
    for(i = 0; i < max_entries; i++){
        position = ftell(p_file);
        fread(&file, sizeof(struct dir_file), 1, p_file);
        if(file.file_name[0] != '\0'){
            files[*number_of_objects] = file;
            if(positions != NULL){
                positions[*number_of_objects] = position;
            }
            *number_of_objects += 1;
        }
    }

    return files;
}

/**
 * Zjisti zda slozka zacinajici na danem miste v souboru obsahuje dalsi soubory nebo slozky.
 *
 * @param p_file soubor obsahujici fat
 * @param max_entries maximalni pocet zaznamu ve slozce
 * @param start_of_dir zacatek adresare v souboru
 * @return -1 pri chybe, 1 je-li prazdny, 0 obsahuje-li data
 */
int is_dir_empty(FILE *p_file, int32_t max_entries, int32_t start_of_dir) {
    int i = 0;
    struct dir_file *file = NULL;

    if (p_file == NULL) {
        return -1;
    }

    file = (struct dir_file *) malloc(sizeof(struct dir_file));

    fseek(p_file, start_of_dir, SEEK_SET);
    for (i = 0; i < max_entries; i++) {
        fread(file, sizeof(struct dir_file), 1, p_file);
        if (file != NULL && file->file_name[0] != '\0') {
            free(file);
            return 0;
        }
    }
    free(file);
    return 1;
}

/**
 * Hleda, zda adresar obsahuje volne misto.
 *
 * @param p_file soubor obsahujici fat
 * @param max_entries maximalni pocet polozek ve slozce
 * @param start_of_dir zacatek adresare v souboru
 * @return pozici volneho mista, nebo -1.
 */
long find_empty_space_in_dir(FILE *p_file, int32_t max_entries, int32_t start_of_dir) {
    int i = 0;
    struct dir_file *file = NULL;

    if (p_file == NULL) {
        return -1;
    }

    file = (struct dir_file *) malloc(sizeof(struct dir_file));

    fseek(p_file, start_of_dir, SEEK_SET);
    for (i = 0; i < max_entries; i++) {
        fread(file, sizeof(struct dir_file), 1, p_file);
        if(file != NULL && file->file_name[0] == '\0'){
            free(file);
            return ftell(p_file) - sizeof(struct dir_file);
        }
    }
    free(file);
    return -1;
}

/**
 * Zapise strukturu file do souboru na dane misto.
 *
 * @param p_file soubor obsahujici fat
 * @param file soubor ktery se zapisuje do slozky
 * @param write_position pozice na kterou se soubor zapise
 * @return -1 pri chybe, jinak 0
 */
int write_to_dir(FILE *p_file, struct dir_file file, int32_t write_position) {
    if (p_file == NULL) {
        return -1;
    }

    fseek(p_file, write_position, SEEK_SET);
    fwrite(&file, sizeof(struct dir_file), 1, p_file);

    return 0;
}

/**
 * Zapise vstupni soubor na dane clustery do FAT
 *
 * @param fat_file soubor obsahujici fat
 * @param file soubor, ktery se bude zapisovat do fat
 * @param clusters indexy clusteru, ktere soubor zaplni
 * @param clusters_size pocet clusteru, ktere soubor zaplni
 * @param start_of_data zacatek datoveho segmentu v souboru
 * @param cluster_size velikost jednoho clusteru
 * @return -1 pri chybe, jinak 0
 */
int write_file_to_fat(FILE *fat_file, char *file, int file_size, int32_t *clusters, int32_t clusters_size, uint start_of_data, int16_t cluster_size){
    long cluster_position = 0;
    uint16_t u_cluster_size = 0;
    int i = 0;
    char *data = NULL;
    if(fat_file == NULL || file == NULL || clusters == NULL || cluster_size < 0){
        return -1;
    }
    u_cluster_size = (uint16_t) cluster_size;

    for(i = 0; i < clusters_size; i++){
        data = &file[i * cluster_size];

        cluster_position = start_of_data + (clusters[i] * cluster_size);
        fseek(fat_file, cluster_position, SEEK_SET);

        if((i+1) * cluster_size < file_size){
            u_cluster_size = (uint16_t) (file_size - i * cluster_size);
        }

        fwrite(data, u_cluster_size, 1, fat_file);
    }

    return 0;
}

int write_bytes_to_fat(FILE *fat_file, char *bytes, int bytes_size, long offset, int32_t *clusters, int32_t clusters_size, uint start_of_data, int16_t cluster_size){
    uint16_t first_cluster = 0;
    int cluster_offset = 0;
    int i = 0;
    int writed_bytes = 0;
    int write_size = 0;
    long write_position = 0;

    if(fat_file == NULL || bytes == NULL || clusters == NULL || cluster_size < 0){
        return 0;
    }

    first_cluster = (uint16_t)(offset / cluster_size);
    cluster_offset = (int)(offset % cluster_size);

    writed_bytes = 0;
    for(i = first_cluster; i < clusters_size; i++){

        write_size = cluster_size - cluster_offset;
        write_position = start_of_data + (clusters[i] * cluster_size) + cluster_offset;

        fseek(fat_file, write_position, SEEK_SET);

        if (writed_bytes + write_size > bytes_size){
            write_size = bytes_size - writed_bytes;
        }

        fwrite(&bytes[writed_bytes], (size_t) write_size, 1, fat_file);

        writed_bytes += write_size;
        cluster_offset = 0;
    }

    return writed_bytes;
}

/**
 * Vytvori prazdny adresar zabirajici velikost pres dane clustery.
 *
 * @param fat_file soubor obsahujici fat
 * @param clusters indexy clusteru, ktere adresar zaplni
 * @param clusters_size pocet clusteru, ktere adresar zaplni
 * @param start_of_data zacatek datoveho segmentu v souboru
 * @param cluster_size velikost jednoho clusteru
 * @return -1 pri chybe, jinak 0.
 */
int write_empty_dir_to_fat(FILE *fat_file, int32_t *clusters, int32_t clusters_size, uint start_of_data, int16_t cluster_size){
    int i = 0;
    long cluster_position = 0;
    char *data = NULL;
    uint16_t u_cluster_size = 0;

    if(fat_file == NULL || clusters == NULL){
        return -1;
    }

    u_cluster_size = (uint16_t) cluster_size;

    data = malloc(u_cluster_size);
    memset(data, 0, u_cluster_size);

    for(i = 0; i < clusters_size; i++){
        cluster_position = start_of_data + (clusters[i] * cluster_size);
        fseek(fat_file, cluster_position, SEEK_SET);

        fwrite(data, u_cluster_size, 1, fat_file);
    }

    free(data);
    return 0;
}

/**
 * Hleda ve fat tabulce potrebny pocet prazdnych clusteru a jejich indexy uklada do vstupniho pole clusters.
 * @param usable_cluster_count pocet vsech pouzitelnych clusteru ve fat
 * @param fat fat taublka
 * @param clusters pole, do ktereho se ulozi nalezene indexy
 * @param number_of_clusters pocet potrebnych clusteru
 * @return -1 neni-li nalezen dostatek clusteru, jinak 0
 */
int find_empty_clusters(int32_t usable_cluster_count, int32_t *fat, int32_t *clusters, long number_of_clusters) {
    int i = 0, j = 0;

    for (i = 0; i < usable_cluster_count; i++) {
        if (fat[i] == FAT_UNUSED) {
            clusters[j] = i;
            j++;
            if (j == number_of_clusters) {
                return 0;
            }
        }
    }
    return -1;
}

/**
 * Odstrani hodnoty z fat tabulky na pozicich danych polem indexes.
 * @param fat fat tabulka
 * @param indexes indexy do fat tabulky, na kterych se ma smazat hodnota
 * @param number_of_clusters velikost pole indexes
 * @return  -1 pri chybe jinak 0
 */
int rm_from_fat(int32_t *fat, int32_t *indexes, int32_t number_of_clusters) {
    int i = 0;

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

/**
 * Odstrani hodnoty ze vsech kopii fat tabulky v souboru na pozicich danych polem indexes.
 *
 * @param p_file soubor obsahujici fat
 * @param fat_record_size velikost jednoho zaznamu ve fat tabulce
 * @param fat_start zacat fat tabulky v souboru
 * @param indexes indexy do fat tabulky, na kterych se ma smazat hodnota
 * @param number_of_clusters velikost pole indexes
 * @param fat_size velikost fat tabulky v souboru
 * @param number_of_fat pocet fat kopii
 * @return -1 pri chybe jinak 0
 */
int rm_from_all_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes,
                           int32_t number_of_clusters, uint fat_size, int8_t number_of_fat){
    int i = 0;
    int result = 0;

    for(i = 0; i < number_of_fat; i++){
        result = rm_from_physic_fat(p_file, fat_record_size, fat_start + (i * fat_size), indexes, number_of_clusters);
        if(result == -1){
            return -1;
        }
    }

    return 0;
}

/**
 * Odstrani hodnoty z fat tabulky v souboru na pozicich danych polem indexes.
 * @param p_file soubor obsahujici fat
 * @param fat_record_size velikost jednoho zaznamu ve fat tabulce
 * @param fat_start zacat fat tabulky v souboru
 * @param indexes indexy do fat tabulky, na kterych se ma smazat hodnota
 * @param number_of_clusters velikost pole indexes
 * @return  -1 pri chybe jinak 0
 */
int rm_from_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes,
                       int32_t number_of_clusters) {
    int i = 0;
    int32_t index = 0;
    int32_t new_value = 0;

    if (p_file == NULL || indexes == NULL || number_of_clusters == 0) {
        return -1;
    }

    new_value = FAT_UNUSED;
    for (i = 0; i < number_of_clusters; i++) {
        fseek(p_file, fat_start + (fat_record_size * indexes[i]), SEEK_SET);
        fread(&index, fat_record_size, 1, p_file);
        fseek(p_file, ftell(p_file)-fat_record_size, SEEK_SET);
        if (index != FAT_BAD_CLUSTER) {
            fwrite(&new_value, fat_record_size, 1, p_file);
        }
    }

    return 0;
}

/**
 * Na zaklade vstupnich poli indexes a values zmeni vstupni fat tabulku.
 * @param fat fat tabulka
 * @param indexes indexy do fat tabulky, na kterych se ma zmenit hodnota
 * @param values hodnoty na ktere se zmeni hodnoty ve fat tabulce na indexech v poli indexes
 * @param number_of_clusters velikost poli indexes a values
 * @return  -1 pri chybe jinak 0
 */
int change_fat(int32_t *fat, int32_t *indexes, int32_t *values, int32_t number_of_clusters) {
    int i = 0;

    if (number_of_clusters == 0 || indexes == NULL || values == NULL || fat == NULL) {
        return -1;
    }

    for (i = 0; i < number_of_clusters; i++) {
        fat[indexes[i]] = values[i];
    }

    return 0;
}

/**
 * Na zaklade vstupnich poli indexes a values zmeni fat tabulku v souboru zacinajici na pozici fat_start.
 *
 * @param p_file soubor obsahujici fat
 * @param fat_record_size velikost jednoho zaznamu ve fat tabulce
 * @param fat_start zacatek fat tabulky ve vstupnim souboru
 * @param indexes indexy do fat tabulky, na kterych se ma zmenit hodnota
 * @param values hodnoty na ktere se zmeni hodnoty ve fat tabulce na indexech v poli indexes
 * @param number_of_clusters velikost poli indexes a values
 * @return  -1 pri chybe jinak 0
 */
int change_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes, int32_t *values,
                      int32_t number_of_clusters) {
    int i = 0;

    if (p_file == NULL || indexes == NULL || values == NULL || number_of_clusters == 0) {
        return -1;
    }

    for (i = 0; i < number_of_clusters; i++) {
        fseek(p_file, fat_start + (fat_record_size * indexes[i]), SEEK_SET);
        fwrite(&values[i], fat_record_size, 1, p_file);
    }

    return 0;
}

/**
 * Na zaklade vstupnich poli indexes a values zmeni vsechny kopie fat tabulky v souboru zacinajici na pozici fat_start.
 *
 * @param p_file soubor obsahujici fat
 * @param fat_record_size velikost jednoho zaznamu ve fat tabulce
 * @param fat_start zacatek fat tabulky ve vstupnim souboru
 * @param indexes indexy do fat tabulky, na kterych se ma zmenit hodnota
 * @param values hodnoty na ktere se zmeni hodnoty ve fat tabulce na indexech v poli indexes
 * @param number_of_clusters velikost poli indexes a values
 * @param fat_size velikost fat tabulky v souboru
 * @param number_of_fat pocet fat kopii
 * @return -1 pri chybe jinak 0
 */
int change_all_physic_fat(FILE *p_file, uint fat_record_size, int32_t fat_start, int32_t *indexes, int32_t *values,
                          int32_t number_of_clusters, uint fat_size, int8_t number_of_fat){
    int i = 0;
    int result = 0;

    for(i = 0; i < number_of_fat; i++){
        result = change_physic_fat(p_file, fat_record_size, fat_start + (i * fat_size), indexes, values,
                                   number_of_clusters);
        if(result == -1){
            return -1;
        }
    }

    return 0;
}

/**
 * Tiskne obsah cele slozky. Kazdy adresar ve slozce rekurzivne vytiskne.
 *
 * @param p_file soubor obsahujici fat
 * @param files obsah slozky
 * @param number_of_objects pocet polozek ve slozce
 * @param cluster_size velikost jednoho clusteru
 * @param start_of_data zacatek datoveho segmentu v souboru
 * @param max_dir_entries maximalni pocet polozek ve slozce
 * @param level level vnoreni podadresare
 */
void print_directory(FILE *p_file, struct dir_file *files, int number_of_objects, int16_t cluster_size, uint start_of_data, uint max_dir_entries, int level){
    int i = 0;
    int number_of_clusters = 0;
    struct dir_file *next_files = NULL;
    int next_number_of_objects = 0;
    char *space = malloc(sizeof(char) * (level+1));

    for(i = 0; i < level; i++){
        space[i] = '\t';
    }
    space[level] = '\0';

    for(i = 0; i < number_of_objects; i++){
        if(files[i].file_type == OBJECT_FILE){
            number_of_clusters = files[i].file_size/cluster_size;
            if(files[i].file_size%cluster_size > 0){
                number_of_clusters++;
            }
            printf("%s-%s %d %d\n", space, files[i].file_name, files[i].first_cluster, number_of_clusters);
        }else if(files[i].file_type == OBJECT_DIRECTORY){
            printf("%s+%s\n", space, files[i].file_name);
            next_files = get_all_in_dir(p_file, &next_number_of_objects, NULL, start_of_data + (files[i].first_cluster * cluster_size), max_dir_entries);
            print_directory(p_file, next_files, next_number_of_objects, cluster_size, start_of_data, max_dir_entries, level + 1);
        }
    }

    space[level-1] = '\0';
    printf("%s--\n", space);
    free(space);
}