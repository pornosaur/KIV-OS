#pragma once

#include <iostream>
#include <cassert>

#include "VfsFat.h"

class Test_vfs
{
	public:
		Test_vfs();
		~Test_vfs();
	private:
		VfsFat *vfs;

		void init();
		void term();

		// OPEN FILE
		void open_existing_file();
		void open_not_exist_file();
		void open_file_with_too_long_name();

		// CREATE FILE
		void create_new_file();
		void create_new_file_with_long_name();
		void create_new_file_with_bad_path();
		void create_existing_file();
		void create_file_with_max_name();

		// REMOVE FILE
		void remomve_file();
		void remove_file_bigger_than_cluster();
		void try_remove_not_existing_file();

		// CREATE DIR
		void create_dir_in_root();
		void create_dir_with_existing_name();
		void create_dir_in_full_directory();
		void create_dir_not_in_root();
		void create_dir_with_too_long_name();
		void create_dir_in_not_exist_path();


};