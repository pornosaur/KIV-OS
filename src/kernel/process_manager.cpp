#include "process_manager.h"



void Handle_Proc(kiv_os::TRegisters &regs) {
	switch (regs.rax.l) {
	case kiv_os::scClone: {
		//TODO vlakno/proces
		Create_Process(regs);
		break;
	}
	case kiv_os::scWait_For: {
		Wait_For(regs);
		break;
	}
	}
}

void Create_Process(kiv_os::TRegisters &regs) {

	char *prog_name = reinterpret_cast<char*> (regs.rdx.r);
	kiv_os::TProcess_Startup_Info *tsi = reinterpret_cast<kiv_os::TProcess_Startup_Info*> (regs.rdi.r);
	kiv_os::TEntry_Point program = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, prog_name);
	if (program == nullptr) {
		regs.flags.carry = kiv_os::erInvalid_Argument;
		//regs.rax.r = ""; //TODO

	}
	std::shared_ptr<PCB> pcb = std::make_shared<PCB>();
	pcb->proc_name = prog_name;
	pcb->stdin_t = tsi->stdin_t;
	pcb->stdout_t = tsi->stdout_t;
	pcb->stderr_t = tsi->stderr_t;
	kiv_os::THandle proc_handle = Add_Process(pcb);
	
	pcb->proc_thread = std::thread(program, regs);

	regs.rax.r = static_cast<decltype(regs.rdx.x)>(proc_handle);
	
}

void Wait_For(kiv_os::TRegisters &regs) {
	kiv_os::THandle *proc_handles = reinterpret_cast<kiv_os::THandle*> (regs.rdx.r);
	size_t proc_count = regs.rcx.r;

	for (int i = 0; i < proc_count; i++) {
		std::shared_ptr<PCB> pcb = Get_Process(proc_handles[i]);
		if (pcb !=nullptr && pcb->proc_thread.joinable()) {
			pcb->proc_thread.join();
		}
		if (pcb != nullptr) {
			Remove_Process(proc_handles[i]);
		}
	}

}

