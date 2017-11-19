#pragma once

#include "..\api\api.h"
#include <vector>

#define MAX_SIZE_BUFFER_IN	1024

namespace kiv_os_rtl {

	size_t Get_Last_Error();

	kiv_os::THandle Create_File(const char* file_name, size_t flags, uint8_t atributes);
	//podle flags otevre, vytvori soubor a vrati jeho deskriptor
	//vraci nenulovy handle, kdyz vse OK
	bool Write_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written);
	//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
	//vraci true, kdyz vse OK
	bool Close_File(const kiv_os::THandle file_handle);
	//uzavre soubor identifikovany pomoci deskriptoru
	//vraci true, kdyz vse OK
	bool Read_File(const kiv_os::THandle file_handle, void *buffer, const size_t buffer_size, size_t &read);
	//cte ze souboru do buffer o velikosti buffer_size a vraci pocet precenych dat ve read
	//vraci true, kdyz vse OK
	bool Remove_File(const char* file_name);
	// smaze soubor z disku
	// vraci true, kdyz vse OK
	bool Set_File_Position(const kiv_os::THandle file_handle, long &position, uint8_t origin, uint16_t set_size);
	bool Get_File_Position(const kiv_os::THandle file_handle, size_t &position);
	bool Set_Current_Directory(const char *path);
	bool Get_Current_Direcotry(const void *buffer, const size_t buffer_size, size_t &written);
	bool Create_Process(const char *program_name, kiv_os::TProcess_Startup_Info tsi, kiv_os::THandle &process_handle);
	bool Create_Thread(kiv_os::TThread_Proc ttp, void *data, kiv_os::THandle &process_handle);
	//Vytvoreni procesu podle nazvu programu program_name s prislunymi paramatery a handlery ulozenych v tso.
	bool Wait_For(std::vector<kiv_os::THandle> proc_hadles, const size_t count);

	//Vytvori pipe, kde: pipe_handles[0] == zapis; pipe_handles[1] == cteni
	bool Create_Pipe(kiv_os::THandle pipe_handles[2]);

}