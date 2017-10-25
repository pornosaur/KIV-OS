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

		void open_existing_file();
		void open_not_exist_file();
		void open_file_with_too_long_name();

		void create_new_file();
		void create_new_file_with_long_name();
		void create_new_file_with_bad_path();
		void create_existing_file();


};