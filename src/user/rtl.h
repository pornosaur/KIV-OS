#pragma once

#include "..\api\api.h"

namespace kiv_os_rtl {

	size_t Get_Last_Error();

	kiv_os::THandle Create_File(const char* file_name, size_t flags);
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
	bool Create_Process(const char *program_name, kiv_os::TProcess_Startup_Info *tso, kiv_os::THandle &process_handle);
	//Vytvoreni procesu podle nazvu programu program_name s prislunymi paramatery a handlery ulozenych v tso.
	bool Wait_For(const kiv_os::THandle *proc_handles, const size_t count);

}