#include "Test_vfs.h"

Test_vfs::Test_vfs()
{
	init();

	open_existing_file();
	open_not_exist_file();
	open_file_with_too_long_name();
	create_new_file();
	create_new_file_with_long_name();
	create_new_file_with_bad_path();
	create_existing_file();



	system("pause");
}

Test_vfs::~Test_vfs()
{
	term();
}


void Test_vfs::init() {
	std::cout << "initialize fat" << std::endl;
	vfs = new VfsFat();
	
	assert(vfs != NULL);
}

void Test_vfs::term()
{
	std::cout << "terminate fat" << std::endl;
	delete vfs;
}

void Test_vfs::open_existing_file()
{
	std::cout << "opennig existing file" << std::endl;
	
	Vfs::file *file = vfs->open_file("a.txt");
	
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_file_type == Vfs::VFS_OBJECT_FILE);
	assert(file->f_dentry->d_count == 1);

	std::cout << "closing file" << std::endl;
	int result = vfs->close_file(file);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_file()
{
	std::cout << "opennig not existing file" << std::endl;

	Vfs::file *file = vfs->open_file("not.txt");

	assert(file == NULL);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_file_with_too_long_name()
{
	std::cout << "opennig not existing file with long name of file" << std::endl;

	Vfs::file *file = vfs->open_file("not_existing_file_with_very_very_long_name.txt");

	assert(file == NULL);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file()
{
	std::cout << "creating valid file with name new.txt" << std::endl;

	Vfs::file *file = vfs->create_file("new.txt");

	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_file_type == Vfs::VFS_OBJECT_FILE);
	assert(file->f_dentry->d_count == 1);
	assert(file->f_dentry->d_name == "new.txt");
	assert(file->f_dentry->d_mounted != 1);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "closing new.txt" << std::endl;
	int result = vfs->close_file(file);
	assert(result == 0);
	file = NULL;

	std::cout << "opening new.txt" << std::endl;
	file = vfs->open_file("new.txt");
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_name == "new.txt");
	assert(file->f_dentry->d_mounted != 1);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file(file);
	assert(result == 0);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_with_long_name()
{
	std::cout << "creating file with long name" << std::endl;

	Vfs::file *file = vfs->create_file("new_file_with_very_long_name.txt");
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_with_bad_path()
{
	std::cout << "creating file with bad file path" << std::endl;

	Vfs::file *file = vfs->create_file("folder/new.txt");
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_existing_file()
{
	std::cout << "creating file over exist file" << std::endl;

	Vfs::file *file = vfs->create_file("new.txt");
	assert(file != NULL);

	char buffer[] = "text of new file with name new.txt";
	int buff_size = 34;

	std::cout << "write data to file new.txt" << std::endl;
	int result = vfs->write_to_file(file, buffer, buff_size);
	assert(file->f_dentry->d_size > 1);
	assert(file->f_dentry->d_blocks == 1);
	assert(result == buff_size);

	std::cout << "closing file new.txt" << std::endl;
	result = vfs->close_file(file);
	assert(result == 0);

	std::cout << "creating same file again" << std::endl;
	file = vfs->create_file("new.txt");
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file(file);
	assert(result == 0);
	std::cout << "OK\n" << std::endl;
}


