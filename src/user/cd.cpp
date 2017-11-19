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

	if (str.empty()) {
		cd_print();
		//TODO clear remove str?
		return 0; // TODO
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
			else if(tmp.find(":") == std::string::npos) {
				change_dir(tmp);
			}
			else {
				change_dir_with_disk(tmp, change_disk);
			}
			break;
		}
		
		str = match.suffix();
	}

	//TODO clear remove str?
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
		cd_print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, writen);
	if (!res) {
		free(buffer);
		cd_print_error();
		return;
	}

	std::string tmp(buffer);
	tmp.append(delimeter_str).append(path);
	free(buffer);

	path_compiler(tmp);
	res = kiv_os_rtl::Set_Current_Directory(tmp.c_str());
	if (!res) {
		cd_print_error();
		return;
	}
}

/**
* Change working dir with relative path witch starts at root directory.
*
* @param path relative path from root directory
*/
void change_dir_from_root(std::string &path)
{
	size_t writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	if (!buffer) {
		cd_print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, writen);
	if (!res) {
		free(buffer);
		cd_print_error();
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

	path_compiler(tmp);
	res = kiv_os_rtl::Set_Current_Directory(tmp.c_str());
	if (!res) {
		cd_print_error();
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
		cd_print_error("Out of memory.");
		return;
	}

	size_t pos = path.find(':');

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, writen);
	if (!res) {
		free(buffer);
		cd_print_error();
		return;
	}

	if(strncmp(buffer, path.c_str(), pos) && pos + 1 + delimeter_size < path.size() && !change_disk) {
		return;
	}

	path_compiler(path);
	res = kiv_os_rtl::Set_Current_Directory(path.c_str());
	if (!res) {
		cd_print_error();
		return;
	}
}

/**
* Transform absolute path with ".." to valid path.
*/
void path_compiler(std::string &path)
{
	size_t bck_pos = 0;
	while ((bck_pos = path.find("..")) != std::string::npos)
	{
		size_t start_pos = path.rfind(delimeter_chr, bck_pos - 2);

		if (start_pos == std::string::npos) {
			path.erase(bck_pos - delimeter_size, 2 + delimeter_size);
		}
		else {
			path.erase(start_pos, bck_pos - start_pos + 2);
		}
	}
	path.erase(path.find_last_not_of(delimeter_str) + 1);
	path.erase(0, path.find_first_not_of(delimeter_str));
}

/**
 * Print to stdout actual working directory
 */
void cd_print()
{
	size_t read = 0, writen = 0;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	if (!buffer) {
		cd_print_error("Out of memory.");
		return;
	}

	bool res = kiv_os_rtl::Get_Current_Direcotry(buffer, buffer_size, read);
	if (!res) {
		free(buffer);
		cd_print_error();
		return;
	}

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, buffer, read, writen);
	if (!res) {
		free(buffer);
		cd_print_error();
		return;
	}
	free(buffer);

	res = kiv_os_rtl::Write_File(kiv_os::stdOutput, "\n\n", 2, writen);
	if (!res) {	
		cd_print_error();
		return;
	}
}

void cd_print_error()
{
	switch (kiv_os_rtl::Get_Last_Error()) {
		case kiv_os::erInvalid_Handle:
			cd_print_error("Internal error. (Invalid Handle)");
			break;

		case kiv_os::erInvalid_Argument:
			cd_print_error("Invalid input arugments.");
			break;

		case kiv_os::erFile_Not_Found:
			cd_print_error("System can not find path.");
			break;

		case kiv_os::erDir_Not_Empty:
			cd_print_error("Directory is not empty.");
			break;

		case kiv_os::erNo_Left_Space:
			cd_print_error("Out of disk space.");
			break;

		case kiv_os::erPermission_Denied:
			cd_print_error("Operation is not permitted.");
			break;

		case kiv_os::erOut_Of_Memory:
			cd_print_error("Out of memory.");
			break;

		case kiv_os::erIO:
			cd_print_error("Disk error.");
			break;
	}
}

void cd_print_error(std::string msg)
{
	size_t writen = 0;

	msg.append("\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdOutput, msg.c_str(), msg.size(), writen);
	if (!res) {
		return;
	}
}