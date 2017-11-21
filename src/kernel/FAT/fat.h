#pragma once
#ifndef FAT_H
#define FAT_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "fat_structure.h"

struct fat_data {
	char *memory;			/* pole predstavuje fat disk*/
	size_t memory_size;     /* velikost pole memory */

	struct boot_record *boot_record;	/* boot record Fat disku */
	uint32_t *fat1;						/* prvni kopie fat tabulky */
	uint32_t *fat2;						/* druha kopie fat tabulky */
	unsigned int start_of_fat;			/* index zacatku fat tabulek */
	unsigned int fat_record_size;		/* Velikost zaznamu ve fat tabulce */
	unsigned int fat_size;				/* Velikost fat tabulky */
	unsigned int start_of_root_dir;		/* index zacatku root slozky */
	unsigned int max_dir_entries;		/* maximalni pocet zaznamu ve slozce */
};

/**
* Z fat data nacte fat disk a inicializuje potrebne struktury v f_data podle dat ve fat disku
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @return -1 pri chybe, 0 pri uspechu
*/
int fat_init(struct fat_data *f_data);

/**
* Vycisti alokovanou pamet ve strukture f_data
*
* @param f_data struktura obsahujici informace o nactenem fat disku
*/
void close_fat(struct fat_data *f_data);

/**
* Vytvori prazdny soubor ve fat na danem miste
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param new_file nastavi na nove vytvoreny soubor
* @param file_name jmeno vytvareneho souboru
* @param act_fat_position pozice adresare ve kterem vytvarime soubor
* @param dir_position pozice zaznamu o souboru ve slozce vzhledem k celemu disku
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_create_file(struct fat_data *f_data, struct dir_file **new_file, const char *file_name, uint32_t act_fat_position, unsigned long *dir_position);

/**
* Vrati zaznam o souboru nacteneho z fat disku nachazici se ve slozce zacinajici na act_fat_position
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param file_name jmeno hledaneho souboru
* @param file_type typ hledaneho souboru 0 - dir 1 - file
* @param act_fat_position pozice adresare ve kterem hledame soubor
* @param dir_position pozice zaznamu o souboru ve slozce vzhledem k celemu disku
* @return zaznam nalezeneho souboru nebo NULL
*/
struct dir_file *fat_get_object_info_by_name(struct fat_data *f_data, const char *file_name, unsigned int file_type, uint32_t act_fat_position, unsigned long *dir_position);

/**
* Smaze soubor ve FAT v danem adresari s danym jmenem.
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param file_name jmeno hledaneho souboru
* @param act_fat_position pozice adresare ve kterem hledame soubor
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_delete_file_by_name(struct fat_data *f_data, const char *file_name, uint32_t act_fat_position);

/**
* Smaze soubor ve FAT v dany zaznamem file a pozici zaznamu position.
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param file zaznam predstavujici soubor, ktery se smaze
* @param position pozice zaznamu v nadrazene slozce vzhledem k celemu fat disku
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_delete_file_by_file(struct fat_data *f_data, struct dir_file *file, unsigned long position);

/**
* Precte dany soubor od dane pozice a vysledek ulozi do buffer.
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param file zaznam souboru ze ktereho se bude cist
* @param buffer pole do ktereho se budou nacitat data
* @param buffer_size velikost bufferu
* @param read pocet prectenych znaku
* @param offset od ktere pozice se bude v souboru cist
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_read_file(struct fat_data *f_data, struct dir_file *file, char *buffer, unsigned int buffer_size, size_t *read, unsigned long offset);

/**
* Zapise data z bufferu do daneho souboru. Zapis zacne od pozice offset a puvodni data se prepisi.
* Offset nesmi byt vetsi nez je velikost souboru. Souboru je zmenena velikost v zavislosti na
* zapsanych datech
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param file zaznam souboru ze ktereho se bude cist
* @param dir_position pozice zaznamu v nadrazene slozce vzhledem k celemu fat disku
* @param buffer pole ze ktereho se budou nacitat data
* @param buffer_size velikost bufferu
* @param writed pocet zapsanych znaku
* @param offset od ktere pozice se bude v souboru zapisovat
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_write_file(struct fat_data *f_data, struct dir_file *file, unsigned long dir_position, char *buffer, unsigned int buffer_size, size_t *writed, unsigned long offset);

/**
* Vytrovy novy prazdny adresar na zvolenem miste.
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param new_dir zaznam adresare, ktery se vytvori
* @param dir_name jmeno hledaneho adresare
* @param act_fat_position pozice adresare ve kterem hledame adresar
* @param dir_position pozice zaznamu vytvoreneho souboru v nadrazene slozce vzhledem k celemu disku
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_create_dir(struct fat_data *f_data, struct dir_file **new_dir, const char *dir_name, uint32_t act_fat_position, unsigned long *dir_position);

/**
* Smaze prazdny adresar ve FAT.
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param dir_name jmeno hledaneho adresare
* @param act_fat_position pozice adresare ve kterem hledame adresar
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_delete_empty_dir(struct fat_data *f_data, const char *dir_name, uint32_t act_fat_position);

/**
* Nacte veskere zaznamy o souborech a slozkach v adresari
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param act_fat_position index do fat tabulky zacatku prohledavane slozky
* @param files pocet vracenych zaznamu
* @param positions pozice zaznamu nactenych souboru v adresari vzhledem k celemu fat disku
* @return pole(pointer) nactenych zaznamu, NULL pri chybe
*/
struct dir_file *fat_read_dir(struct fat_data *f_data, uint32_t act_fat_position, uint32_t *files, unsigned long * positions);

/**
* Nastavi velikost soubour na hodnotu danou file_size a uvolni nepotrebne clustery.
* file_size nesmi byt vetsi nez je skutecna velikost souboru
*
* @param f_data struktura obsahujici informace o nactenem fat disku
* @param file zaznam souboru, ktery se bude zmensovat
* @param file_size nova velikost souboru
* @param dir_position pozice zaznamu v nadrazene slozce vzhledem k celemu fat disku
* @return chybovy kod nebo 0 pri uspechu
*/
int fat_set_file_size(struct fat_data *f_data, struct dir_file * file, size_t file_size, unsigned long dir_position);

#endif