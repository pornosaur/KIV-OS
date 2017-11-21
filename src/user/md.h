#include "../api/api.h"
#include "string.h"

#include <regex>

/* chars which are deleted from input arguments from left and right */
#define ERASE_CHARS " \n\r\t\"'\\"
/* path delimeter */
#define DELIMETER "\\"

static std::regex reg_md_multi("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");
static std::regex reg_md_recur("[^\\\\]+");

/**
* Command MD
*/
extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs);

/**
* Create every folder in path if no folder is created print error message to stdError
*/
void create_directories(std::string &path);

/**
* Function print help for command MD to stdOutput
*/
void md_print_help();