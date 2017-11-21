#ifndef FAT_LIBRARY_H
#define FAT_LIBRARY_H

#include <stdlib.h>
#include <assert.h>
#include "fat_structure.h"

/**
* Nacte strukturu boot_record ze vstupni pameti memory.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @return vyplnenou strukturu boot_record nebo NULL pri chybe
*/
struct boot_record *get_boot_record(char *memory, size_t memory_size);

/**
* Nacte fat tabulku ze vstupni pameti memory.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param start_of_fat index do memory ukazujici na nacitanou fat tabulku
* @param fat_record_size velikost jednoho zaznamu ve fat tabulce
* @param cluster_count pocet zaznamu ve fat tabulce
* @param fat_size velikost fat tabulky v bitech
* @return pole obsahujici fat tabulku nebo NULL pri chybe
*/
uint32_t *get_fat(char *memory, size_t memory_size, size_t start_of_fat, unsigned int fat_record_size, unsigned long cluster_count, unsigned long fat_size);

/**
* Odstrani veskery obsah clusteru ze vstupni pameti memory
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param start_of_cluster index zacatku clusteru, pocitan od zacatku souboru
* @param cluster_size velikost jednoho clusteru
*/
void delete_cluster(char *memory, size_t memory_size, unsigned int start_of_cluster, uint16_t cluster_size);

/**
* Najde indexy clusteru ve fat tabulce, na kterych je soubor ulozen.
*
* @param file soubor jehoz clustery se hledaji
* @param clusters_size pointer na pocet clusteru v souboru, hodnota je nastavovana v teto funkci
* @param cluster_size velikost jednoho clusteru
* @param fat fat tabulka
* @param dir_clusters maximalni pocet clusteru na slozku
* @return NULL pri chybe, jinak indexy clusteru ve fat tabulce.
*/
uint32_t *get_file_clusters(struct dir_file *file, uint32_t *clusters_size, uint16_t cluster_size, const uint32_t *fat, uint16_t dir_clusters);

/**
* Hleda soubor nebo slozku ve slozce zacinajici v souboru na indexu danem hodnotu start_position.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param name nazev hledaneho souboru, slozky, null terminated string
* @param file_type typ souboru, ktery se hleda 0 - dir, 1 - file
* @param start_position pocatecni pozice adresare v souboru, ve kterem se hleda
* @param max_entries maximalni pocet polozek ve slozce
* @param object_dir_pos pozice zaznamu ve slozce nalezenoho souboru
* @return nalezeny soubor nebo NULL pri chybe
*/
struct dir_file *get_object_in_dir(char *memory, size_t memory_size, const char name[], unsigned int file_type, uint32_t start_position, unsigned int max_entries, uint32_t *object_dir_pos);

/**
* Vrati veskere polozky v adresari.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param number_of_objects navratova hodnoto poctu polozek v adresari
* @param positions navratova hodnota pozic polozek v souboru. Muze by NULL
* @param start_position zacatek prohledavane slozky v souboru
* @param max_entries maximalni pocet polozek v adresari
* @return pole obsahujici polozky adresare, nebo NULL pri chybe
*/
struct dir_file *get_all_in_dir(char *memory, size_t memory_size, uint32_t *number_of_objects, unsigned long * positions, unsigned long start_position, unsigned int max_entries);

/**
* Zjisti zda slozka zacinajici na danem miste v souboru obsahuje dalsi soubory nebo slozky.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param max_entries maximalni pocet zaznamu ve slozce
* @param start_of_dir zacatek adresare v souboru
* @return -1 pri chybe, 1 je-li prazdny, 0 obsahuje-li data
*/
int is_dir_empty(char *memory, size_t memory_size, uint32_t max_entries, uint32_t start_of_dir);

/**
* Hleda, zda adresar obsahuje volne misto.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param position pozice volneho mista
* @param max_entries maximalni pocet polozek ve slozce
* @param start_of_dir zacatek adresare v souboru
* @return -1 pri chybe jinak 0
*/
int find_empty_space_in_dir(char *memory, size_t memory_size, unsigned long *position, uint32_t max_entries, uint32_t start_of_dir);

/**
* Zapise strukturu file do pole memory na dane misto.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param file soubor ktery se zapisuje do slozky
* @param write_position pozice na kterou se soubor zapise
* @return -1 pri chybe, jinak 0
*/
int write_to_dir(char *memory, size_t memory_size, struct dir_file *file, uint32_t write_position);

/**
* Zapise vstupni pole bytes na dane clustery do FAT s danym offsetem
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param bytes data, ktera se bude zapisovat do fat
* @param bytes_size velikost pole bytes
* @param offset kolik bytu je posun od zacatku prvniho clusteru
* @param clusters indexy clusteru, ktere soubor zaplni
* @param clusters_size pocet clusteru, ktere soubor zaplni
* @param start_of_data zacatek datoveho segmentu v souboru
* @param cluster_size velikost jednoho clusteru
* @return -1 pri chybe, jinak 0
*/
size_t write_bytes_to_fat(char *memory, size_t memory_size, char *bytes, unsigned long bytes_size, unsigned long offset, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size);

/**
* Prepise hodnoty clusteru ve fat na 0
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param clusters indexy clusteru, ktere adresar zaplni
* @param clusters_size pocet clusteru, ktere adresar zaplni
* @param start_of_data zacatek datoveho segmentu v souboru
* @param cluster_size velikost jednoho clusteru
* @return -1 pri chybe, jinak 0.
*/
int write_empty_dir_to_fat(char *memory, size_t memory_size, const uint32_t *clusters, uint32_t clusters_size, unsigned int start_of_data, uint16_t cluster_size);

/**
* Hleda ve fat tabulce potrebny pocet prazdnych clusteru a jejich indexy uklada do vstupniho pole clusters.
* @param usable_cluster_count pocet vsech pouzitelnych clusteru ve fat
* @param fat fat taublka
* @param clusters pole, do ktereho se ulozi nalezene indexy
* @param number_of_clusters pocet potrebnych clusteru
* @return -1 neni-li nalezen dostatek clusteru, jinak 0
*/
int find_empty_clusters(uint32_t usable_cluster_count, const uint32_t *fat, uint32_t *clusters, unsigned long number_of_clusters);

/**
* Nastavi hodnoty ve fat tabulce na pozicich danych polem indexes na hodnotu FAT_UNUSED.
*
* @param fat fat tabulka
* @param indexes indexy do fat tabulky, ketere se nastavuji
* @param number_of_clusters velikost pole indexes
* @return  -1 pri chybe jinak 0
*/
int rm_from_fat(uint32_t *fat, const uint32_t *indexes, uint32_t number_of_clusters);

/**
* Nastavi hodnoty ve vsech kopiich fat tabulky v poli memory na pozicich danych polem indexes na hodnotu FAT_UNUSED.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param fat_record_size velikost jednoho zaznamu ve fat tabulce
* @param fat_start zacat fat tabulky v souboru
* @param indexes indexy do fat tabulky, ketere se nastavuji
* @param number_of_clusters velikost pole indexes
* @param fat_size velikost fat tabulky v souboru
* @param number_of_fat pocet fat kopii
* @return -1 pri chybe jinak 0
*/
int rm_from_all_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, uint32_t *indexes, uint32_t number_of_clusters, unsigned int fat_size, uint8_t number_of_fat);

/**
* Nastavi hodnoty ve fat tabulce v souboru na pozicich danych polem indexes na hodnotu FAT_UNUSED.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param fat_record_size velikost jednoho zaznamu ve fat tabulce
* @param fat_start zacat fat tabulky v souboru
* @param indexes indexy do fat tabulky, ketere se nastavuji
* @param number_of_clusters velikost pole indexes
* @return  -1 pri chybe jinak 0
*/
int rm_from_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, const uint32_t *indexes, uint32_t number_of_clusters);

/**
* Na zaklade vstupnich poli indexes a values zmeni vstupni fat tabulku.
*
* @param fat fat tabulka
* @param indexes indexy do fat tabulky, na kterych se ma zmenit hodnota
* @param values hodnoty na ktere se zmeni hodnoty ve fat tabulce na indexech v poli indexes
* @param number_of_clusters velikost poli indexes a values
* @return  -1 pri chybe jinak 0
*/
int change_fat(uint32_t *fat, const uint32_t *indexes, const uint32_t  *values, uint32_t number_of_clusters);

/**
* Na zaklade vstupnich poli indexes a values zmeni fat tabulku v souboru zacinajici na pozici fat_start.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param fat_record_size velikost jednoho zaznamu ve fat tabulce
* @param fat_start zacatek fat tabulky ve vstupnim souboru
* @param indexes indexy do fat tabulky, na kterych se ma zmenit hodnota
* @param values hodnoty na ktere se zmeni hodnoty ve fat tabulce na indexech v poli indexes
* @param number_of_clusters velikost poli indexes a values
* @return  -1 pri chybe jinak 0
*/
int change_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, const uint32_t *indexes, uint32_t *values, uint32_t number_of_clusters);

/**
* Na zaklade vstupnich poli indexes a values zmeni vsechny kopie fat tabulky v souboru zacinajici na pozici fat_start.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param fat_record_size velikost jednoho zaznamu ve fat tabulce
* @param fat_start zacatek fat tabulky ve vstupnim souboru
* @param indexes indexy do fat tabulky, na kterych se ma zmenit hodnota
* @param values hodnoty na ktere se zmeni hodnoty ve fat tabulce na indexech v poli indexes
* @param number_of_clusters velikost poli indexes a values
* @param fat_size velikost fat tabulky v souboru
* @param number_of_fat pocet fat kopii
* @return -1 pri chybe jinak 0
*/
int change_all_physic_fat(char *memory, size_t memory_size, unsigned int fat_record_size, uint32_t fat_start, uint32_t *indexes, uint32_t *values, uint32_t number_of_clusters, unsigned int fat_size, uint8_t number_of_fat);

/**
* Funkce odstrani zaznam ze slozky, ktery zacina na dane pozici.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param position index na pozici, kde zaznam zacina
*/
void remove_record_in_dir(char *memory, size_t memory_size, unsigned long position);

/**
* Cte obsah souboru daneho clustry a uklada jej do buffer. Cteni je posunuto o offset.
*
* @param memory pole predstavujici Fat disk
* @param memory_size velikost pole s Fat diskem
* @param buffer do ktereho se ukladaji nacitana data
* @param buffer_size velikost bufferu
* @param offset predstavuje pozici v souboru od ktere se zacina cist
* @param file_size velikost cteneho souboru
* @param cluster_size velikost clusteru
* @param clusters indexy do fat tabulky predstavujici data souboru
* @param pozice zacatku slozky root
* @param velikost clusters
* @return pocet zapsanych znaku
*/
size_t read_file(char *memory, size_t memory_size, char *buffer, unsigned int buffer_size, unsigned long offset, uint32_t file_size, uint16_t cluster_size, uint32_t *clusters, unsigned int start_of_root_dir, uint32_t file_clusters_size);
#endif
