#include "cd.h"

/**
* Command cd
*
* @param parameters string contains argument /d and path
*/
size_t cmd_cd(const std::string &parameters)
{
	std::smatch match;
	bool change_disk = false;
	std::string str(parameters);

	str.erase(str.find_last_not_of(erase_chars) + 1);
	str.erase(0, str.find_first_not_of(erase_chars));
	
	if (str.empty()) {
		cd_print();
		return 0; // TODO
	}
	if (!str.find("/?")) {
		cd_print_help();
		return 0;
	}
	if (str == ".") {
		return 0;
	}

	while (!str.empty() && std::regex_search(str, match, reg_cd)) {

		std::string tmp = match[2].str();
		if (!tmp.empty()) {
			change_disk = true;
		}

		tmp = match[1].str();
		if (!tmp.empty()) {
			tmp.erase(tmp.find_last_not_of(erase_chars) + 1);
			tmp.erase(0, tmp.find_first_not_of(erase_chars));

			if (tmp[0] == delimeter_chr) {
				change_dir_from_root(tmp.erase(0, 1));
			}
			else if (tmp.find(":") == std::string::npos) {
				change_dir(tmp);
			}
			else {
				change_dir_with_disk(tmp, change_disk);
			}
			break;
		}

		str = match.suffix();
	}

	return 0; // TODO what return
}

/**
* Change working dir with relative path.
*
* @param path relative path
*/
void change_dir(std::string &path)
{
	std::smatch match;

	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	std::string tmp(buffer);
	tmp.append(delimeter_str).append(path);
	free(buffer);

	res = kiv_os_rtl::Set_Current_Directory(tmp.c_str());
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}

/**
* Change working dir with relative path which starts at root directory.
*
* @param path relative path from root directory
*/
void change_dir_from_root(std::string &path)
{
	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	char * first = strchr(buffer, ':');

	if (first) {
		*(first + 1) = '\0';
	}
	else {
		free(buffer);
		return;
	}

	std::string tmp(buffer);
	tmp.append(delimeter_str).append(path);

	free(buffer);

	res = kiv_os_rtl::Set_Current_Directory(tmp.c_str());
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}

/**
* Change working dir with absolute path.
*
* @param path absolute path
* @param change_disk must be true to change disk and folder. To change only disk can be false
*/
void change_dir_with_disk(std::string &path, bool change_disk)
{
	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	size_t pos = path.find(':');

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	if (strncmp(buffer, path.c_str(), pos) && pos + 1 + delimeter_size < path.size() && !change_disk) {
		free(buffer);
		return;
	}
	free(buffer);

	res = kiv_os_rtl::Set_Current_Directory(path.c_str());
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}

/**
* Print to stdOutput actual working directory.
*/
void cd_print()
{
	size_t read = 0, writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, read);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, buffer, read, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}
	free(buffer);

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, "\n\n", 2, writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}

/**
* Function print help for command CD to stdOutput
*/
void cd_print_help()
{
	size_t writen;

	std::string text("Displays the name of or changes the current directory.\n\nCD[/D][drive:][path]\nCD[..]\n\n\t.. Specifies that you want to change to the parent directory.\n\nType CD without parameters to display the current drive and directory.\n\nUse the /D switch to change current drive in addition to changing current directory for a drive.\n\nFor names with spaces use double quotes[\"] or quotes['].\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, text.c_str(), text.size(), writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}