#undef stdin
#undef stdout
#undef stderr
#include "../api/api.h"
#include "string.h"

#include <regex>

/* chars which are deleted from input arguments from left and right */
#define ERASE_CHARS " \n\r\t\"'\\"
/* path delimeter */
#define DELIMETER "\\"
/* size of read buffer */
#define BUFFER_SIZE 1024

static std::regex reg_dir("(/s|/S)|\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

/**
* Command DIR
*/
extern "C" size_t __stdcall dir(const kiv_os::TRegisters &regs);

/**
* Print content of directory to stdOutput. If recursively is set to true, content of subdirectories will be also printed.
*
* @param path of directory which will be printed
* @param recursively if true, content of subdirectories will be also printed
* @return true on success, false otherwise
*/
bool print_directory(std::string path, const kiv_os::TRegisters &regs, bool &recursively);

/**
* Print name of input dentry to stdOutput
* If entry is directory text <DIR> will be also printed
*
* @return true on success, false otherwise
*/
bool print_entry(kiv_os::TDir_Entry *entry);

/**
* Print input parameter name to stdOutput
*
* @return true on success, false otherwise
*/
bool print_name(std::string name);

/**
* Return current working directory in input parameter path
*
* @param path will contain working directory at the end
* @return true on success, false otherwise
*/
bool get_current_dir(std::string &path);

/**
* Check if parameters starts with /s /S.
* If starts with /s or /S set recursively to true.
* Remove founded strings from parameters.
*/
void check_params(std::string &parameters, bool &recursively);

/**
* Function print help for command DIR to stdOutput
*/
void dir_print_help();