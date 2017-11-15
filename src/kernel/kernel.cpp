#pragma once

#include "kernel.h"

ProcessManager *processManager;
std::shared_ptr<Handles> handles;
BinSemaphore interrupt_sem;
HMODULE User_Programs;
Vfs *vfs;
char *fat_memory;

void Set_Error(const bool failed, kiv_os::TRegisters &regs) {
	if (failed) {
		regs.flags.carry = true;
		regs.rax.r = GetLastError();
	}
	else
		regs.flags.carry = false;
}

void Lock_Kernel()
{
	interrupt_sem.P();
}

void Unlock_Kernel() 
{
	interrupt_sem.V();
}

void Initialize_Kernel() {
	User_Programs = LoadLibrary(L"user.dll");
	handles = std::make_shared<Handles>();
	processManager = new ProcessManager();
	handles->init_console_handles();
	
	// virtual memory initialization
	size_t memory_size = 4096;
	fat_memory = new char[memory_size];

	// intialization FAT
	FatFS::init_fat_disk(fat_memory, memory_size, 128u);
	FS *fs = new FatFS(fat_memory, memory_size, "C:");
	
	// register FAT in VFS
	vfs = new Vfs();
	vfs->register_fs("C:", fs);
}

void Shutdown_Kernel() {
	FreeLibrary(User_Programs);

	delete[] fat_memory;
	delete vfs;
}

void __stdcall Sys_Call(kiv_os::TRegisters &regs) 
{
	switch (regs.rax.h) {
		case kiv_os::scIO:
			HandleIO(regs);
			break;
		case kiv_os::scProc:
			processManager->handle_proc(regs);
			break;
	}	
}

void __stdcall Run_VM() {
	Initialize_Kernel();

	//spustime shell - v realnem OS bychom ovsem spousteli login
	//kiv_os::TEntry_Point shell = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, "shell");
	//if (shell) {
		
		kiv_os::TRegisters regs{ 0 };
		kiv_os::TProcess_Startup_Info tsi;
		tsi.arg = "shell"; //argumenty
		tsi.stdin_t = kiv_os::stdInput; //nastaveni std - jiz presmerovanych
		tsi.stdout_t = kiv_os::stdOutput;
		tsi.stderr_t = kiv_os::stdError;
		kiv_os::THandle proc_handles[1];
		
		processManager->create_process("shell", &tsi, regs);
		proc_handles[0] = static_cast<kiv_os::THandle>(regs.rax.r);
		processManager->wait_for(proc_handles, 1);

	//}

	Shutdown_Kernel();
}