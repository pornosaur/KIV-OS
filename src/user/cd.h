#include "../api/api.h"
#include "string.h"
#include "rtl.h"

#include <regex>

#define erase_chars " \n\r\t\"'"
#define delimeter_chr '\\'
#define delimeter_str "\\"
#define delimeter_size 1
#define buffer_size 2048

static std::regex reg_cd("([^\\/]+)|(\\/d|\\/D)");

size_t cmd_cd(const std::string &arguments);

void change_dir(std::string &path);
void change_dir_from_root(std::string &path);
void change_dir_with_disk(std::string &path, bool change_disk);
void path_compiler(std::string &path);
void print();
void print_error();
void print_error(std::string msg);