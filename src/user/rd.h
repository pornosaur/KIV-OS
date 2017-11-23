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

static std::regex reg_rd("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");
static std::regex reg_rd_params("(/s|/S)|(/q|/Q)|\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

/**
* Command RD
*/
extern "C" size_t __stdcall rd(const kiv_os::TRegisters regs);

/**
* Recursively remove folder and every subfolders and subfiles.
*
* @param path path to file/folder which will be removed
* @return true on success, false otherwise
*/
bool remove_recursively(std::string &path, const kiv_os::TRegisters &regs);

/**
* Remove every files in folder and call remove_recursively to all subfolders
*
* @param path to file which subfiles/subfolders will be removed
* @return true on success, false otherwise
*/
bool remove_subfiles(std::string path, const kiv_os::TRegisters &regs);

/**
* Check if parameters starts with /q /Q /s /S.
* If starts with /q or /Q set quiet to true and when starts with /s or /S set recursively to true.
* Remove founded strings from parameters
*/
void check_params(std::string &parameters, bool &recursively, bool &quiet);

/**
* Print to stdOut text path and text "Are you sure (y/n)?" and wait for response.
* Return true if answer was y(yes). On answer n(no) or on error return false.
*/
bool ask_for_deletion(std::string &path);

/**
* Function print help for command RD to stdOutput
*/
void rd_print_help();