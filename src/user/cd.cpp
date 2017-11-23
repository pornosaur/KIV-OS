#include "cd.h"
#include "rtl.h"

size_t cmd_cd(const std::string &parameters)
{
	std::smatch match;
	bool change_disk = false;
	std::string str(parameters);

	str.erase(str.find_last_not_of(ERASE_CHARS) + 1);
	str.erase(0, str.find_first_not_of(ERASE_CHARS));
	
	if (str.empty()) {
		cd_print();
		return 0;
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
			tmp.erase(tmp.find_last_not_of(ERASE_CHARS) + 1);
			tmp.erase(0, tmp.find_first_not_of(ERASE_CHARS));

			if (tmp[0] == DELIMETER_CHR) {
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

	return 0;
}


void change_dir(std::string &path)
{
	std::smatch match;

	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, BUFFER_SIZE, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	std::string tmp(buffer);
	tmp.append(DELIMETER_STR).append(path);
	free(buffer);

	res = kiv_os_rtl::Set_Current_Directory(tmp.c_str());
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}


void change_dir_from_root(std::string &path)
{
	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, BUFFER_SIZE, writen);
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
	tmp.append(DELIMETER_STR).append(path);

	free(buffer);

	res = kiv_os_rtl::Set_Current_Directory(tmp.c_str());
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}


void change_dir_with_disk(std::string &path, bool change_disk)
{
	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	size_t pos = path.find(':');

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, BUFFER_SIZE, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	if (strncmp(buffer, path.c_str(), pos) && pos + 1 + DELIMETER_SIZE < path.size() && !change_disk) {
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


void cd_print()
{
	size_t read = 0, writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	if (!buffer) {
		kiv_os_rtl::print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, BUFFER_SIZE-1, read);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}

	buffer[read] = '\n';

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, buffer, read + 1, writen);
	if (!res) {
		free(buffer);
		kiv_os_rtl::print_error();
		return;
	}
	free(buffer);
}


void cd_print_help()
{
	size_t writen;

	std::string text("Displays the name of or changes the current directory.\n\nCD[/D][drive:][path]\nCD[..]\n\n\t.. Specifies that you want to change to the parent directory.\n\nType CD without parameters to display the current drive and directory.\n\nUse the /D switch to change current drive in addition to changing current directory for a drive.\n\nFor names with spaces use double quotes[\"] or quotes['].\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, text.c_str(), text.size(), writen);
	if (!res) {
		kiv_os_rtl::print_error();
		return;
	}
}