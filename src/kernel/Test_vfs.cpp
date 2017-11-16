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
	test_more_subfiles();
	write_to_twice_open_file();
	open_dir_twice();
	remove_twice_open_file();
	remove_twice_open_dir();
	create_file_over_open_file();
	set_size_for_empty_file();
	set_size_in_one_cluster();
	set_size_more_than_one_cluster();
	set_size_more_than_file_size();
	read_empty_dir();
	read_full_dir();
	read_half_full_dir();
	read_dir_with_small_buffer();


	test_close_all_dentry_memory_leak(); // At the end. Left data in memory

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
	uint16_t result = vfs->create_file(&file, "C:/file.text");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->open_object(&file, "C:/file.text", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_count() == 1);
	delete file;

	std::cout << "removing file" << std::endl;
	result = vfs->remove_file("C:/file.text");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_file()
{
	std::cout << "opennig not existing file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->open_object(&file, "C:/not.txt", FS::FS_OBJECT_FILE);

	assert(file == NULL);
	assert(result == kiv_os::erFile_Not_Found);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_file_with_too_long_name()
{
	std::cout << "opennig not existing file with long name of file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->open_object(&file, "C:/not_existing_file_with_very_very_long_name.txt", FS::FS_OBJECT_FILE);

	assert(file == NULL);
	assert(result == kiv_os::erFile_Not_Found);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file()
{
	std::cout << "creating valid file with name new.txt" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/new.txt");

	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "closing new.txt" << std::endl;
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);
	

	std::cout << "opening new.txt" << std::endl;
	result = vfs->open_object(&file, "C:/new.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file("C:/new.txt");
	assert(result == kiv_os::erSuccess);
	

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_not_in_root()
{
	std::cout << "creating file with name new.txt in directory dir" << std::endl;

	std::cout << "creating dir" << std::endl;
	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/dir");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "creating file" << std::endl;
	FileHandler *file = NULL;
	result = vfs->create_file(&file, "C:/dir/new.txt");

	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "closing new.txt" << std::endl;
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "opening new.txt" << std::endl;
	result = vfs->open_object(&file, "C:/dir/new.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_name == "new.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "removing file adn folder" << std::endl;
	result = vfs->remove_file("C:/dir/new.txt");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_with_long_name()
{
	std::cout << "creating file with long name" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/new_file_with_very_long_name.txt");
	assert(file == NULL);
	assert(result == kiv_os::erInvalid_Argument);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_new_file_with_bad_path()
{
	std::cout << "creating file with bad file path" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/folder/new.txt");
	assert(file == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_existing_file()
{
	std::cout << "creating file over exist file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/new.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	char buffer[] = "text of new file with name new.txt";
	int buff_size = 34;

	std::cout << "write data to file new.txt" << std::endl;
	size_t bytes = 0;
	result = file->write(buffer, buff_size, bytes);
	assert(file->get_dentry()->d_size > 1);
	assert(file->get_dentry()->d_blocks == 1);
	assert(result == kiv_os::erSuccess);
	assert(bytes == buff_size);

	std::cout << "closing file new.txt" << std::endl;
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "creating same file again" << std::endl;
	result = vfs->create_file(&file, "C:/new.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "removing new.txt from FAT" << std::endl;
	result = vfs->remove_file("C:/new.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_with_max_name()
{
	std::cout << "creating file with maximal name of file filename.txt" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/filename.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	assert(file->get_dentry()->d_count == 1);
	assert(file->get_dentry()->d_name == "filename.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	std::cout << "closing filename.txt" << std::endl;
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "opening filename.txt" << std::endl;
	result = vfs->open_object(&file, "C:/filename.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry() != NULL);
	assert(file->get_dentry()->d_name == "filename.txt");
	assert(file->get_dentry()->d_mounted != 1);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "removing filename.txt from FAT" << std::endl;
	result = vfs->remove_file("C:/filename.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remomve_file()
{
	std::cout << "test on removing file" << std::endl;
	std::cout << "creating file file.txt" << std::endl;
	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/file.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	std::cout << "closing file.txt" << std::endl;
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file("C:/file.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_file_bigger_than_cluster()
{
	std::cout << "test on removing file which is bigger than one cluster" << std::endl;
	std::cout << "creating file file.txt" << std::endl;
	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/file.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	char * buffer = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
	int buff_size = 208;

	size_t bytes = 0;
	result = file->write(buffer, buff_size, bytes);
	assert(result == kiv_os::erSuccess);
	assert(bytes == buff_size);

	std::cout << "closing file.txt" << std::endl;
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == buff_size);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file("C:/file.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "opening file.txt" << std::endl;
	result = vfs->open_object(&file, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(file == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_remove_not_existing_file()
{
	std::cout << "removing not existing file(dentry == NULL)" << std::endl;

	uint16_t result = vfs->remove_file("C:/file");
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_root()
{
	std::cout << "createing normal dir in root with name directory" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_existing_name()
{
	std::cout << "createing normal dir with name directory" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	std::cout << "closing direcotry" << std::endl;
	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "createing dir with same name (directory)" << std::endl;
	result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_full_directory()
{
	std::cout << "createing dir with name directory1" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/directory1");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);


	FileHandler *file = NULL;
	result = vfs->create_file(&file, "C:/file1");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->create_file(&file, "C:/file2");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->create_file(&file, "C:/file3");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->create_file(&file, "C:/file4");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	std::cout << "createing another dir with name directory2" << std::endl;
	result = vfs->create_dir(&dir, "C:/directory2");
	assert(dir == NULL);
	assert(result == kiv_os::erNo_Left_Space);

	std::cout << "removing dir and files" << std::endl;
	
	result = vfs->remove_emtpy_dir("C:/directory1");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_file("C:/file1");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_file("C:/file2");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_file("C:/file3");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_file("C:/file4");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_not_in_root()
{
	std::cout << "createing normal dir in root with name directory" << std::endl;

	FileHandler *dir1 = NULL;
	uint16_t result = vfs->create_dir(&dir1, "C:/directory");
	assert(dir1 != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir1->get_dentry() != NULL);
	assert(dir1->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	assert(dir1->get_dentry()->d_subdirectories == NULL);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory/directory");
	assert(dir2 != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir2->get_dentry() != NULL);
	assert(dir2->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	assert(dir2->get_dentry()->d_parent != NULL);

	assert(dir1->get_dentry()->d_subdirectories != NULL);

	if (dir1->close_handler()) {
		delete dir1;
		dir1 = NULL;
	}
	assert(dir1 == NULL);
	
	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	result = vfs->open_object(&dir2, "C:/directory/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir2 != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir2->get_dentry()->d_parent != NULL);
	result = vfs->open_object(&dir1, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir1 != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir1->get_dentry()->d_subdirectories != NULL);

	if (dir1->close_handler()) {
		delete dir1;
		dir1 = NULL;
	}
	assert(dir1 == NULL);

	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	std::cout << "removing dir2" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory/directory");
	assert(result == 0);

	std::cout << "removing dir1" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_too_long_name()
{
	std::cout << "createing dir with too long name" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/very_very_long_name_for_simple_direcotry");
	assert(dir == NULL);
	assert(result == kiv_os::erInvalid_Argument);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_in_not_exist_path()
{
	std::cout << "createing dir with too long name" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/direcotry/direcotry/dir");
	assert(dir == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_dir()
{
	std::cout << "test for remove dir in root with name directory" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);
	
	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_not_empty_dir()
{
	std::cout << "test for remove not empty dir" << std::endl;

	FileHandler *dir1 = NULL;
	uint16_t result = vfs->create_dir(&dir1, "C:/directory");
	assert(dir1 != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir1->get_dentry() != NULL);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory/directory");
	assert(dir2 != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir2->get_dentry() != NULL);

	if (dir1->close_handler()) {
		delete dir1;
		dir1 = NULL;
	}
	assert(dir1 == NULL);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erPermission_Denied);

	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erDir_Not_Empty);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory/directory");
	assert(result == kiv_os::erSuccess);
	
	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_not_exist_dir()
{
	std::cout << "removing not existing dir" << std::endl;

	uint16_t result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_root()
{
	std::cout << "removing root " << std::endl;

	uint16_t result = vfs->remove_emtpy_dir("C:");
	assert(result == kiv_os::erPermission_Denied);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_normal_dir()
{
	std::cout << "testing dir open" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/directory");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry() != NULL);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_dir()
{
	std::cout << "testing open not existing dir" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_dir_with_long_name()
{
	std::cout << "testing open dir with long name" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->open_object(&dir, "C:/directory_with_very_very_long_name", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_file()
{
	std::cout << "reading and writing to file" << std::endl;
	
	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	
	int buff_size = 100;
	char buffer[100];

	size_t bytes = 0;
	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 1);
	assert(result == kiv_os::erSuccess);

	char text[] = "small text";
	result = file->write(text, 10, bytes);
	assert(bytes == 10);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(result == kiv_os::erSuccess);
	assert(bytes == 10);
	assert(strncmp(buffer, text, 10) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);;
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_file_from_exact_position()
{
	std::cout << "reading file from exact position" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 100;
	char buffer[100];
	size_t bytes = 0;

	std::cout << "writing text to file" << std::endl;
	char *text = "text which contains exactly 36 words";
	result = file->write(text, 36, bytes);
	assert(result == kiv_os::erSuccess);
	assert(bytes == 36);

	
	file->fseek(10, kiv_os::fsBeginning, 0);
	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 26);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text + 10, 26) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_file_bigger_than_one_cluster()
{
	std::cout << "reading and writing to file bigger than one cluster" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];
	size_t bytes = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = file->write(text, 200, bytes);
	assert(bytes == 200);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 200);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 200) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_exactly_one_cluster()
{
	std::cout << "reading and writing exactly one cluster" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];
	size_t bytes = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor.";
	result = file->write(text, 128, bytes);
	assert(bytes == 128);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 128);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 128) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_small_piece_of_file()
{
	std::cout << "reading only piece of file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 30;
	char buffer[30];
	size_t bytes = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = file->write(text, 200, bytes);
	assert(bytes == 200);
	assert(result == kiv_os::erSuccess);

	file->fseek(120, kiv_os::fsBeginning, 0);
	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 30);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text + 120, 30) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_all_file_by_pieces()
{
	std::cout << "reading all file by pieces" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 31;
	char buffer[31];
	size_t bytes = 0;
	int shuld_read = 0;

	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = file->write(text, 200, bytes);
	assert(bytes == 200);
	assert(result == kiv_os::erSuccess);


	shuld_read = 31;
	for (int i = 0; i < 200; i += 31) {
		file->fseek(i, kiv_os::fsBeginning, 0);
		if (200 - i < shuld_read) {
			shuld_read = 200 - i;
		}

		result = file->read(buffer, buff_size, bytes);
		assert(bytes == shuld_read);
		assert(result == kiv_os::erSuccess);
		assert(strncmp(buffer, text + i, shuld_read) == 0);
	}

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::rewrite_file()
{
	std::cout << "rewriting file" << std::endl;

	std::cout << "creating empty file" << std::endl;
	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];
	size_t bytes = 0;

	std::cout << "writing short text" << std::endl;
	char text1[] = "small text";
	result = file->write(text1, 10, bytes);
	assert(bytes == 10);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 10);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text1, 10) == 0);


	std::cout << "writing long text" << std::endl;
	char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque condimentum diam et urna tincidunt vestibulum malesuada ac dolor. Phasellus vel neque vitae erat ultrices luctus fermentum vitae metus.";
	result = file->write(text, 200, bytes);
	assert(bytes == 200);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 200);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 200) == 0);

	std::cout << "writing short text" << std::endl;
	file->fseek(10, kiv_os::fsBeginning, 0);
	result = file->write(text1, 10, bytes);
	assert(bytes == 10);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 20);

	file->fseek(0, kiv_os::fsBeginning, 0);
	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 20);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 10) == 0);
	assert(strncmp(buffer + 10, text1, 10) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_zero_bytes_to_file()
{
	std::cout << "writing zero bytes to file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 30;
	char buffer[30];
	size_t bytes = 0;

	char text[] = "";
	result = file->write(text, 0, bytes);
	assert(bytes == 0);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 1);
	assert(result == kiv_os::erSuccess);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_with_space_in_name()
{
	std::cout << "creating file with space in name" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/my file.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(strcmp("my file.txt", file->get_dentry()->d_name.c_str()) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->open_object(&file, "C:/my file.txt", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	
	char text[] = "short text";
	size_t bytes = 0;
	result = file->write(text, 10, bytes);
	assert(bytes == 10);
	assert(result == kiv_os::erSuccess);

	char buffer[20];
	result = file->read(buffer, 20, bytes);
	assert(bytes == 10);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(text, buffer, 10) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/my file.txt");
	assert(result == kiv_os::erSuccess);

	result = vfs->open_object(&file, "C:/my file.txt", FS::FS_OBJECT_FILE);
	assert(file == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_dir_with_space_in_name()
{
	std::cout << "creating directory with space in name" << std::endl;

	FileHandler *dir = NULL;
	uint16_t result = vfs->create_dir(&dir, "C:/my dir");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(strcmp("my dir", dir->get_dentry()->d_name.c_str()) == 0);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	result = vfs->open_object(&dir, "C:/my dir", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	result = vfs->remove_emtpy_dir("C:/my dir");
	assert(result == kiv_os::erSuccess);

	result = vfs->open_object(&dir, "C:/my dir", FS::FS_OBJECT_DIRECTORY);
	assert(dir == NULL);
	assert(result == kiv_os::erFile_Not_Found);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::folder_dir_same_name()
{
	std::cout << "test for same name for folder and file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/name");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);
	
	FileHandler *dir = NULL;
	result = vfs->create_dir(&dir, "C:/name");
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	result = vfs->open_object(&dir, "C:/name", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);
	assert(dir->get_dentry()->d_file_type == FS::FS_OBJECT_DIRECTORY);

	result = vfs->open_object(&file, "C:/name", FS::FS_OBJECT_FILE);
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_file_type == FS::FS_OBJECT_FILE);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	result = vfs->remove_emtpy_dir("C:/name");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_file("C:/name");
	assert(result == kiv_os::erSuccess);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::test_more_subfiles()
{
	std::cout << "test with more subdirectories" << std::endl;

	FileHandler *dir1 = NULL;
	uint16_t result = vfs->create_dir(&dir1, "C:/dir1");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/dir2");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir3 = NULL;
	result = vfs->create_dir(&dir3, "C:/dir1/dir3");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir4 = NULL;
	result = vfs->create_dir(&dir4, "C:/dir2/dir4");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir5 = NULL;
	result = vfs->create_dir(&dir5, "C:/dir2/dir5");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir6 = NULL;
	result = vfs->create_dir(&dir6, "C:/dir2/dir6");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir7 = NULL;
	result = vfs->create_dir(&dir7, "C:/dir2/dir5/dir7");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir8 = NULL;
	result = vfs->create_dir(&dir8, "C:/dir2/dir5/dir7/dir8");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir9 = NULL;
	result = vfs->create_dir(&dir9, "C:/dir2/dir5/dir9");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir10 = NULL;
	result = vfs->create_dir(&dir10, "C:/dir10");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir11 = NULL;
	result = vfs->create_dir(&dir11, "C:/dir10/dir11");
	assert(result == kiv_os::erSuccess);

	if (dir1->close_handler()) {
		delete dir1;
		dir1 = NULL;
	}
	assert(dir1 == NULL);

	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	if (dir3->close_handler()) {
		delete dir3;
		dir3 = NULL;
	}
	assert(dir3 == NULL);

	if (dir4->close_handler()) {
		delete dir4;
		dir4 = NULL;
	}
	assert(dir4 == NULL);

	if (dir5->close_handler()) {
		delete dir5;
		dir5 = NULL;
	}
	assert(dir5 == NULL);

	if (dir6->close_handler()) {
		delete dir6;
		dir6 = NULL;
	}
	assert(dir6 == NULL);

	if (dir7->close_handler()) {
		delete dir7;
		dir7 = NULL;
	}
	assert(dir7 == NULL);

	if (dir8->close_handler()) {
		delete dir8;
		dir8 = NULL;
	}
	assert(dir8 == NULL);

	if (dir9->close_handler()) {
		delete dir9;
		dir9 = NULL;
	}
	assert(dir9 == NULL);

	if (dir10->close_handler()) {
		delete dir10;
		dir10 = NULL;
	}
	assert(dir10 == NULL);

	if (dir11->close_handler()) {
		delete dir11;
		dir11 = NULL;
	}
	assert(dir11 == NULL);

	result = vfs->remove_emtpy_dir("C:/dir2/dir5/dir7/dir8");
	assert(result == kiv_os::erSuccess);

	result = vfs->open_object(&dir2, "C:/dir2", FS::FS_OBJECT_DIRECTORY);
	assert(result == kiv_os::erSuccess);
	result = vfs->open_object(&dir10, "C:/dir10", FS::FS_OBJECT_DIRECTORY);
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir2/dir5/dir7");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir2/dir5/dir9");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir10/dir11");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir1/dir3");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir2/dir4");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir1");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir2/dir5");
	assert(result == kiv_os::erSuccess);

	result = vfs->remove_emtpy_dir("C:/dir2/dir6");
	assert(result == kiv_os::erSuccess);

	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	result = vfs->remove_emtpy_dir("C:/dir2");
	assert(result == kiv_os::erSuccess);

	if (dir10->close_handler()) {
		delete dir10;
		dir10 = NULL;
	}
	assert(dir10 == NULL);

	result = vfs->remove_emtpy_dir("C:/dir10");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::test_close_all_dentry_memory_leak()
{
	std::cout << "test for close all open dentry (memory leak) (with new solution new memory leak)" << std::endl;

	FileHandler *dir1 = NULL;
	uint16_t result = vfs->create_dir(&dir1, "C:/dir1");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/dir2");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir3 = NULL;
	result = vfs->create_dir(&dir3, "C:/dir1/dir3");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir4 = NULL;
	result = vfs->create_dir(&dir4, "C:/dir2/dir4");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir5 = NULL;
	result = vfs->create_dir(&dir5, "C:/dir2/dir5");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir6 = NULL;
	result = vfs->create_dir(&dir6, "C:/dir2/dir6");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir7 = NULL;
	result = vfs->create_dir(&dir7, "C:/dir2/dir5/dir7");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir8 = NULL;
	result = vfs->create_dir(&dir8, "C:/dir2/dir5/dir7/dir8");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir9 = NULL;
	result = vfs->create_dir(&dir9, "C:/dir2/dir5/dir9");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir10 = NULL;
	result = vfs->create_dir(&dir10, "C:/dir10");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir11 = NULL;
	result = vfs->create_dir(&dir11, "C:/dir10/dir11");
	assert(result == kiv_os::erSuccess);

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

	uint16_t result = vfs->create_file(&file1, "C:/file.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->open_object(&file2, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(result == kiv_os::erSuccess);

	assert(file1 != NULL);
	assert(file1->get_dentry() != NULL);
	assert(file2 != NULL);
	assert(file2->get_dentry() != NULL);
	assert(file1->get_dentry() == file2->get_dentry());
	assert(file1->get_dentry()->d_count == 2);

	char buffer[200];
	char text[] = "text which will be written to file";

	size_t bytes = 0;
	result = file1->write(text, 34, bytes);
	assert(bytes == 34);
	assert(result == kiv_os::erSuccess);

	result = file2->read(buffer, 200, bytes);
	assert(bytes == 34);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 34) == 0);

	if (file1->close_handler()) {
		delete file1;
		file1 = NULL;
	}
	assert(file1 == NULL);

	if (file2->close_handler()) {
		delete file2;
		file2 = NULL;
	}
	assert(file2 == NULL);

	result = vfs->remove_file("C:/file.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_dir_twice()
{
	std::cout << "opening same directory twice" << std::endl;

	FileHandler *dir1 = NULL;
	uint16_t result = vfs->create_dir(&dir1, "C:/directory");
	assert(result == kiv_os::erSuccess);

	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory");
	assert(result == kiv_os::erSuccess);

	assert(dir1 != NULL);
	assert(dir1->get_dentry() != NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);
	assert(dir1->get_dentry() == dir2->get_dentry());
	assert(dir1->get_dentry()->d_count == 2);

	if (dir1->close_handler()) {
		delete dir1;
		dir1 = NULL;
	}
	assert(dir1 == NULL);

	assert(dir1 == NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);

	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_twice_open_file()
{
	std::cout << "test remove twice open file" << std::endl;

	FileHandler *file1 = NULL;
	uint16_t result = vfs->create_file(&file1, "C:/file.txt");
	assert(result == kiv_os::erSuccess);

	FileHandler *file2 = NULL;
	result = vfs->open_object(&file2, "C:/file.txt", FS::FS_OBJECT_FILE);
	assert(result == kiv_os::erSuccess);

	assert(file1 != NULL);
	assert(file1->get_dentry() != NULL);
	assert(file2 != NULL);
	assert(file2->get_dentry() != NULL);
	assert(file1->get_dentry() == file2->get_dentry());
	assert(file1->get_dentry()->d_count == 2);

	result = vfs->remove_file("C:/file.txt");
	assert(result == kiv_os::erPermission_Denied);
	assert(file1 != NULL);
	assert(file2 != NULL);

	if (file1->close_handler()) {
		delete file1;
		file1 = NULL;
	}
	assert(file1 == NULL);

	if (file2->close_handler()) {
		delete file2;
		file2 = NULL;
	}
	assert(file2 == NULL);

	result = vfs->remove_file("C:/file.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::remove_twice_open_dir()
{
	std::cout << "test for remove twice open dir" << std::endl;

	FileHandler *dir1 = NULL;
	uint16_t result = vfs->create_dir(&dir1, "C:/directory");
	assert(result == kiv_os::erSuccess);
	
	FileHandler *dir2 = NULL;
	result = vfs->create_dir(&dir2, "C:/directory");
	assert(result == kiv_os::erSuccess);

	assert(dir1 != NULL);
	assert(dir1->get_dentry() != NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);
	assert(dir1->get_dentry() == dir2->get_dentry());
	assert(dir1->get_dentry()->d_count == 2);

	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erPermission_Denied);
	assert(dir1 != NULL);
	assert(dir1->get_dentry() != NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);

	if (dir1->close_handler()) {
		delete dir1;
		dir1 = NULL;
	}
	assert(dir1 == NULL);

	assert(dir1 == NULL);
	assert(dir2 != NULL);
	assert(dir2->get_dentry() != NULL);

	if (dir2->close_handler()) {
		delete dir2;
		dir2 = NULL;
	}
	assert(dir2 == NULL);

	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::create_file_over_open_file()
{
	std::cout << "test for create twice same file" << std::endl;

	FileHandler *file1 = NULL;
	FileHandler *file2 = NULL;

	uint16_t result = vfs->create_file(&file1, "C:/file.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->create_file(&file2, "C:/file.txt");
	assert(result == kiv_os::erPermission_Denied);

	assert(file1 != NULL);
	assert(file1->get_dentry() != NULL);
	assert(file1->get_dentry()->d_count == 1);

	assert(file2 == NULL);

	if (file1->close_handler()) {
		delete file1;
		file1 = NULL;
	}
	assert(file1 == NULL);

	result = vfs->remove_file("C:/file.txt");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::set_size_for_empty_file()
{
	std::cout << "seting size for empty file" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 1);

	result = file->fseek(0, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 1);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);;

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::set_size_in_one_cluster()
{
	std::cout << "setting size in one cluster" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	size_t bytes = 0;

	char text[] = "small text small text small text small text";
	result = file->write(text, 40, bytes);
	assert(bytes == 40);
	assert(result == kiv_os::erSuccess);

	assert(file->get_dentry()->d_size == 40);

	result = file->fseek(22, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 22);
	
	result = file->fseek(0, kiv_os::fsBeginning, 0);
	assert(result == kiv_os::erSuccess);

	int buff_size = 100;
	char buffer[100];
	
	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 22);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 22) == 0);

	result = file->fseek(0, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 1);
	assert(file->get_dentry()->d_blocks == 1);

	file->fseek(1, kiv_os::fsBeginning, 0);
	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 0);
	assert(result == kiv_os::erSuccess);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);;

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::set_size_more_than_one_cluster()
{
	std::cout << "setting size in more than one cluster" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	size_t bytes = 0;

	char text[] = "very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long text";
	result = file->write(text, 220, bytes);
	assert(bytes == 220);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_blocks == 2);
	assert(file->get_dentry()->d_size == 220);

	result = file->fseek(182, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 182);

	result = file->fseek(0, kiv_os::fsBeginning, 0);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 182);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 182) == 0);

	result = file->fseek(60, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 60);
	assert(file->get_dentry()->d_blocks == 1);

	result = file->fseek(0, kiv_os::fsBeginning, 0);
	assert(result == kiv_os::erSuccess);

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 60);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 60) == 0);

	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);;

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::set_size_more_than_file_size()
{
	std::cout << "setting size more than file size" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);

	size_t bytes = 0;

	char text[] = "small text small text small text small text";
	result = file->write(text, 40, bytes);
	assert(bytes == 40);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 40);

	result = file->fseek(70, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erInvalid_Argument);
	assert(file->get_dentry()->d_size == 40);

	result = file->fseek(0, kiv_os::fsBeginning, 0);
	assert(result == kiv_os::erSuccess);

	result = file->fseek(40, kiv_os::fsBeginning, 1);
	assert(result == kiv_os::erSuccess);
	assert(file->get_dentry()->d_size == 40);

	result = file->fseek(0, kiv_os::fsBeginning, 0);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];

	result = file->read(buffer, buff_size, bytes);
	assert(bytes == 40);
	assert(result == kiv_os::erSuccess);
	assert(strncmp(buffer, text, 40) == 0);


	if (file->close_handler()) {
		delete file;
		file = NULL;
	}
	assert(file == NULL);

	result = vfs->remove_file("C:/text.txt");
	assert(result == kiv_os::erSuccess);;

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_empty_dir()
{
	std::cout << "reading empty directory" << std::endl;

	FileHandler *dir= NULL;
	uint16_t result = vfs->open_object(&dir, "C:", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 300;
	char buffer[300];

	size_t bytes = 0;
	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == 0);
	assert(result == kiv_os::erSuccess);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}
	assert(dir == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_full_dir()
{
	std::cout << "reading full directory" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_file(&file, "C:/text1.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_file(&file, "C:/text2.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_dir(&file, "C:/directory1");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_file(&file, "C:/text3.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_dir(&file, "C:/directory2");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_dir(&file, "C:/directory3");
	assert(file == NULL);
	assert(result == kiv_os::erNo_Left_Space);


	FileHandler *dir = NULL;
	result = vfs->open_object(&dir, "C:", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];

	size_t bytes = 0;
	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == 5 * sizeof(kiv_os::TDir_Entry));
	assert(result == kiv_os::erSuccess);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}

	result = vfs->remove_file("C:/text1.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_file("C:/text2.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_file("C:/text3.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_emtpy_dir("C:/directory1");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_emtpy_dir("C:/directory2");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_half_full_dir()
{
	std::cout << "reading half full directory" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_dir(&file, "C:/directory");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_file(&file, "C:/directory/text1.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_file(&file, "C:/directory/text2.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	FileHandler *dir = NULL;
	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 200;
	char buffer[200];

	size_t bytes = 0;
	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == 2 * sizeof(kiv_os::TDir_Entry));
	assert(result == kiv_os::erSuccess);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}

	result = vfs->remove_file("C:/directory/text1.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_file("C:/directory/text2.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_dir_with_small_buffer()
{
	std::cout << "reading direcotry with small buffer" << std::endl;

	FileHandler *file = NULL;
	uint16_t result = vfs->create_dir(&file, "C:/directory");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_file(&file, "C:/directory/text1.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	result = vfs->create_file(&file, "C:/directory/text2.txt");
	assert(file != NULL);
	assert(result == kiv_os::erSuccess);
	if (file->close_handler()) {
		delete file;
		file = NULL;
	}

	FileHandler *dir = NULL;
	result = vfs->open_object(&dir, "C:/directory", FS::FS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(result == kiv_os::erSuccess);

	int buff_size = 10;
	char buffer[200];

	size_t bytes = 0;
	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == 0);
	assert(result == kiv_os::erSuccess);

	buff_size = 25;
	bytes = 0;
	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == sizeof(kiv_os::TDir_Entry));
	assert(result == kiv_os::erSuccess);

	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == sizeof(kiv_os::TDir_Entry));
	assert(result == kiv_os::erSuccess);

	result = dir->read(buffer, buff_size, bytes);
	assert(bytes == 0);
	assert(result == kiv_os::erSuccess);

	if (dir->close_handler()) {
		delete dir;
		dir = NULL;
	}

	result = vfs->remove_file("C:/directory/text1.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_file("C:/directory/text2.txt");
	assert(result == kiv_os::erSuccess);
	result = vfs->remove_emtpy_dir("C:/directory");
	assert(result == kiv_os::erSuccess);

	std::cout << "OK\n" << std::endl;
}
