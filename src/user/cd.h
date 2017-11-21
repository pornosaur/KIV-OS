#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

/* chars which are deleted from input arguments from left and right */
#define erase_chars " \n\r\t\"'"
/* path delimeter in char */
#define delimeter_chr '\\'
/* path delimeter in string */
#define delimeter_str "\\"
/* size of path delimeter */
#define delimeter_size 1
/* size of read buffer */
#define buffer_size 2048

static std::regex reg_cd("([^\\/]+)|(\\/d|\\/D)");

/**
* Command cd
*
* @param parameters string contains argument /d and path
*/
size_t cmd_cd(const std::string &arguments);

/**
* Change working dir with relative path.
*
* @param path relative path
*/
void change_dir(std::string &path);

/**
* Change working dir with relative path which starts at root directory.
*
* @param path relative path from root directory
*/
void change_dir_from_root(std::string &path);

/**
* Change working dir with absolute path.
*
* @param path absolute path
* @param change_disk must be true to change disk and folder. To change only disk can be false
*/
void change_dir_with_disk(std::string &path, bool change_disk);

/**
* Print to stdOutput actual working directory.
*/
void cd_print();

/**
* Function print help for command CD to stdOutput
*/
void cd_print_help();