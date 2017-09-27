#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "library.h"
#include "badblock_recovery.h"

FILE *p_file;

uint start_of_fat = 0;
uint fat_record_size = 0;
uint fat_size = 0;
uint start_of_root_dir = 0;
uint max_dir_entries = 0;
uint start_of_data = 0;

void init_values(struct boot_record *boot_record);

void run_check_unused_clusters(struct boot_record *boot_record, pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, int32_t *fat);

void run_check_used_clusters(struct boot_record *boot_record, pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, int32_t *fat);


void *check_unused_clusters(void *void_input);

int get_next_unused_cluster(int32_t *fat, int32_t *index, pthread_mutex_t *fat_mutex, int32_t usable_cluster_count);



void *treat_directory(void *void_input);

int8_t check_is_cluster_ok(char *cluster, int cluster_length);

void repair_bad_cluster(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, struct boot_record *boot_record, struct dir_file *object, long object_position, int32_t *fat, int32_t bad_cluster, char *cluster_body);

void change_first_cluster(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, int32_t *fat, struct dir_file *object, long object_position, int32_t new_empty_cluster, int32_t bad_cluster, int8_t fat_copies);

int change_not_first_cluster(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, struct boot_record *boot_record, int32_t *fat, struct dir_file *object, int32_t new_empty_cluster, int32_t bad_cluster);

void repair_object(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, struct boot_record *boot_record, struct dir_file *object, int32_t *fat, long object_position);

/**
 * Spousti kontrolu a opravu bad blocku ve FAT.
 * @param argc pocet vstupnich argumentu
 * @param argv vstupni argumenty
 * @return -1 pri chybe, jinak 0
 */
int main(int argc, char *argv[]) {
    struct boot_record *boot_record = NULL;
    int32_t *fat = NULL;
    pthread_mutex_t *file_mutex = NULL;
    pthread_mutex_t *fat_mutex = NULL;
    static struct timeval tm1;
    static struct timeval tm2;

    if(argc != 2){
        printf("Only name of fat is available as input argument");
        return -1;
    }

    p_file = fopen(argv[1], "r+");
    if (p_file == NULL) {
        printf("Can't open file\n");
        return -1;
    }
    fseek(p_file, 0, SEEK_SET);

    // get BOOT_RECORD
    boot_record = get_boot_record(p_file);
    init_values(boot_record);

    // get FAT
    fseek(p_file, start_of_fat, SEEK_SET);
    fat = get_fat(p_file, fat_record_size, boot_record->usable_cluster_count, fat_size);

    // mutex init
    file_mutex = malloc(sizeof(pthread_mutex_t));
    fat_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(file_mutex, NULL);
    pthread_mutex_init(fat_mutex, NULL);

    gettimeofday(&tm1, NULL);

    run_check_unused_clusters(boot_record, file_mutex, fat_mutex, fat);
    run_check_used_clusters(boot_record, file_mutex, fat_mutex, fat);

    gettimeofday(&tm2, NULL);

    printf("Repairing completed in: %li ms\n", ((tm2.tv_sec * 1000) + (tm2.tv_usec/1000)) - ((tm1.tv_sec * 1000) + (tm1.tv_usec/1000)));

    free(fat);
    free(boot_record);
    free(file_mutex);
    free(fat_mutex);
    fclose(p_file);
}

/**
 * Initializace potrebnych konstant
 * @param boot_record zavadec nacteny z FAT
 */
void init_values(struct boot_record *boot_record){
    start_of_fat = boot_record->reserved_cluster_count * boot_record->cluster_size;
    fat_record_size = sizeof(int32_t);
    fat_size = boot_record->usable_cluster_count * fat_record_size;
    start_of_root_dir = start_of_fat + (boot_record->fat_copies * fat_size);
    max_dir_entries = boot_record->cluster_size / sizeof(struct dir_file);
    start_of_data = start_of_root_dir + (boot_record->dir_clusters * boot_record->cluster_size);
}

/**
 * Spusti vlakna pro prohledavani prazdnych clusteru a nakonec pocka na jejich dokonceni.
 *
 * @param boot_record zavadec nacteny z FAT
 * @param file_mutex mutex pro praci s FAT souborem
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param fat FAT tabulka
 */
void run_check_unused_clusters(struct boot_record *boot_record, pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, int32_t *fat){
    int i = 0;
    int32_t *index = NULL;
    pthread_t *threads = NULL;
    struct check_unused_clusters_struct check_unused_clusters_struct;

    index = malloc(sizeof(int32_t));
    threads = malloc(sizeof(pthread_t) * 10);

    check_unused_clusters_struct.boot_record = boot_record;
    check_unused_clusters_struct.index = index;
    check_unused_clusters_struct.file_mutex = file_mutex;
    check_unused_clusters_struct.fat_mutex = fat_mutex;
    check_unused_clusters_struct.fat = fat;

    //spusteni vlaken
    for(i = 0; i < 10; i++){
        pthread_create(&threads[i], NULL, check_unused_clusters, (void *) &check_unused_clusters_struct);
    }

    //cekani na ukonceni vlaken
    for(i = 0; i < 10; i++){
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(index);
}

/**
 * Spusti prohledavani root adresare
 *
 * @param boot_record zavadec nacteny z FAT
 * @param file_mutex mutex pro praci s FAT souborem
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param fat FAT tabulka
 */
void run_check_used_clusters(struct boot_record *boot_record, pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, int32_t *fat){
    struct treat_directory_struct treat_directory_struct;
    treat_directory_struct.file_mutex = file_mutex;
    treat_directory_struct.fat_mutex = fat_mutex;
    treat_directory_struct.boot_record = boot_record;
    treat_directory_struct.fat = fat;
    treat_directory_struct.start_position = start_of_root_dir;

    treat_directory(&treat_directory_struct);
}

// =====================================================================================================================

/**
 * Prohledva veskere nepouzivane clustery, zda nesjou spatne.
 *
 * @param void_input struktura check_unused_clusters_struct obsahuje veskera potrebna data pro chod funkce.
 */
void *check_unused_clusters(void *void_input){
    struct check_unused_clusters_struct *input = (struct check_unused_clusters_struct*)void_input;
    char *cluster = NULL;
    int i = 0;
    int32_t values = 0;
    int32_t indexes = 0;

    cluster = malloc(sizeof(char) * input->boot_record->cluster_size);

    while((i = get_next_unused_cluster(input->fat, input->index, input->fat_mutex, input->boot_record->usable_cluster_count)) != -1 ) {

        pthread_mutex_lock(input->file_mutex);
        fseek(p_file, start_of_data + (i * input->boot_record->cluster_size), SEEK_SET);
        fread(cluster, (size_t) input->boot_record->cluster_size, 1, p_file);
        pthread_mutex_unlock(input->file_mutex);

        if (check_is_cluster_ok(cluster, input->boot_record->cluster_size) == 0) {

            indexes = i;
            values = FAT_BAD_CLUSTER;

            pthread_mutex_lock(input->fat_mutex);
            change_fat(input->fat, &indexes, &values, 1);
            pthread_mutex_unlock(input->fat_mutex);

            pthread_mutex_lock(input->file_mutex);
            change_all_physic_fat(p_file, fat_record_size, start_of_fat, &indexes, &values, 1, fat_size, input->boot_record->fat_copies);
            pthread_mutex_unlock(input->file_mutex);
        }
    }
    return 0;
}

/**
 * Vyhleda dalsi nepouzivany cluster, ktery je potreba zkontrolovat
 *
 * @param fat fat tabulka
 * @param index index, od ktereho zacit hledat
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param usable_cluster_count pocet vsech clusteru FAT urcenych pro ukladani souboru
 * @return index dalsiho clusteru, -1 neni-li jiz dalsi cluster
 *
 */
int get_next_unused_cluster(int32_t *fat, int32_t *index, pthread_mutex_t *fat_mutex, int32_t usable_cluster_count){
    int i = 0;

    pthread_mutex_lock(fat_mutex);
    for (i = *index; i < usable_cluster_count; i++) {
        if (fat[i] == FAT_UNUSED) {
            *index = i + 1;
            pthread_mutex_unlock(fat_mutex);
            return i;
        }
    }
    *index = usable_cluster_count;
    pthread_mutex_unlock(fat_mutex);
    return -1;
}

//======================================================================================================================

/**
 * Zkontroluje kazdy prvek v adresari, jestli neobsahuje spatne clustery. Na kazdy nalezeny adresar spusti v novem
 * vlakne stejnou kontrolu.
 *
 * @param void_input struktura treat_directory_sturct obsahuje veskera potrebna data pro chod funkce.
 */
void *treat_directory(void *void_input) {
    struct treat_directory_struct *input = (struct treat_directory_struct*) void_input;
    struct treat_directory_struct *inputs = NULL;
    struct dir_file *objects = NULL;
    pthread_t * threads = NULL;
    long *positions = NULL;
    long position = 0;
    int32_t number_of_objects = 0;
    int number_of_threads = 0;
    int i = 0;

    inputs = malloc(sizeof(struct treat_directory_struct) * max_dir_entries);
    threads = malloc(sizeof(pthread_t) * max_dir_entries);
    positions = malloc(sizeof(long) * max_dir_entries);

    pthread_mutex_lock(input->file_mutex);
    objects = get_all_in_dir(p_file, &number_of_objects, positions, input->start_position, max_dir_entries);
    pthread_mutex_unlock(input->file_mutex);

    // prohledavani adresare
    for (i = 0; i < number_of_objects; i++) {
        repair_object(input->file_mutex, input->fat_mutex, input->boot_record, &objects[i], input->fat, positions[i]);
        if (objects[i].file_type == OBJECT_DIRECTORY) {
            position = start_of_data + (objects[i].first_cluster * input->boot_record->cluster_size);

            inputs[number_of_threads].boot_record = input->boot_record;
            inputs[number_of_threads].fat = input->fat;
            inputs[number_of_threads].fat_mutex = input->fat_mutex;
            inputs[number_of_threads].file_mutex = input->file_mutex;
            inputs[number_of_threads].start_position = position;

            pthread_create(&threads[number_of_threads], NULL, treat_directory, (void *)&inputs[number_of_threads]);
            number_of_threads++;
        }
    }

    // cekani na skonceni vytvorenych vlaken
    while(number_of_threads != 0){
        number_of_threads--;
        pthread_join(threads[number_of_threads], NULL);
    }

    // uklid alokovane pameti
    free(inputs);
    free(threads);
    free(positions);
    free(objects);

    return 0;
}

/**
 * Nacte hodnoty clusteru pro dany objekt. Zkontroluje, jestli je cluster v poradku. Pokud neni v poradku, spusti se
 * oprava.
 *
 * @param p_file soubor s FAT
 * @param file_mutex mutex pro praci s FAT souborem
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param boot_record zavadec nacteny z FAT
 * @param object objekt, ktereho se zmena tyka
 * @param fat fat tabulka
 * @param object_position pozice objektu v souboru s FAT
 */
void repair_object(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, struct boot_record *boot_record, struct dir_file *object, int32_t *fat, long object_position) {
    int32_t *clusters = NULL;
    int32_t clusters_size = 0;
    char *cluster = NULL;
    int i = 0;

    cluster = malloc(sizeof(char) * boot_record->cluster_size);
    clusters = get_file_clusters(object, &clusters_size, boot_record->cluster_size, fat, boot_record->dir_clusters);

    for (i = 0; i < clusters_size; i++) {
        pthread_mutex_lock(file_mutex);
        fseek(p_file, start_of_data + (clusters[i] * boot_record->cluster_size), SEEK_SET);
        fread(cluster, (size_t) boot_record->cluster_size, 1, p_file);
        pthread_mutex_unlock(file_mutex);
        if (check_is_cluster_ok(cluster, boot_record->cluster_size) == 0) {
            strncpy(cluster, "******", 6);
            strncpy(cluster + boot_record->cluster_size - 6, "******", 6);
            repair_bad_cluster(file_mutex, fat_mutex, boot_record, object, object_position, fat, clusters[i], cluster);
        }
    }

    free(cluster);
}

/**
 * Opravy soubor obsahujici jeden spatny cluster.
 *
 * @param p_file soubor s FAT
 * @param file_mutex mutex pro praci s FAT souborem
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param boot_record zavadec nacteny z FAT
 * @param object objekt, ktereho se zmena tyka
 * @param object_position pozice objektu v souboru s FAT
 * @param fat fat tabulka
 * @param bad_cluster index spatneho clusteru, ktery musi byt prepsan
 * @param cluster_body obsah clusteru
 */
void repair_bad_cluster(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, struct boot_record *boot_record, struct dir_file *object, long object_position,
                        int32_t *fat, int32_t bad_cluster, char *cluster_body) {

    int32_t new_empty_cluster = 0;

    // hledani noveho clusteru
    pthread_mutex_lock(fat_mutex);
    if (find_empty_clusters(boot_record->usable_cluster_count, fat, &new_empty_cluster, 1)) {
        printf("FAT IS FULL\n");
        return;
    }
    fat[new_empty_cluster] = FAT_FILE_END; // nastavena nodnota, aby cluster nebyl zabran jinym vlaknem
    pthread_mutex_unlock(fat_mutex);

    //zmena fat retezce
    if (object->first_cluster == bad_cluster) {
        change_first_cluster(file_mutex, fat_mutex, fat, object, object_position, new_empty_cluster, bad_cluster,boot_record->fat_copies);
    } else {
        if(change_not_first_cluster(file_mutex, fat_mutex, boot_record, fat, object, new_empty_cluster, bad_cluster)){
            fat[new_empty_cluster] = FAT_UNUSED;
            return;
        }
    }

    //presun hodnoty clusteru
    pthread_mutex_lock(file_mutex);
    fseek(p_file, start_of_data + (new_empty_cluster * boot_record->cluster_size), SEEK_SET);
    fwrite(cluster_body, (size_t) boot_record->cluster_size, 1, p_file);
    pthread_mutex_unlock(file_mutex);
}

/**
 * Zmeni prvni hodnotu FAT retezce souboru. => zmeni fat a navic zmeni hodnotu ve svem zaznamu v nadrazenem adresari.
 *
 * @param p_file soubor s FAT
 * @param file_mutex mutex pro praci s FAT souborem
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param fat fat tabulka
 * @param object objekt, ktereho se zmena tyka
 * @param object_position pozice objektu v souboru s FAT
 * @param new_empty_cluster index prazdneho clusteru, ktery muze byt pouzit
 * @param bad_cluster index spatneho clusteru, ktery musi byt prepsan
 * @param fat_copies pocet kopii fat tabulky
 */
void change_first_cluster(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, int32_t *fat, struct dir_file *object, long object_position,
                          int32_t new_empty_cluster, int32_t bad_cluster, int8_t fat_copies){
    int32_t *indexes = NULL;
    int32_t *values = NULL;

    object->first_cluster = new_empty_cluster;
    pthread_mutex_lock(file_mutex);
    fseek(p_file, object_position, SEEK_SET);
    fwrite(object, sizeof(struct dir_file), 1, p_file);
    pthread_mutex_unlock(file_mutex);

    values = malloc(sizeof(int32_t) * 2);
    indexes = malloc(sizeof(int32_t) * 2);

    indexes[0] = new_empty_cluster;
    indexes[1] = bad_cluster;
    values[0] = fat[bad_cluster];
    values[1] = FAT_BAD_CLUSTER;

    pthread_mutex_lock(fat_mutex);
    change_fat(fat, indexes, values, 2);
    pthread_mutex_unlock(fat_mutex);

    pthread_mutex_lock(file_mutex);
    change_all_physic_fat(p_file, fat_record_size, start_of_fat, indexes, values, 2, fat_size, fat_copies);
    pthread_mutex_unlock(file_mutex);

    free(indexes);
    free(values);
}

/**
 * Zmeni hodnoty FAT retezce souboru, tak aby neobsahovali bad_cluster.
 *
 * @param file_mutex mutex pro praci s FAT souborem
 * @param fat_mutex mutex pro praci s FAT tabulkou
 * @param boot_record zavadec nacteny z FAT
 * @param fat fat tabulka
 * @param object objekt, ktereho se zmena tyka
 * @param new_empty_cluster incex prazdneho clusteru, ktery muze byt pouzit
 * @param bad_cluster index spatneho clusteru, ktery musi byt prepsan
 * @return -1 nebyl-li bad_cluster v ceste nalezen, jinak vraci 0
 */
int change_not_first_cluster(pthread_mutex_t *file_mutex, pthread_mutex_t *fat_mutex, struct boot_record *boot_record, int32_t *fat, struct dir_file *object,
                              int32_t new_empty_cluster, int32_t bad_cluster){
    int i = 0;
    int32_t cluster_to_change = -1;
    int32_t *indexes = NULL;
    int32_t *values = NULL;
    int32_t clusters_size = 0;
    int *clusters = NULL;

    clusters = get_file_clusters(object, &clusters_size, boot_record->cluster_size, fat, boot_record->dir_clusters);

    for (i = 0; i < clusters_size - 1; i++) {
        if (clusters[i + 1] == bad_cluster) {
            cluster_to_change = clusters[i];
            break;
        }
    }
    free(clusters);

    if (cluster_to_change == -1) {
        printf("CANT REPAIR FILE\n");
        return -1;
    }

    values = malloc(sizeof(int32_t) * 3);
    indexes = malloc(sizeof(int32_t) * 3);
    indexes[0] = new_empty_cluster;
    indexes[1] = bad_cluster;
    indexes[2] = cluster_to_change;
    values[0] = fat[bad_cluster];
    values[1] = FAT_BAD_CLUSTER;
    values[2] = new_empty_cluster;

    pthread_mutex_lock(fat_mutex);
    change_fat(fat, indexes, values, 3);
    pthread_mutex_unlock(fat_mutex);
    pthread_mutex_lock(file_mutex);
    change_all_physic_fat(p_file, fat_record_size, start_of_fat, indexes, values, 3, fat_size, boot_record->fat_copies);
    pthread_mutex_unlock(file_mutex);

    free(indexes);
    free(values);

    return 0;
}

/**
 * Funkce zkontroluje vstupni retezec, zda neobahuje na konci a na zacatku sest pismen 'F' v raade. Pokavad ano je
 * vraceno 0, jinak 1.
 *
 * @param cluster cely obsah cluteru
 * @param cluster_length delka celeho cluteru
 * @return 1 je-li v poradku, 0 neni-li v poradku, -1 je-li retezec mensi nez 6 znaku
 */
int8_t check_is_cluster_ok(char *cluster, int cluster_length) {
    if (cluster_length < 6) {
        return -1;
    }

    if (strncasecmp(cluster, "FFFFFF", 6)) {
        return 1;
    }
    if (strncasecmp(cluster + cluster_length - 6, "FFFFFF", 6)) {
        return 1;
    }

    return 0;
}