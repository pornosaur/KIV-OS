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
	
	Vfs::file *file = vfs->create_file("file.text");
	assert(file != NULL);
	int result = vfs->close_file(&file);

	file = vfs->open_object("file.text", Vfs::VFS_OBJECT_FILE);
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_file_type == Vfs::VFS_OBJECT_FILE);
	assert(file->f_dentry->d_count == 1);

	std::cout << "removing file" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_file()
{
	std::cout << "opennig not existing file" << std::endl;

	Vfs::file *file = vfs->open_object("not.txt", Vfs::VFS_OBJECT_FILE);

	assert(file == NULL);
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_file_with_too_long_name()
{
	std::cout << "opennig not existing file with long name of file" << std::endl;

	Vfs::file *file = vfs->open_object("not_existing_file_with_very_very_long_name.txt", Vfs::VFS_OBJECT_FILE);

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
	file = vfs->open_object("new.txt", Vfs::VFS_OBJECT_FILE);
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

void Test_vfs::create_new_file_not_in_root()
{
	std::cout << "creating file with name new.txt in directory dir" << std::endl;

	std::cout << "creating dir" << std::endl;
	Vfs::file *dir = vfs->create_dir("dir");
	assert(dir != NULL);

	std::cout << "creating file" << std::endl;
	Vfs::file *file = vfs->create_file("dir/new.txt");

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
	file = vfs->open_object("dir/new.txt", Vfs::VFS_OBJECT_FILE);
	assert(file != NULL);
	assert(file->f_dentry != NULL);
	assert(file->f_dentry->d_name == "new.txt");
	assert(file->f_dentry->d_mounted != 1);
	assert(file->f_dentry->d_size == 1);
	assert(file->f_dentry->d_blocks == 1);

	std::cout << "removing file adn folder" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);
	result = vfs->remove_emtpy_dir(&dir);
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
	file = vfs->open_object("filname.txt", Vfs::VFS_OBJECT_FILE);
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
	file = vfs->open_object("file.txt", Vfs::VFS_OBJECT_FILE);
	assert(file != NULL);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);

	std::cout << "opening file.txt" << std::endl;
	file = vfs->open_object("file.txt", Vfs::VFS_OBJECT_FILE);
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
	file = vfs->open_object("file.txt", Vfs::VFS_OBJECT_FILE);
	assert(file != NULL);
	assert(file->f_dentry->d_size == buff_size);

	std::cout << "removing file.txt from FAT" << std::endl;
	result = vfs->remove_file(&file);
	assert(result == 0);

	std::cout << "opening file.txt" << std::endl;
	file = vfs->open_object("file.txt", Vfs::VFS_OBJECT_FILE);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_remove_not_existing_file()
{
	std::cout << "removing not existing file(dentry == NULL)" << std::endl;

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

	dir = vfs->open_object("directory", Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir == NULL);

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

	Vfs::file *file1 = vfs->create_file("file1");
	assert(file1 != NULL);
	Vfs::file *file2 = vfs->create_file("file2");
	assert(file2 != NULL);
	Vfs::file *file3 = vfs->create_file("file3");
	assert(file3 != NULL);
	Vfs::file *file4 = vfs->create_file("file4");
	assert(file4 != NULL);

	std::cout << "createing another dir with name directory2" << std::endl;
	Vfs::file *dir2 = vfs->create_dir("directory2");
	assert(dir2 == NULL);

	std::cout << "removing direcotry1" << std::endl;
	int result = vfs->remove_emtpy_dir(&dir1);
	assert(result == 0);
	result = vfs->remove_file(&file1);
	assert(result == 0);
	result = vfs->remove_file(&file2);
	assert(result == 0);
	result = vfs->remove_file(&file3);
	assert(result == 0);
	result = vfs->remove_file(&file4);
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

	int result = vfs->close_file(&dir1);
	assert(result == 0);
	result = vfs->close_file(&dir2);
	assert(result == 0);

	dir2 = vfs->open_object("directory/directory", Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir2 != NULL);
	assert(dir2->f_dentry->d_parent != NULL);
	dir1 = vfs->open_object("directory", Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir1 != NULL);
	assert(dir1->f_dentry->d_subdirectories != NULL);

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

void Test_vfs::remove_dir()
{
	std::cout << "test for remove dir in root with name directory" << std::endl;

	Vfs::file *dir = vfs->create_dir("directory");
	assert(dir != NULL);
	assert(dir->f_dentry != NULL);
	assert(dir->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	int result = vfs->remove_emtpy_dir(&dir);
	assert(result == 0);
	assert(dir == NULL);

	dir = vfs->open_object("directory", Vfs::VFS_OBJECT_DIRECTORY);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_not_empty_dir()
{
	std::cout << "test for remove not empty dir" << std::endl;

	Vfs::file *dir1 = vfs->create_dir("directory");
	assert(dir1 != NULL);
	assert(dir1->f_dentry != NULL);

	Vfs::file *dir2 = vfs->create_dir("directory/directory");
	assert(dir2 != NULL);
	assert(dir2->f_dentry != NULL);

	std::cout << "removing direcotry" << std::endl;
	int result = vfs->remove_emtpy_dir(&dir1);
	assert(result == -1);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir2);
	assert(result == 0);
	
	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir1);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::try_delete_not_exist_dir()
{
	std::cout << "removing not existing dir(dentry == NULL)" << std::endl;

	Vfs::file *dir = new Vfs::file();
	dir->f_dentry = NULL;
	dir->f_count = 1;
	dir->position = 2342;

	int result = vfs->remove_emtpy_dir(&dir);
	assert(result == -1);

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

	Vfs::file *dir = vfs->create_dir("directory");
	assert(dir != NULL);
	assert(dir->f_dentry != NULL);
	assert(dir->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	int result = vfs->close_file(&dir);
	assert(result == 0);

	dir = vfs->open_object("directory", Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir != NULL);
	assert(dir->f_dentry != NULL);
	assert(dir->f_dentry->d_file_type == Vfs::VFS_OBJECT_DIRECTORY);

	std::cout << "removing direcotry" << std::endl;
	result = vfs->remove_emtpy_dir(&dir);
	assert(result == 0);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_not_exist_dir()
{
	std::cout << "testing open not existing dir" << std::endl;

	Vfs::file *dir = vfs->open_object("directory", Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::open_dir_with_long_name()
{
	std::cout << "testing open dir with long name" << std::endl;

	Vfs::file *dir = vfs->open_object("directory_with_very_very_long_name", Vfs::VFS_OBJECT_DIRECTORY);
	assert(dir == NULL);

	std::cout << "OK\n" << std::endl;
}

void Test_vfs::write_read_file()
{
	std::cout << "reading and writing to file" << std::endl;
	
	Vfs::file *file = vfs->create_file("sext.txt");
	assert(file != NULL);
	
	int buff_size = 100;
	char buffer[100];
	int count = 0;

	count = vfs->read_file(file, buffer, buff_size);
	assert(count == 1);

	char text[] = "small textsmall textsmall textsmall textsmall text";
	count = vfs->write_to_file(file, text, 10);
	assert(count == 10);

	count = vfs->read_file(file, buffer, buff_size);
	assert(count == 10);
	assert(strncmp(buffer, text, 10) == 0);

	int result = vfs->remove_file(&file);
	assert(result == 0);
	assert(file == NULL);
	
	std::cout << "OK\n" << std::endl;
}

void Test_vfs::read_file_from_exact_position()
{
	std::cout << "reading file from exact position" << std::endl;

	Vfs::file *file = vfs->create_file("text.txt");
	assert(file != NULL);

	int buff_size = 100;
	char buffer[100];
	int count = 0;

	std::cout << "writing text to file" << std::endl;
	char *text = "text which contains exactly 36 words";
	count = vfs->write_to_file(file, text, 36);
	assert(count == 36);

	
	file->position = 10;	
	count = vfs->read_file(file, buffer, buff_size);
	assert(count == 26);
	assert(strncmp(buffer, text + 10, 26) == 0);

	int result = vfs->remove_file(&file);
	assert(result == 0);
	assert(file == NULL);

	std::cout << "OK\n" << std::endl;
}
