#include "Test_vfs.h"

Test_vfs::Test_vfs()
{
	init();

	open_existing_file();
	open_not_exist_file();
	open_file_with_too_long_name();
	create_new_file();
	create_new_file_not_in_root();
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
	remove_dir();
	try_delete_not_empty_dir();
	try_delete_not_exist_dir();
	try_delete_root();
	open_normal_dir();
	open_not_exist_dir();
	open_dir_with_long_name();
	write_read_file();
	read_file_from_exact_position();
	write_read_file_bigger_than_one_cluster();
	write_read_exactly_one_cluster();
	read_small_piece_of_file();
	read_all_file_by_pieces();
	rewrite_file();
	write_zero_bytes_to_file();
	create_file_with_space_in_name();
	create_dir_with_space_in_name();
	folder_dir_same_name();
	remove_root();
	test_more_subfiles();
	test_close_all_dentry_memory_leak();
	write_to_twice_open_file();
	open_dir_twice();
	remove_twice_open_file();
	remove_twice_open_dir();
	create_file_over_open_file();

	system("pause");
}

Test_vfs::~Test_vfs()
{
	term();
}


void Test_vfs::init() {
	std::cout << "initialize fat" << std::endl;
	memory_size = 4096;
	memory = new char[memory_size];

	FatFS::init_fat_disk(memory, memory_size, 128u);
	FS *fs = new FatFS(memory, memory_size, "C:");
	vfs = new Vfs();
	vfs->register_fs("C:", fs);
	
	assert(vfs != NULL);
}

void Test_vfs::term()
{
	std::cout << "terminate fat" << std::endl;

	delete[] memory;
	delete vfs;
}

void Test_vfs::open_existing_file()
{
	std::cout << "opennig existing file" << std::endl;
	
	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/file.text");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&file, "C:/file.text", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);

	std::cout << "removing file" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_file()
{
	std::cout << "opennig not existing file" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->open_object(&file, "C:/not.txt", FS::FS_OBJECT_FILE);

	assert(file == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_file_with_too_long_name()
{
	std::cout << "opennig not existing file with long name of file" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->open_object(&file, "C:/not_existing_file_with_very_very_long_name.txt", FS::FS_OBJECT_FILE);

	assert(file == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file()
{
	std::cout << "creating valid file with name new.txt" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/new.txt");

	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "closing new.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);
	file = NULL;

	std::cout << "opening new.txt" << std::endl;
	result = vfs->open_object(&file, "C:/new.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_not_in_root()
{
	std::cout << "creating file with name new.txt in directory dir" << std::endl;

	std::cout << "creating dir" << std::endl;
	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/dir");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "creating file" << std::endl;
	FileHandler *file = NULL;
	result = vfs->create_file(&file, "C:/dir/new.txt");

	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "closing new.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);
	file = NULL;

	std::cout << "opening new.txt" << std::endl;
	result = vfs->open_object(&file, "C:/dir/new.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "removing file adn folder" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_with_long_name()
{
	std::cout << "creating file with long name" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/new_file_with_very_long_name.txt");
	assert(file == NULL);
	assert(result == FS::ERR_INVALID_ARGUMENTS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_with_bad_path()
{
	std::cout << "creating file with bad file path" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/folder/new.txt");
	assert(file == NULL);
	assert(result == FS::ERR_INVALID_PATH);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_existing_file()
{
	std::cout << "creating file over exist file" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/new.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	char buffer[] = "text of new file with name new.txt";
	int buff_size = 34;

	std::cout << "write data to file new.txt" << std::endl;
	size_t bytes = 0;
	result = vfs->write_to_file(file, &bytes, buffer, buff_size);
	assert(file->get_dentry()->d_size > 1);
	assert(file->get_dentry()->d_blocks == 1);
	assert(result == FS::ERR_SUCCESS);
	assert(bytes == buff_size);

	std::cout << "closing file new.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "creating same file again" << std::endl;
	result = vfs->create_file(&file, "C:/new.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_with_max_name()
{
	std::cout << "creating file with maximal name of file filname.txt" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/filname.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_dentry()->d_name == "filname.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "closing filname.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);
	file = NULL;

	std::cout << "opening filname.txt" << std::endl;
	result = vfs->open_object(&file, "C:/filname.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_name == "filname.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "removing filname.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remomve_file()
{
	std::cout << "test on removing file" << std::endl;
	std::cout << "creating file file.txt" << std::endl;
	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/file.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "closing file.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);
	file = NULL;

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_file_bigger_than_cluster()
{
	std::cout << "test on removing file which is bigger than one cluster" << std::endl;
	std::cout << "creating file file.txt" << std::endl;
	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/file.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	char * buffer = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
	int buff_size = 208;

	size_t bytes = 0;
	result = vfs->write_to_file(file, &bytes, buffer, buff_size);
	assert(result == FS::ERR_SUCCESS);
	assert(bytes == buff_size);

	std::cout << "closing file.txt" << std::endl;
	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry()->d_size == buff_size);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_remove_not_existing_file()
{
	std::cout << "removing not existing file(dentry == NULL)" << std::endl;

	FileHandler *file = new FileHandler(0, NULL, 1, 2342);

	int result = vfs->remove_file(&file);
	assert(result == FS::ERR_INVALID_ARGUMENTS);

	vfs->close_file(&file);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_root()
{
	std::cout << "createing normal dir in root with name directory" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_existing_name()
{
	std::cout << "createing normal dir with name directory" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	std::cout << "closing direcotry" << std::endl;
	result = vfs->close_file(&dir);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "createing dir with same name (directory)" << std::endl;
	result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_full_directory()
{
	std::cout << "createing dir with name directory1" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/directory1");
	assert(dir1 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir1->get_dentry() != NULL);
	assert(dir1->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	FileHandler *file1 = NULL;
	result = vfs->create_file(&file1, "C:/file1");
	assert(file1 != NULL);
	assert(result == FS::ERR_SUCCESS);

	FileHandler *file2 = NULL;
	result = vfs->create_file(&file2, "C:/file2");
	assert(file2 != NULL);
	assert(result == FS::ERR_SUCCESS);

	FileHandler *file3 = NULL;
	result = vfs->create_file(&file3, "C:/file3");
	assert(file3 != NULL);
	assert(result == FS::ERR_SUCCESS);

	FileHandler *file4 = NULL;
	result = vfs->create_file(&file4, "C:/file4");
	assert(file4 != NULL);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "createing another dir with name directory2" << std::endl;
	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory2");
	assert(dir2 == NULL);
	assert(result == FS::ERR_DIRECTORY_IS_FULL);

	std::cout << "removing direcotry1" << std::endl;
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_file(&file1);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_file(&file2);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_file(&file3);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_file(&file4);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_not_in_root()
{
	std::cout << "createing normal dir in root with name directory" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/directory");
	assert(dir1 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir1->get_dentry() != NULL);
	assert(dir1->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	assert(dir1->get_dentry()->d_subdirectories == NULL);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory/directory");
	assert(dir2 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir2->get_dentry() != NULL);
	assert(dir2->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	assert(dir2->get_dentry()->d_parent != NULL);

	assert(dir1->get_dentry()->d_subdirectories != NULL);

	result = vfs->close_file(&dir1);
	assert(result == 0);
	result = vfs->close_file(&dir2);
	assert(result == 0);

	result = vfs->open_object(&dir2, "C:/directory/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir2 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir2->get_dentry()->d_parent != NULL);
	result = vfs->open_object(&dir1, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir1 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir1->get_dentry()->d_subdirectories != NULL);

	std::cout << "removing dir2" << std::endl;
	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == 0);

	std::cout << "removing dir1" << std::endl;
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_too_long_name()
{
	std::cout << "createing dir with too long name" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/very_very_long_name_for_simple_direcotry");
	assert(dir == NULL);
	assert(result == FS::ERR_INVALID_ARGUMENTS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_not_exist_path()
{
	std::cout << "createing dir with too long name" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/direcotry/direcotry/dir");
	assert(dir == NULL);
	assert(result == FS::ERR_INVALID_PATH);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_dir()
{
	std::cout << "test for remove dir in root with name directory" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);
	assert(dir == NULL);

	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_not_empty_dir()
{
	std::cout << "test for remove not empty dir" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/directory");
	assert(dir1 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir1->get_dentry() != NULL);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory/directory");
	assert(dir2 != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir2->get_dentry() != NULL);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == FS::ERR_FILE_OPEN_BY_OTHER);

	result = vfs->close_file(&dir2);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == FS::ERR_DIRECTORY_IS_NOT_EMPTY);

	result = vfs->open_object(&dir2, "C:/directory/directory", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == FS::ERR_SUCCESS);
	
	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_not_exist_dir()
{
	std::cout << "removing not existing dir(dentry == NULL)" << std::endl;

	FileHandler *dir = new FileHandler(0, NULL, 1, 2342);

	int result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_INVALID_ARGUMENTS);

	vfs->close_file(&dir);


	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_root()
{
	// TODO need open root
}

void Test_vfs::open_normal_dir()
{
	std::cout << "testing dir open" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	result = vfs->close_file(&dir);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_dir()
{
	std::cout << "testing open not existing dir" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_dir_with_long_name()
{
	std::cout << "testing open dir with long name" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->open_object(&dir, "C:/directory_with_very_very_long_name", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_file()
{
	std::cout << "reading and writing to file" << std::endl;
	
	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	
	int buff_size = 100;
	char buffer[100];

	size_t bytes = 0;
	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 1);
	assert(result == FS::ERR_SUCCESS);

	char text[] = "small text";
	result = vfs->write_to_file(file, &bytes, text, 10);
	assert(bytes == 10);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(result == FS::ERR_SUCCESS);
	assert(bytes == 10);
	assert(strncmp(buffer, text, 10) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);;
	assert(file == NULL);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_file_from_exact_position()
{
	std::cout << "reading file from exact position" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 100;
	char buffer[100];
	size_t bytes = 0;

	std::cout << "writing text to file" << std::endl;
	char *text = "text which contains exactly 36 words";
	result = vfs->write_to_file(file, &bytes, text, 36);
	assert(result == FS::ERR_SUCCESS);
	assert(bytes == 36);

	
	file->set_position(10);
	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 26);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text + 10, 26) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_file_bigger_than_one_cluster()
{
	std::cout << "reading and writing to file bigger than one cluster" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 200;
	char buffer[200];
	size_t bytes = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = vfs->write_to_file(file, &bytes, text, 200);
	assert(bytes == 200);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 200);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text, 200) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_exactly_one_cluster()
{
	std::cout << "reading and writing exactly one cluster" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 200;
	char buffer[200];
	size_t bytes = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor.";
	result = vfs->write_to_file(file, &bytes, text, 128);
	assert(bytes == 128);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 128);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text, 128) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_small_piece_of_file()
{
	std::cout << "reading only piece of file" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 30;
	char buffer[30];
	size_t bytes = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = vfs->write_to_file(file, &bytes, text, 200);
	assert(bytes == 200);
	assert(result == FS::ERR_SUCCESS);

	file->set_position(120);
	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 30);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text + 120, 30) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_all_file_by_pieces()
{
	std::cout << "reading all file by pieces" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 31;
	char buffer[31];
	size_t bytes = 0;
	int shuld_read = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = vfs->write_to_file(file, &bytes, text, 200);
	assert(bytes == 200);
	assert(result == FS::ERR_SUCCESS);


	shuld_read = 31;
	for (int i = 0; i < 200; i += 31) {
		file->set_position(i);
		if (200 - i < shuld_read) {
			shuld_read = 200 - i;
		}

		result = vfs->read_file(file, &bytes, buffer, buff_size);
		assert(bytes == shuld_read);
		assert(result == FS::ERR_SUCCESS);
		assert(strncmp(buffer, text + i, shuld_read) == 0);
	}

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::rewrite_file()
{
	std::cout << "rewriting file" << std::endl;

	std::cout << "creating empty file" << std::endl;
	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 200;
	char buffer[200];
	size_t bytes = 0;

	std::cout << "writing short text" << std::endl;
	char text1[] = "small text";
	result = vfs->write_to_file(file, &bytes, text1, 10);
	assert(bytes == 10);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 10);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text1, 10) == 0);


	std::cout << "writing long text" << std::endl;
	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = vfs->write_to_file(file, &bytes, text, 200);
	assert(bytes == 200);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 200);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text, 200) == 0);

	std::cout << "writing short text" << std::endl;
	file->set_position(10);
	result = vfs->write_to_file(file, &bytes, text1, 10);
	assert(bytes == 10);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry()->d_size == 20);

	file->set_position(0);
	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 20);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text, 10) == 0);
	assert(strncmp(buffer + 10, text1, 10) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_zero_bytes_to_file()
{
	std::cout << "writing zero bytes to file" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);

	int buff_size = 30;
	char buffer[30];
	size_t bytes = 0;

	char text[] = "";
	result = vfs->write_to_file(file, &bytes, text, 0);
	assert(bytes == 0);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file, &bytes, buffer, buff_size);
	assert(bytes == 1);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_with_space_in_name()
{
	std::cout << "creating file with space in name" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/my file.txt");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(strcmp("my file.txt", file->get_dentry()->d_name.c_str()) == 0);

	result = vfs->close_file(&file);
	assert(result == 0);
	assert(file == NULL);

	result = vfs->open_object(&file, "C:/my file.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	
	char text[] = "short text";
	size_t bytes = 0;
	result  = vfs->write_to_file(file, &bytes, text, 10);
	assert(bytes == 10);
	assert(result == FS::ERR_SUCCESS);

	char buffer[20];
	result = vfs->read_file(file, &bytes, buffer, 20);
	assert(bytes == 10);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(text, buffer, 10) == 0);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);

	result = vfs->open_object(&file, "C:/my file.txt", FS::FS_OBJECT_FILE);
	assert(file == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_space_in_name()
{
	std::cout << "creating directory with space in name" << std::endl;

	FileHandler *dir = NULL;
	int result = vfs->create_dir(&dir, "C:/my dir");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(strcmp("my dir", dir->get_dentry()->d_name.c_str()) == 0);

	result = vfs->close_file(&dir);
	assert(result == FS::ERR_SUCCESS);
	assert(dir == NULL);

	result = vfs->open_object(&dir, "C:/my dir", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);
	assert(dir == NULL);

	result = vfs->open_object(&dir, "C:/my dir", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == FS::ERR_FILE_NOT_FOUND);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::folder_dir_same_name()
{
	std::cout << "test for same name for folder and file" << std::endl;

	FileHandler *file = NULL;
	int result = vfs->create_file(&file, "C:/name");
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	
	FileHandler *dir = NULL;
	result = vfs->create_dir(&dir, "C:/name");
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	result = vfs->close_file(&file);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir, "C:/name", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	result = vfs->open_object(&file, "C:/name", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == FS::ERR_SUCCESS);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);

	result = vfs->remove_emtpy_dir(&dir);
	assert(result == FS::ERR_SUCCESS);
	assert(dir == NULL);

	result = vfs->remove_file(&file);
	assert(result == FS::ERR_SUCCESS);
	assert(file == NULL);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_root()
{
	std::cout << "removing root" << std::endl;

	FileHandler *root = NULL;
	int result = vfs->open_object(&root, "C:", FS::FS_OBJECT_DIRECTORY);
	assert(root != NULL);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->remove_emtpy_dir(&root);
	assert(result == FS::ERR_INVALID_PATH);

	result = vfs->close_file(&root);
	assert(result == FS::ERR_SUCCESS);
	assert(result == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::test_more_subfiles()
{
	std::cout << "test with more subdirectories" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/dir1");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/dir2");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir3 = NULL;
	result = vfs->create_dir(&dir3, "C:/dir1/dir3");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir4 = NULL;
	result = vfs->create_dir(&dir4, "C:/dir2/dir4");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir5 = NULL;
	result = vfs->create_dir(&dir5, "C:/dir2/dir5");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir6 = NULL;
	result = vfs->create_dir(&dir6, "C:/dir2/dir6");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir7 = NULL;
	result = vfs->create_dir(&dir7, "C:/dir2/dir5/dir7");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir8 = NULL;
	result = vfs->create_dir(&dir8, "C:/dir2/dir5/dir7/dir8");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir9 = NULL;
	result = vfs->create_dir(&dir9, "C:/dir2/dir5/dir9");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir10 = NULL;
	result = vfs->create_dir(&dir10, "C:/dir10");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir11 = NULL;
	result = vfs->create_dir(&dir11, "C:/dir10/dir11");
	assert(result == FS::ERR_SUCCESS);

	result = vfs->close_file(&dir1);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir2);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir3);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir4);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir5);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir6);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir7);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir8);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir9);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir10);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->close_file(&dir11);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir8, "C:/dir2/dir5/dir7/dir8", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir8);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir2, "C:/dir2", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->open_object(&dir10, "C:/dir10", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir7, "C:/dir2/dir5/dir7", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir7);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir9, "C:/dir2/dir5/dir9", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir9);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir11, "C:/dir10/dir11", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir11);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir3, "C:/dir1/dir3", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir3);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir4, "C:/dir2/dir4", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir4);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir1, "C:/dir1", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir5, "C:/dir2/dir5", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir5);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->open_object(&dir6, "C:/dir2/dir6", FS::FS_OBJECT_DIRECTORY);
	assert(result == FS::ERR_SUCCESS);
	result = vfs->remove_emtpy_dir(&dir6);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->remove_emtpy_dir(&dir10);
	assert(result == FS::ERR_SUCCESS);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::test_close_all_dentry_memory_leak()
{
	std::cout << "test for close all open dentry (memory leak)" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/dir1");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/dir2");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir3 = NULL;
	result = vfs->create_dir(&dir3, "C:/dir1/dir3");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir4 = NULL;
	result = vfs->create_dir(&dir4, "C:/dir2/dir4");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir5 = NULL;
	result = vfs->create_dir(&dir5, "C:/dir2/dir5");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir6 = NULL;
	result = vfs->create_dir(&dir6, "C:/dir2/dir6");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir7 = NULL;
	result = vfs->create_dir(&dir7, "C:/dir2/dir5/dir7");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir8 = NULL;
	result = vfs->create_dir(&dir8, "C:/dir2/dir5/dir7/dir8");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir9 = NULL;
	result = vfs->create_dir(&dir9, "C:/dir2/dir5/dir9");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir10 = NULL;
	result = vfs->create_dir(&dir10, "C:/dir10");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir11 = NULL;
	result = vfs->create_dir(&dir11, "C:/dir10/dir11");
	assert(result == FS::ERR_SUCCESS);

	delete dir1;
	delete dir2;
	delete dir3;
	delete dir4;
	delete dir5;
	delete dir6;
	delete dir7;
	delete dir8;
	delete dir9;
	delete dir10;
	delete dir11;

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_to_twice_open_file()
{
	std::cout << "opening same file twice" << std::endl;

	FileHandler *file1 = NULL;
	FileHandler *file2 = NULL;

	int result = vfs->create_file(&file1, "C:/file.txt");
	assert(result == FS::ERR_SUCCESS);
	result = vfs->open_object(&file2, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(result == FS::ERR_SUCCESS);

	assert(file1 != NULL);
	assert(file1->get_dentry() != NULL);
	assert(file2 != NULL);
	assert(file2->get_dentry() != NULL);
	assert(file1->get_dentry() == file2->get_dentry());
	assert(file1->get_dentry()->d_count == 2);

	char buffer[200];
	char text[] = "text which will be written to file";

	size_t bytes = 0;
	result = vfs->write_to_file(file1, &bytes, text, 34);
	assert(bytes == 34);
	assert(result == FS::ERR_SUCCESS);

	result = vfs->read_file(file2, &bytes, buffer, 200);
	assert(bytes == 34);
	assert(result == FS::ERR_SUCCESS);
	assert(strncmp(buffer, text, 34) == 0);

	result = vfs->close_file(&file1);
	assert(result == FS::ERR_SUCCESS);
	assert(file1 == NULL);

	result = vfs->remove_file(&file2);
	assert(result == FS::ERR_SUCCESS);
	assert(file2 == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_dir_twice()
{
	std::cout << "opening same directory twice" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/directory");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory");
	assert(result == FS::ERR_SUCCESS);

	assert(dir1 != NULL);
	assert(dir1->get_dentry() != NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);
	assert(dir1->get_dentry() == dir2->get_dentry());
	assert(dir1->get_dentry()->d_count == 2);

	result = vfs->close_file(&dir1);
	assert(result == FS::ERR_SUCCESS);
	assert(dir1 == NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);

	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == FS::ERR_SUCCESS);
	assert(dir2 == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_twice_open_file()
{
	std::cout << "test remove twice open file" << std::endl;

	FileHandler *file1 = NULL;
	int result = vfs->create_file(&file1, "C:/file.txt");
	assert(result == FS::ERR_SUCCESS);

	FileHandler *file2 = NULL;
	result = vfs->open_object(&file2, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(result == FS::ERR_SUCCESS);

	assert(file1 != NULL);
	assert(file1->get_dentry() != NULL);
	assert(file2 != NULL);
	assert(file2->get_dentry() != NULL);
	assert(file1->get_dentry() == file2->get_dentry());
	assert(file1->get_dentry()->d_count == 2);

	result = vfs->remove_file(&file1);
	assert(result == FS::ERR_FILE_OPEN_BY_OTHER);
	assert(file1 != NULL);

	result = vfs->remove_file(&file2);
	assert(result == FS::ERR_FILE_OPEN_BY_OTHER);
	assert(file2 != NULL);

	result = vfs->close_file(&file2);
	assert(result == FS::ERR_SUCCESS);
	assert(file2 == NULL);

	result = vfs->remove_file(&file1);
	assert(result == FS::ERR_SUCCESS);
	assert(file1 == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_twice_open_dir()
{
	std::cout << "test for remove twice open dir" << std::endl;

	FileHandler *dir1 = NULL;
	int result = vfs->create_dir(&dir1, "C:/directory");
	assert(result == FS::ERR_SUCCESS);
	
	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory");
	assert(result == FS::ERR_SUCCESS);

	assert(dir1 != NULL);
	assert(dir1->get_dentry() != NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);
	assert(dir1->get_dentry() == dir2->get_dentry());
	assert(dir1->get_dentry()->d_count == 2);

	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == FS::ERR_FILE_OPEN_BY_OTHER);
	assert(dir1 != NULL);
	assert(dir1->get_dentry() != NULL);

	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == FS::ERR_FILE_OPEN_BY_OTHER);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);

	result = vfs->close_file(&dir1);
	assert(result == FS::ERR_SUCCESS);
	assert(dir1 == NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);

	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == FS::ERR_SUCCESS);
	assert(dir2 == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_over_open_file()
{
	std::cout << "test for create twice same file" << std::endl;

	FileHandler *file1 = NULL;
	FileHandler *file2 = NULL;

	int result = vfs->create_file(&file1, "C:/file.txt");
	assert(result == FS::ERR_SUCCESS);
	result = vfs->create_file(&file2, "C:/file.txt");
	assert(result == FS::ERR_FILE_OPEN_BY_OTHER);

	assert(file1 != NULL);
	assert(file1->get_dentry() != NULL);
	assert(file1->get_dentry()->d_count == 1);

	assert(file2 == NULL);

	result = vfs->remove_file(&file1);
	assert(result == FS::ERR_SUCCESS);
	assert(file1 == NULL);

	std::cout << "OK\n" << std::endl;
}
