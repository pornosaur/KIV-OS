#include "io.h"
#include "kernel.h"
#include "Handles.h"
#include "IHandleObject.h"
#include <memory>



void HandleIO(kiv_os::TRegisters &regs) {

	//V ostre verzi pochopitelne do switche dejte volani funkci a ne primo vykonny kod
	
	switch (regs.rax.l) {
		case kiv_os::scCreate_File: {
			create_file(regs);
			break;	//scCreateFile
		}

		case kiv_os::scWrite_File: {
			write_file(regs);
			break; //scWriteFile
		}

		case kiv_os::scClose_Handle: {
			close_handle(regs);
			break;	//CloseFile
		}
			
		case kiv_os::scRead_File: {
			read_file(regs);			
			break;
		}
	}
}

void create_file(kiv_os::TRegisters &regs) {
	//pro stdout/in/err neni potreba vytvaret soubor
	//TODO volani FS pro otevreni souboru
	
	/*HANDLE result = CreateFileA((char*)regs.rdx.r, GENERIC_READ | GENERIC_WRITE, (DWORD)regs.rcx.r, 0, OPEN_EXISTING, 0, 0);
	//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
	regs.flags.carry = result == INVALID_HANDLE_VALUE;
	if (!regs.flags.carry) regs.rax.x = Convert_Native_Handle(result);
	else regs.rax.r = GetLastError();
	*/
}

void write_file(kiv_os::TRegisters &regs) {
	DWORD written;

	std::shared_ptr<IHandleObject> cons = handles->get_handle_object(regs.rdx.x);
	written = cons->write(reinterpret_cast<char*>(regs.rdi.r), 0, (size_t)regs.rcx.r); //TODO offset(0) zmenit na promenou
	regs.rax.r = written;

		/*
		HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
		regs.flags.carry = hnd == INVALID_HANDLE_VALUE;
		if (!regs.flags.carry) regs.flags.carry = !WriteFile(hnd, reinterpret_cast<void*>(regs.rdi.r), (DWORD)regs.rcx.r, &written, NULL);
		if (!regs.flags.carry) regs.rax.r = written;
		else regs.rax.r = GetLastError();
		*/
	
	
}

void read_file(kiv_os::TRegisters &regs) {
	DWORD read;
	//Unlock_Kernel();	//TODO: Can I allow to interruption while reading a file?
	std::shared_ptr<IHandleObject> cons = handles->get_handle_object(regs.rdx.x);
	read = cons->read(reinterpret_cast<char*>(regs.rdi.r), (size_t)0, (size_t)regs.rcx.r); //TODO offset(0) zmenit na promenou, nastavovat offset pro konzoli?
	regs.rax.r = read;
		//TODO Cteni z fs

		/*HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
		regs.flags.carry = hnd == INVALID_HANDLE_VALUE;
		if (!regs.flags.carry) regs.flags.carry = !ReadFile(hnd, reinterpret_cast<void*>(regs.rdi.r), (DWORD)regs.rcx.r, &read, NULL);
		if (!regs.flags.carry) regs.rax.r = read;
		else regs.rax.r = GetLastError();*/
	

}

void close_handle(kiv_os::TRegisters &regs) { //TODO close pro konzoli? 
	//HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
	//regs.flags.carry = !CloseHandle(hnd);
	//if (!regs.flags.carry) Remove_Handle(regs.rdx.x);
	//else regs.rax.r = GetLastError();
}