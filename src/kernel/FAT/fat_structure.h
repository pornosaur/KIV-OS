#ifndef FAT_FAT_STRUCTURE_H
#define FAT_FAT_STRUCTURE_H

#define NAME_ALOC_SIZE 13

#include <stdint.h>

static const uint32_t FAT_UNUSED = INT32_MAX - 1;
static const uint32_t FAT_FILE_END = INT32_MAX - 2;
static const uint32_t FAT_BAD_CLUSTER = INT32_MAX - 3;

static const unsigned int OBJECT_DIRECTORY = 0;
static const unsigned int OBJECT_FILE = 1;
static const unsigned int NAME_SIZE = NAME_ALOC_SIZE - 1;

struct boot_record {
	char volume_descriptor[256];				//popis
	uint8_t fat_type;							//typ FAT - pocet clusteru = 2^fat_type (priklad FAT 12 = 4096)
	uint8_t fat_copies;							//kolikrat je za sebou v souboru ulozena FAT
	uint16_t cluster_size;						//velikost clusteru ve znacich
	uint32_t usable_cluster_count;				//pocet pouzitelnych clusteru (2^fat_type - reserved_cluster_count)
	uint32_t reserved_cluster_count;			//pocet rezervovanych clusteru pro systemove polozky
	uint16_t dir_clusters;						//pocet clusteru pro kazdy adresar vcetne root adresare
	char signature[9];							//pro vstupni data od vyucujicich konzistence FAT - "OK","NOK","FAI" - v poradku / FAT1 != FAT2 != FATx / FAIL - ve FAT1 == FAT2 == FAT3, ale obsahuje chyby, nutna kontrola
};


struct dir_file {
	char file_name[NAME_ALOC_SIZE];				//NAME_SIZE + '/0' (8.3 format)
	uint8_t file_type;							//1 = soubor, 0 = adresar
	uint32_t file_size;							//pocet znaku v souboru
	uint32_t first_cluster;						//cluster ve FAT, kde soubor zacina
};

#endif
