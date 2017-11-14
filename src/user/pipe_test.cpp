#include "pipe_test.h"
#include <chrono>
#include <iostream>
#include <string.h>
#include <memory>
#include <string>
#include <iostream>

std::mutex m;

void pipe_test() 
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	print_smt("****PIPE_TEST_WITH_DELAY_READER****");
	kiv_os::THandle pipe_handles[2];
	assert(pipe_handles);
	bool result = kiv_os_rtl::Create_Pipe(pipe_handles);
	assert(result);


	std::thread w = std::thread(pipe1, pipe_handles[0]);
	std::thread r = std::thread(pipe2, pipe_handles[1]);

	r.join();
	w.join();

	print_smt("***********************************");
}


void pipe1(const kiv_os::THandle& writer)
{
	print_smt("CREATED WRITER");

	const char *write = "Tohle je testovaci string, jeslti se zapis a cteni v cajku?!";
	//std::this_thread::sleep_for(std::chrono::seconds(3));
	
	size_t written;
	bool res = kiv_os_rtl::Write_File(writer, write, strlen(write), written);
	std::string s1 = "\tWRITTEN -> " + std::string(write) /*+ "[" + std::to_string(written) + "]"*/;
	print_smt(s1);

	assert(res);

	kiv_os_rtl::Close_File(writer);
	print_smt("WRITER CLOSED HANDLER");
}

void pipe2(const kiv_os::THandle& reader)
{
	print_smt("CREATED READER");
	size_t size_read = 10;
	char *read = new char[size_read];

	size_t read_r;
	int i = 1;

	bool res = false;
	std::string res_str = "";

	do {
		res = kiv_os_rtl::Read_File(reader, read, size_read, read_r);
		if (res && read_r > 0) {
			res_str += std::string(read);
			i++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	} while (res);

	if (!res && read_r == 0) {
		print_smt("\tREADER - EOF");
		kiv_os_rtl::Close_File(reader);
		{
			std::lock_guard<std::mutex> l(m);
			std::cout << "\tREAD -> " << res_str << std::endl;
		}
		print_smt("READER CLOSED HANDLER");
	}

	delete[] read;		/* Here coulde be problem in debug because DLL lib has diferent heap than the main program.*/
}

void print_smt(const std::string& str)
{
	std::lock_guard<std::mutex> lock(m);
	std::cout << str << std::endl;
}