#ifndef FAT_FAT_STRUCTURE_H
#define FAT_FAT_STRUCTURE_H

#include <stdint.h>

static const int32_t FAT_UNUSED = INT32_MAX - 1;
static const int32_t FAT_FILE_END = INT32_MAX - 2;
static const int32_t FAT_BAD_CLUSTER = INT32_MAX - 3;

static const int OBJECT_DIRECTORY = 0;
static const int OBJECT_FILE = 1;

struct boot_record {
    char volume_descriptor[256];               //popis
    int8_t fat_type;                           //typ FAT - pocet clusteru = 2^fat_type (priklad FAT 12 = 4096)
    int8_t fat_copies;                         //kolikrat je za sebou v souboru ulozena FAT
    int16_t cluster_size;                      //velikost clusteru ve znacich
    int32_t usable_cluster_count;              //pocet pouzitelnych clusteru (2^fat_type - reserved_cluster_count)
    uint32_t reserved_cluster_count;       //pocet rezervovanych clusteru pro systemove polozky
    int16_t dir_clusters;                      //pocet clusteru pro kazdy adresar vcetne root adresare
    char signature[9];                         //pro vstupni data od vyucujicich konzistence FAT - "OK","NOK","FAI" - v poradku / FAT1 != FAT2 != FATx / FAIL - ve FAT1 == FAT2 == FAT3, ale obsahuje chyby, nutna kontrola
};


struct dir_file{
    char file_name[12];             //8+3 format + '/0'
    int8_t file_type;               //1 = soubor, 0 = adresar
    int32_t file_size;              //pocet znaku v souboru
    unsigned long first_cluster;          //cluster ve FAT, kde soubor zacina
};

#endif
