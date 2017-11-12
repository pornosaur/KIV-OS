#include "ProcessManager.h"
#include <algorithm>
#include <string>      
#include <iostream> 
#ifdef DEBUG
#define LOG(str)	std::cout << "LOG: " << str << std::endl;
#endif
ProcessManager::ProcessManager() {
	proc_filesystem = new ProcFilesystem(); //TODO delete?
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
		break;
	}
	case kiv_os::scWait_For: { //wait for processes
		kiv_os::THandle *proc_handles = reinterpret_cast<kiv_os::THandle*> (regs.rdx.r);
		size_t proc_count = regs.rcx.r;
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
	pcb->proc_name = prog_name;
	pcb->open_files.push_back(tsi->stdin_t);
	pcb->open_files.push_back(tsi->stdout_t);
	pcb->open_files.push_back(tsi->stderr_t);
	std::unique_lock<std::mutex> lck(proc_table_mutex);
	kiv_os::THandle proc_handle = proc_filesystem->add_process(pcb);
	if (pcb->pid != 0) { //first process
		std::thread::id thread_id = std::this_thread::get_id();
		std::vector<std::shared_ptr<PCB>>::iterator it = std::find_if(proc_filesystem->process_table.begin(), proc_filesystem->process_table.end(),
			[&](const std::shared_ptr<PCB>& element) {
			if (element == nullptr) { return false; }
			return element->proc_thread.get_id() == thread_id; 
		});
		if (it == proc_filesystem->process_table.end()) {
			//TODO error
			proc_filesystem->remove_process(proc_handle);
			return;
		}	
		pcb->ppid = (*it)->pid;
	}
	else { //first process
		pcb->ppid = 0; //TODO realy 0?
	}

	std::thread::id thread_id = std::this_thread::get_id();
	std::thread proc_thread = std::thread(program, regs);
	pcb->proc_thread = move(proc_thread);	
	//std::cout << proc_filesystem->pcb_table_to_str();
	regs.rax.r = static_cast<decltype(regs.rdx.x)>(proc_handle);

}

void ProcessManager::create_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs) {
	
	std::shared_ptr<PCB> pcb = std::make_shared<PCB>();
	pcb->proc_name = "thread";

	std::unique_lock<std::mutex> lck(proc_table_mutex);
	kiv_os::THandle proc_handle = proc_filesystem->add_process(pcb);

	std::thread::id thread_id = std::this_thread::get_id();
	std::vector<std::shared_ptr<PCB>>::iterator it = std::find_if(proc_filesystem->process_table.begin(), proc_filesystem->process_table.end(),
		[&](const std::shared_ptr<PCB>& element) { 
		if (element == nullptr) { return false; }
		return element->proc_thread.get_id() == thread_id; });
	if (it == proc_filesystem->process_table.end()) {
		//TODO error
		proc_filesystem->remove_process(proc_handle);
		return;
	}
	pcb->ppid = (*it)->pid;
	pcb->open_files = (*it)->open_files;

	std::thread proc_thread = std::thread(thread_proc, data);
	pcb->proc_thread = move(proc_thread);
	
	regs.rax.r = static_cast<decltype(regs.rdx.x)>(proc_handle);
}
void ProcessManager::wait_for(kiv_os::THandle *proc_handles, size_t proc_count) {

	std::shared_ptr<PCB> pcb;
	kiv_os::THandle proc_handle;
	for (int i = 0; i < proc_count; i++) {
		proc_handle = proc_handles[i];
		pcb = proc_filesystem->process_table[proc_handle];
		
		if (pcb->proc_thread.joinable()) {
			pcb->proc_thread.join();
		}
		proc_filesystem->remove_process(proc_handles[i]);
	}
	//TODO return hadnle
}



