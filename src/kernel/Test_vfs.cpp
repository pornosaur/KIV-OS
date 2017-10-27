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
	create_file_with_max_name();
	remomve_file();
	remove_file_bigger_than_cluster();
	try_remove_not_existing_file();
	create_dir_in_root();
	create_dir_with_existing_name();
	create_dir_in_full_directory();
	create_dir_not_in_root();
	create_dir_with_too_long_name();
	create_dir_in_not_exist_path();
	


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
	int result = vfs->close_file(&file);
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
	int result = vfs->close_file(&file);
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
	result = vfs->remove_file(&file);
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
	result = vfs->close_file(&file);
	assert(result == 0);

	std::cout << "creating same file again" << std::endl;
	file = vfs->create_file("new.txt");
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_with_max_name()
{
	std::cout << "creating file with maximal name of file filname.txt" << std::endl;

	Vfs::file *file = vfs->create_file("filname.txt");
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_file_type == Vfs::VFS_OBJECT_FILE);
	assert(file->f_dentry->d_count == 1);
	assert(file->f_dentry->d_name == "filname.txt");
	assert(file->f_dentry->d_mounted != 1);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "closing filname.txt" << std::endl;
	int result = vfs->close_file(&file);
	assert(result == 0);
	file = NULL;

	std::cout << "opening filname.txt" << std::endl;
	file = vfs->open_file("filname.txt");
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_name == "filname.txt");
	assert(file->f_dentry->d_mounted != 1);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "removing filname.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remomve_file()
{
	std::cout << "test on removing file" << std::endl;
	std::cout << "creating file file.txt" << std::endl;
	Vfs::file *file = vfs->create_file("file.txt");
	assert(file != NULL);

	std::cout << "closing file.txt" << std::endl;
	int result = vfs->close_file(&file);
	assert(result == 0);
	file = NULL;

	std::cout << "opening file.txt" << std::endl;
	file = vfs->open_file("file.txt");
	assert(file != NULL);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);

	std::cout << "opening file.txt" << std::endl;
	file = vfs->open_file("file.txt");
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_file_bigger_than_cluster()
{
	std::cout << "test on removing file which is bigger than one cluster" << std::endl;
	std::cout << "creating file file.txt" << std::endl;
	Vfs::file *file = vfs->create_file("file.txt");
	assert(file != NULL);

	char * buffer = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
	int buff_size = 208;

	int result = vfs->write_to_file(file, buffer, buff_size);
	assert(result == buff_size);

	std::cout << "closing file.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == 0);
	file = NULL;

	std::cout << "opening file.txt" << std::endl;
	file = vfs->open_file("file.txt");
	assert(file != NULL);
	assert(file->f_dentry->d_size == buff_size);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);

	std::cout << "opening file.txt" << std::endl;
	file = vfs->open_file("file.txt");
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_remove_not_existing_file()
{
	std::cout << "removing not existing file(dentry == NULL)" << std::endl;

	std::cout << "removing no.txt from FAT" << std::endl;
	Vfs::file *file = new Vfs::file();
	file->f_dentry = NULL;
	file->f_count = 1;
	file->position = 2342;

	int result = vfs->remove_file(&file);
	assert(result == -1);

	vfs->close_file(&file);
	

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_root()
{
	std::cout << "createing normal dir in root with name directory" << std::endl;

	Vfs::file *dir = vfs->create_dir("directory");
	assert(dir != NULL);
	assert(dir->f_dentry != NULL);
	assert(dir->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	int result = vfs->remove_emtpy_dir(&dir);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_existing_name()
{
	std::cout << "createing normal dir with name directory" << std::endl;

	Vfs::file *dir = vfs->create_dir("directory");
	assert(dir != NULL);
	assert(dir->f_dentry != NULL);
	assert(dir->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	std::cout << "closing direcotry" << std::endl;
	int result = vfs->close_file(&dir);
	assert(result == 0);

	std::cout << "createing dir with same name (directory)" << std::endl;
	dir = vfs->create_dir("directory");
	assert(dir != NULL);
	assert(dir->f_dentry != NULL);
	assert(dir->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_full_directory()
{
	std::cout << "createing dir with name directory1" << std::endl;

	Vfs::file *dir1 = vfs->create_dir("directory1");
	assert(dir1 != NULL);
	assert(dir1->f_dentry != NULL);
	assert(dir1->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	std::cout << "createing another dir with name directory2" << std::endl;
	Vfs::file *dir2 = vfs->create_dir("directory2");
	assert(dir2 == NULL);

	std::cout << "removing direcotry1" << std::endl;
	int result = vfs->remove_emtpy_dir(&dir1);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_not_in_root()
{
	std::cout << "createing normal dir in root with name directory" << std::endl;

	Vfs::file *dir1 = vfs->create_dir("directory");
	assert(dir1 != NULL);
	assert(dir1->f_dentry != NULL);
	assert(dir1->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir1->f_dentry->d_subdirectories == NULL);

	Vfs::file *dir2 = vfs->create_dir("directory/directory");
	assert(dir2 != NULL);
	assert(dir2->f_dentry != NULL);
	assert(dir2->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir2->f_dentry->d_parent != NULL);

	assert(dir1->f_dentry->d_subdirectories != NULL);

	std::cout << "removing dir2" << std::endl;
	int result = vfs->remove_emtpy_dir(&dir2);
	assert(result == 0);

	std::cout << "removing dir1" << std::endl;
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_too_long_name()
{
	std::cout << "createing dir with too long name" << std::endl;

	Vfs::file *dir = vfs->create_dir("very_very_long_name_for_simple_direcotry");
	assert(dir == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_not_exist_path()
{
	std::cout << "createing dir with too long name" << std::endl;

	Vfs::file *dir = vfs->create_dir("direcotry/direcotry/dir");
	assert(dir == NULL);

	std::cout << "OK\n" << std::endl;
}
