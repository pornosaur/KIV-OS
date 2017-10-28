#include "proc_filesystem.h"
#include "map"

kiv_os::THandle next_pid = 0;
std::map<kiv_os::THandle, std::shared_ptr<PCB>> process_table;

kiv_os::THandle Add_Process(std::shared_ptr<PCB> pcb) {
	pcb->PID = next_pid;
	process_table[next_pid] = pcb;
	next_pid++;
	return pcb->PID;
}

std::shared_ptr<PCB> Get_Process(kiv_os::THandle pid) {
	auto pcb_item = process_table.find(pid);
	if (pcb_item != process_table.end()) {
		return pcb_item->second;
	}
	return nullptr;	
}

void Remove_Process(kiv_os::THandle pid) {
	process_table.erase(pid);
}

