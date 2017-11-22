#pragma once
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

static std::regex reg_type("\"([\\S\\s][^\"]+)\"|\'([\\S\\s][^\']+)\'|(\\S+)");

/**
* Command TYPE
*/
extern "C" size_t __stdcall type(const kiv_os::TRegisters &regs);

/**
* write all data from in handle to stdOutput
*
* @param in handle from which will be read data
*/
void read_and_write(kiv_os::THandle &in);

/**
* if counter != 0 write name to stdOutput
*
* @param print if is set to non zero value name will be printed
* @param name string which will be printed
*/
void write_file_name(int &counter, std::string &name);

/**
* return true if name is in lower case  equals to string
*
* @param name string which will be transfer to lower case
* @param second string to campare
*/
bool is_string_name_lower(std::string name, std::string string);

/**
* Function print help for command TYPE to stdOutput
*/
void type_print_help();