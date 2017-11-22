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
	size_t memory_size = 4096;
	fat_memory = new char[memory_size];

	// intialization FAT
	FatFS::init_fat_disk(fat_memory, memory_size, 128u);
	FS *fs = new FatFS(fat_memory, memory_size, "C:");
	
	// register FAT in VFS
	vfs = new Vfs();
	vfs->register_fs("C:", fs);

	//registr ProcFileSystem
	proc_filesystem = new ProcFilesystem();
	
	vfs->register_fs("0:", proc_filesystem);
	processManager = new ProcessManager(proc_filesystem);
	// TODO REMOVE ONLY TEST FILE ==================================================================================
	FileHandler *file = NULL;
	vfs->create_file(&file, "C:\\joke.txt");
	size_t writen = 0;
	file->write("Teacher: \"Kids, what does the chicken give you ? \"\n Student : \"Meat!\"\n Teacher : \"Very good! Now what does the pig give you?\"\n Student : \"Bacon!\"\n Teacher : \"Great! And what does the fat cow give you?\"\n Student : \"Homework!\"", 238, writen);
	delete file;
	file = NULL;
	// =============================================================================================================
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

	//spustime shell - v realnem OS bychom ovsem spousteli login
	//kiv_os::TEntry_Point shell = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, "shell");
	//if (shell) {
		
		kiv_os::TRegisters regs{ 0 };
		kiv_os::TProcess_Startup_Info tsi;
		tsi.arg = "shell"; //argumenty
		tsi.stdin = kiv_os::stdInput; //nastaveni std - jiz presmerovanych
		tsi.stdout = kiv_os::stdOutput;
		tsi.stderr = kiv_os::stdError;
		kiv_os::THandle proc_handles[1];
		
		processManager->create_process("shell", &tsi, regs);
		proc_handles[0] = static_cast<kiv_os::THandle>(regs.rax.r);
		processManager->wait_for(proc_handles, 1);

	//}

	Shutdown_Kernel();
}