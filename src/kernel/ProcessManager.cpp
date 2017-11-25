#include "ProcessManager.h"
#include "ProcFilesystem.h"
#include "kernel.h"
#include <algorithm>
#include <string>      
#include <iostream> 

ProcessManager::ProcessManager(ProcFilesystem *proc_filesys) {
	proc_filesystem = proc_filesys;
}
ProcessManager::~ProcessManager() {
}
void ProcessManager::handle_proc(kiv_os::TRegisters &regs) {
	switch (regs.rax.l) {
	case kiv_os::scClone: {

		if (regs.rcx.l == kiv_os::clCreate_Process) { //create process
			char *prog_name = reinterpret_cast<char*> (regs.rdx.r);
			kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);

			create_process(prog_name, tsi, regs);
		}
		if (regs.rcx.l == kiv_os::clCreate_Thread) { //create thread
			kiv_os::TThread_Proc thread_proc = reinterpret_cast<kiv_os::TThread_Proc> (regs.rdx.r);
			void *data = reinterpret_cast<void*> (regs.rdi.r);

			create_thread(thread_proc, data, regs);
		}
		Unlock_Kernel();
		break;
	}
	case kiv_os::scWait_For: { //wait for processes
		Unlock_Kernel();
		kiv_os::THandle *proc_handles = reinterpret_cast<kiv_os::THandle*> (regs.rdx.r);
		size_t proc_count = static_cast<size_t>(regs.rcx.r);
		wait_for(proc_handles, proc_count);
		break;
	}
	}
}

void ProcessManager::create_process(char *prog_name, kiv_os::TProcess_Startup_Info *tsi, kiv_os::TRegisters &regs) {

	kiv_os::TEntry_Point program = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, prog_name);

	if (program == nullptr) {
		regs.flags.carry = 1;
		regs.rax.r = static_cast<decltype(regs.flags.carry)>(kiv_os::erInvalid_Argument);
		return;

	}

	std::shared_ptr<PCB> pcb = std::make_shared<PCB>();
	std::shared_ptr<PCB> pcb_context;

	
	pcb_context = get_proc_context();
	
	if (pcb_context == nullptr) {
		regs.flags.carry = 1;
		regs.rax.r = static_cast<decltype(regs.flags.carry)>(kiv_os::erOut_Of_Memory);
		return;
	}
	pcb->proc_name = prog_name;
	pcb->ppid = pcb_context->pid;
	pcb->workind_dir = pcb_context->workind_dir;
	pcb->open_files.push_back(nullptr);
	pcb->open_files.push_back(pcb_context->open_files[tsi->stdin]);
	pcb->open_files.push_back(pcb_context->open_files[tsi->stdout]);
	pcb->open_files.push_back(pcb_context->open_files[tsi->stderr]);

	proc_filesystem->add_process(pcb);
	char *args = nullptr;
	if (tsi != nullptr) {
		args = tsi->arg;
	}

	proc_filesystem->lock_pfs();
	//add thread
	std::shared_ptr<TCB> tcb = proc_filesystem->add_thread(pcb);
	if (tcb == nullptr) {
		regs.flags.carry = 1;
		regs.rax.r = static_cast<decltype(regs.flags.carry)>(kiv_os::erOut_Of_Memory);
		return;
	}

	std::thread proc_thread = std::thread(&ProcessManager::run_process, this, program, regs, args);
	tcb->proc_thread = std::move(proc_thread);
	proc_filesystem->unlock_pfs();

	regs.rax.r = static_cast<decltype(regs.rdx.x)>(tcb->tid);

}

void ProcessManager::create_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs) {

	std::shared_ptr<PCB> pcb_context = get_proc_context();

	if (pcb_context == nullptr) {
		regs.flags.carry = 1;
		regs.rax.r = static_cast<decltype(regs.flags.carry)>(kiv_os::erOut_Of_Memory);
		return;
	}

	proc_filesystem->lock_pfs();

	std::shared_ptr<TCB> tcb = proc_filesystem->add_thread(pcb_context);
	if (tcb == nullptr) {
		regs.flags.carry = 1;
		regs.rax.r = static_cast<decltype(regs.flags.carry)>(kiv_os::erOut_Of_Memory);
		return;
	}

	std::thread thread = std::thread(&ProcessManager::run_thread, this, thread_proc, data, regs);
	tcb->proc_thread = std::move(thread);
	proc_filesystem->unlock_pfs();

	regs.rax.r = static_cast<decltype(regs.rdx.x)>(tcb->tid);
}

kiv_os::THandle ProcessManager::wait_for(kiv_os::THandle *proc_handles, size_t proc_count) {

	std::shared_ptr<TCB> tcb;
	kiv_os::THandle proc_handle = kiv_os::erInvalid_Handle;

	for (size_t i = 0; i < proc_count; i++) {
		proc_handle = proc_handles[i];
		tcb = proc_filesystem->get_tcb_by_handle(proc_handle);
		if (tcb != nullptr) {

			if (tcb->proc_thread.joinable()) {
				tcb->proc_thread.join();
			}
			proc_filesystem->remove_thread(proc_handles[i]);
		}
	}
	return proc_handle;
}

std::shared_ptr<PCB> ProcessManager::get_proc_context() {
	std::thread::id thread_id = std::this_thread::get_id();
	return proc_filesystem->get_pcb_by_thread_id(thread_id);
}

kiv_os::THandle ProcessManager::add_handle(std::shared_ptr<Handler> handle) {
	std::shared_ptr<PCB> pcb = get_proc_context();
	std::vector<std::shared_ptr<Handler>>::iterator it = std::find_if(pcb->open_files.begin() + 1, pcb->open_files.end(),
		[&](std::shared_ptr<Handler> element) { return element == nullptr; });

	if (it == pcb->open_files.end()) {
		pcb->open_files.push_back(handle);
		return static_cast<kiv_os::THandle>(pcb->open_files.size() - 1);
	}
	else {
		pcb->open_files[std::distance(pcb->open_files.begin(), it)] = handle;
		return static_cast<kiv_os::THandle>(std::distance(pcb->open_files.begin(), it));
	}

}

void ProcessManager::run_process(kiv_os::TEntry_Point program, kiv_os::TRegisters &regs, char *args) {
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)> (args);

	program(regs);
	close_handles();

	if (args != nullptr) {
		free(args);
		args = nullptr;
	}

}

void ProcessManager::run_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs) {
	thread_proc(data);
	close_handles();
}

bool ProcessManager::close_handle(const kiv_os::THandle hnd) {
	std::shared_ptr<PCB> pcb = get_proc_context();
	if (hnd < pcb->open_files.size() && pcb->open_files[hnd] != nullptr) {
		pcb->open_files[hnd].reset();
		pcb->open_files[hnd] = nullptr;
		return true;
	}
	else {
		return false;
	}

}

void ProcessManager::set_proc_work_dir(std::string working_dir) {
	std::shared_ptr<PCB> pcb = get_proc_context();
	pcb->workind_dir = working_dir;
}

std::string ProcessManager::get_proc_work_dir() {
	std::shared_ptr<PCB> pcb = get_proc_context();
	return pcb->workind_dir;
}

std::shared_ptr<Handler> ProcessManager::get_handle_object(const kiv_os::THandle hnd) {
	std::shared_ptr<PCB> pcb = get_proc_context();
	if (hnd < pcb->open_files.size() && pcb->open_files[hnd] != nullptr) {
		return pcb->open_files[hnd];
	}
	return nullptr;
}

void ProcessManager::close_handles() {
	std::shared_ptr<PCB> pcb = get_proc_context();
	for (size_t i = 0; i < pcb->open_files.size(); i++) {
		if (pcb->open_files[i] != nullptr) {
			pcb->open_files[i].reset();
		}
	}
}

void ProcessManager::init() {
	size_t const proc_count = 1;
	//first pcb
	std::shared_ptr<PCB> pcb = std::make_shared<PCB>();
	pcb->proc_name = "INIT";
	pcb->pid = 1;
	pcb->ppid = 0;
	pcb->workind_dir = "C:";
	//init console
	pcb->open_files.push_back(nullptr);
	pcb->open_files.push_back(std::make_shared<Console>(kiv_os::stdInput));
	pcb->open_files.push_back(std::make_shared<Console>(kiv_os::stdOutput));
	pcb->open_files.push_back(std::make_shared<Console>(kiv_os::stdError));

	proc_filesystem->add_process(pcb);

	proc_filesystem->lock_pfs();
	std::shared_ptr<TCB> tcb = proc_filesystem->add_thread(pcb);
	std::thread proc_thread = std::thread(&ProcessManager::run_init, this);
	tcb->proc_thread = std::move(proc_thread);
	proc_filesystem->unlock_pfs();

	kiv_os::THandle proc_handles[proc_count];
	proc_handles[0] = tcb->tid;
	processManager->wait_for(proc_handles, proc_count);
}

void ProcessManager::run_init() {
	size_t const param_size = 6;
	size_t const proc_count = 1;
	kiv_os::TRegisters regs{ 0 };
	kiv_os::TProcess_Startup_Info tsi;
	tsi.arg = (char*)malloc(param_size * sizeof(char));
	strcpy_s(tsi.arg, param_size, "shell");
	tsi.stdin = kiv_os::stdInput;
	tsi.stdout = kiv_os::stdOutput;
	tsi.stderr = kiv_os::stdError;
	kiv_os::THandle proc_handles[proc_count];

	processManager->create_process("shell", &tsi, regs);
	proc_handles[0] = static_cast<kiv_os::THandle>(regs.rax.r);
	processManager->wait_for(proc_handles, proc_count);
}