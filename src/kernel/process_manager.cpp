#include "process_manager.h"
#include <algorithm>
#ifdef DEBUG
#define LOG(str)	std::cout << "LOG: " << str << std::endl;
#endif
ProcessManager::ProcessManager() {

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
		regs.flags.carry = kiv_os::erInvalid_Argument;
		//regs.rax.r = ""; //TODO
		return;

	}
	std::shared_ptr<PCB> pcb = std::make_shared<PCB>();
	pcb->proc_name = prog_name;
	pcb->stdin_t = tsi->stdin_t;
	pcb->stdout_t = tsi->stdout_t;
	pcb->stderr_t = tsi->stderr_t;
	kiv_os::THandle proc_handle = Add_Process(pcb);
	if (pcb->pid != 0) { //first process
		std::thread::id thread_id = std::this_thread::get_id();
		std::vector<thread_item>::iterator it = std::find_if(thread_list.begin(), thread_list.end(),
			[&](const thread_item& element) { return element.second.get_id() == thread_id; });
		
		pcb->ppid = it->first;
		
	}
	std::thread proc_thread = std::thread(program, regs);
	thread_list.push_back(thread_item(pcb->pid, move(proc_thread)));

	
	regs.rax.r = static_cast<decltype(regs.rdx.x)>(proc_handle);

}

void ProcessManager::create_thread(kiv_os::TThread_Proc thread_proc, void *data, kiv_os::TRegisters &regs) {
	
	std::shared_ptr<PCB> pcb = std::make_shared<PCB>();
	pcb->proc_name = nullptr;
	kiv_os::THandle proc_handle = Add_Process(pcb);
	std::thread proc_thread = std::thread(thread_proc, data);
	
	thread_list.push_back(std::pair<kiv_os::THandle, std::thread>(pcb->pid, move(proc_thread)));

	regs.rax.r = static_cast<decltype(regs.rdx.x)>(proc_handle);
}
void ProcessManager::wait_for(kiv_os::THandle *proc_handles, size_t proc_count) {

	std::shared_ptr<PCB> pcb;
	for (int i = 0; i < proc_count; i++) {
		if ((pcb=Get_Process(proc_handles[i])) != nullptr) {
			std::vector<thread_item>::iterator it = std::find_if(thread_list.begin(), thread_list.end(),
				[&](const thread_item& element) { return element.first == pcb->pid; });

			if (it->second.joinable()) {
				it->second.join();
			}
			thread_list.erase(it);
			Remove_Process(proc_handles[i]);
		}
	}
		
}



