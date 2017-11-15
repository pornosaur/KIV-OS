#include "io.h"
#include "kernel.h"
#include "Handles.h"
#include "Handler.h"
#include "PipeHandler.h"
#include "Pipe.h"

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

		case kiv_os::scCreate_Pipe: {
			create_pipe(regs);
			break;
		}

		case kiv_os::scSet_File_Position: {
			set_file_position(regs);
			break;
		}

		case kiv_os::scGet_File_Position: {
			get_file_position(regs);
			break;
		}
	}
}

void create_file(kiv_os::TRegisters &regs) {
	
	char *path = reinterpret_cast<char*>(regs.rdx.r);
	uint8_t open_always = (uint8_t)regs.rcx.r;
	uint8_t file_atributes = (uint8_t)regs.rdi.r;

	FileHandler * handler = NULL;
	uint16_t ret_code = 0;
	
	if (file_atributes & kiv_os::faDirectory) { 
		// create/open directory
		ret_code = vfs->create_dir(&handler, std::string(path));
	}
	else {
		if (open_always) {
			// open file
			ret_code = vfs->open_object(&handler, std::string(path), FS::FS_OBJECT_FILE);
		}
		else {
			// crate file
			ret_code = vfs->create_file(&handler, std::string(path));
		}
	}

	if (ret_code) {
		set_error(regs, ret_code);
		return;
	}

	std::shared_ptr<Handler> shared_handler(handler);
	kiv_os::THandle t_handle = handles->add_handle(shared_handler);

	regs.rax.x = t_handle;

	//pro stdout/in/err neni potreba vytvaret soubor
	//TODO volani FS pro otevreni souboru
	//TODO catch errors
	
	/*HANDLE result = CreateFileA((char*)regs.rdx.r, GENERIC_READ | GENERIC_WRITE, (DWORD)regs.rcx.r, 0, OPEN_EXISTING, 0, 0);
	//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
	regs.flags.carry = result == INVALID_HANDLE_VALUE;
	if (!regs.flags.carry) regs.rax.x = Convert_Native_Handle(result);
	else regs.rax.r = GetLastError();
	*/
}

void write_file(kiv_os::TRegisters &regs) {
	size_t written;

	std::shared_ptr<Handler> cons = handles->get_handle_object(regs.rdx.x);
	cons->fseek(0, kiv_os::fsBeginning);//TODO offset(0) zmenit na promenou
	bool result = cons->write(reinterpret_cast<char*>(regs.rdi.r), (size_t)regs.rcx.r, written); //TODO offset(0) zmenit na promenou
	
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
	size_t read;
	//Unlock_Kernel();	//TODO: Can I allow to interruption while reading a file?
	std::shared_ptr<Handler> cons = handles->get_handle_object(regs.rdx.x);
	cons->fseek(0, kiv_os::fsBeginning); //TODO offset(0) zmenit na promenou, nastavovat offset pro konzoli?
	bool result = cons->read(reinterpret_cast<char*>(regs.rdi.r), (size_t)regs.rcx.r, read); //TODO offset(0) zmenit na promenou, nastavovat offset pro konzoli?
	regs.rax.r = read;
	regs.flags.carry = !result; //TODO Erro like this?
		//TODO Cteni z fs

		/*HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
		regs.flags.carry = hnd == INVALID_HANDLE_VALUE;
		if (!regs.flags.carry) regs.flags.carry = !ReadFile(hnd, reinterpret_cast<void*>(regs.rdi.r), (DWORD)regs.rcx.r, &read, NULL);
		if (!regs.flags.carry) regs.rax.r = read;
		else regs.rax.r = GetLastError();*/
	

}

void close_handle(kiv_os::TRegisters &regs) { //TODO close pro konzoli? 
	std::shared_ptr<Handler> cons = handles->get_handle_object(regs.rdx.x);
	regs.flags.carry = !cons;
	if (!regs.flags.carry && cons->close_handler()) {
		handles->Remove_Handle(regs.rdx.x);
		assert(cons.get());
		cons.reset();	/* Free a handler */
	}
	else {
		//TODO: Last Error here !!
	}

	//HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
	//regs.flags.carry = !CloseHandle(hnd);
	//if (!regs.flags.carry) Remove_Handle(regs.rdx.x);
	//else regs.rax.r = GetLastError();
}

void create_pipe(kiv_os::TRegisters &regs)
{
	//TODO debug auto
	//TODO return errors
	kiv_os::THandle* pipe_handles = reinterpret_cast<kiv_os::THandle*>(regs.rdx.r);

	Pipe *pipe = new Pipe();

	std::shared_ptr<PipeHandler> handle_write = std::make_shared<PipeHandler>(pipe, PipeHandler::fmOpen_Write);
	std::shared_ptr<PipeHandler> handle_read = std::make_shared<PipeHandler>(pipe, PipeHandler::fmOpen_Read);
	
	*pipe_handles = handles->add_handle(handle_write);
	*(pipe_handles + 1) = handles->add_handle(handle_read);
}

void get_file_position(kiv_os::TRegisters &regs)
{	
	std::shared_ptr<Handler> handler = handles->get_handle_object(regs.rdx.x);
	if (handler) {
		set_error(regs, kiv_os::erInvalid_Handle);
		return;
	}

	regs.rax.r = handler->ftell();
}

void set_file_position(kiv_os::TRegisters &regs)
{
	
	std::shared_ptr<Handler> handler = handles->get_handle_object(regs.rdx.x);
	if (handler) {
		set_error(regs, kiv_os::erInvalid_Handle);
		return;
	}

	long offset = (long)regs.rdi.r;
	uint8_t origin = regs.rcx.l;

	uint16_t ret_code = handler->fseek(offset, origin);
	if (ret_code) {
		set_error(regs, ret_code);
		return; // not success
	}

	if (regs.rcx.h) {
		// TODO SET FILE SIZE TO POSITION
	}
}


void set_error(kiv_os::TRegisters &regs, uint16_t code) {
	regs.rax.r = code;
	regs.flags.carry = 1;
	return;
}