#include "pipe_test.h"
#include <chrono>
#include <iostream>
#include <string.h>


void pipe_test() 
{
	kiv_os::THandle *pipe_handles = new kiv_os::THandle[2];
	assert(pipe_handles);
	bool result = kiv_os_rtl::Create_Pipe(pipe_handles);
	assert(result);


	std::thread w = std::thread(pipe1, pipe_handles[0]);
	std::thread r = std::thread(pipe2, pipe_handles[1]);

	r.join();
	w.join();

	std::cout << "END PIPE TEST" << std::endl;
}


void pipe1(const kiv_os::THandle& writer)
{
	std::cout << "CREATE WRITER" << std::endl;

	char *write = "Tak tohle chci zapsat", *write2 = "dsjkfsdlafjslkfd";
	std::cout << "\tWRITER - wait 3s" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	
	size_t written;
	bool res = kiv_os_rtl::Write_File(writer, write, strlen(write), written);
	std::cout << "\tWRITTEN - " << write << "[" << written << "]" << std::endl;

	assert(res);

	std::cout << "\tWRITER - wait 3s" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	res = kiv_os_rtl::Write_File(writer, write2, strlen(write2), written);
	std::cout << "\tWRITTEN - " << write2 << "[" << written << "]" << std::endl;

	kiv_os_rtl::Close_File(writer);
	std::cout << "WRITER CLOSE HANDLER" << std::endl;
}

void pipe2(const kiv_os::THandle& reader)
{
	std::cout << "CREATE READER" << std::endl;
	char *read = new char[400];
	size_t read_r;
	int i = 1;

	bool res = kiv_os_rtl::Read_File(reader, read, strlen(read), read_r);;
	std::cout << "\tREADING(" << i << "): " << read << std::endl;

	while (res) {
		i++;
		std::cout << "\tREADING(" << i << "): " << read << std::endl;
		res = kiv_os_rtl::Read_File(reader, read, strlen(read), read_r);
	}
	if (!res && read_r == 0) {
		std::cout << "\tREADING(" << i << "): EOF" << std::endl;
		kiv_os_rtl::Close_File(reader);
	}
}