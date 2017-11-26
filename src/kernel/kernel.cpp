#pragma once

#include "kernel.h"
#undef stdio
#undef stdout
#undef stderr
ProcessManager *processManager;
BinSemaphore interrupt_sem;
HMODULE User_Programs;
ProcFilesystem *proc_filesystem;
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
	
	
	// virtual memory initialization
	size_t memory_size = 65536;
	fat_memory = new char[memory_size];

	// intialization FAT
	FatFS::init_fat_disk(fat_memory, memory_size, 512u);
	FS *fs = new FatFS(fat_memory, memory_size, "C:");
	
	// register FAT in VFS
	vfs = new Vfs();
	vfs->register_fs("C:", fs);

	//registr ProcFileSystem
	proc_filesystem = new ProcFilesystem();
	
	vfs->register_fs("0:", proc_filesystem);
	processManager = new ProcessManager(proc_filesystem);
}

void Shutdown_Kernel() {
	FreeLibrary(User_Programs);

	delete[] fat_memory;
	delete processManager;
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

	processManager->init();
		
	Shutdown_Kernel();
}