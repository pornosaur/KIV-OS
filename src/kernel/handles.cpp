#include "Handles.h"
#include "kernel.h"
#include "ProcFilesystem.h"
#include <cassert>
#include <algorithm>
Handles::Handles() {

}

void Handles::init_console_handles(){
	std::shared_ptr<Console> conIn = std::make_shared<Console>(kiv_os::stdInput);
	handle_map.insert(std::pair<kiv_os::THandle, std::shared_ptr<Handler>>(kiv_os::stdInput, conIn));

	std::shared_ptr<Console> conOut = std::make_shared<Console>(kiv_os::stdOutput);
	handle_map.insert(std::pair<kiv_os::THandle, std::shared_ptr<Handler>>(kiv_os::stdOutput, conOut));

	std::shared_ptr<Console> conError = std::make_shared<Console>(kiv_os::stdError);
	handle_map.insert(std::pair<kiv_os::THandle, std::shared_ptr<Handler>>(kiv_os::stdError, conError));
}

kiv_os::THandle Handles::add_handle(std::shared_ptr<Handler> hnd) {
	std::lock_guard<std::mutex> guard(Handles_Guard);

	kiv_os::THandle proc_handle= processManager->add_open_file(hnd);
	//handle_map.insert(std::pair<kiv_os::THandle, std::shared_ptr<Handler>>(next_handle, hnd));
	//next_handle++;
	return proc_handle;
	
	
}

std::shared_ptr<Handler> Handles::get_handle_object(const kiv_os::THandle hnd) {
	std::lock_guard<std::mutex> guard(Handles_Guard);
	std::shared_ptr<PCB> pcb = processManager->get_proc_context();
	return pcb->open_files[hnd];
	/*auto resolved = handle_map.find(handle_index);
	if (resolved != handle_map.end()) {
		return resolved->second;
	}
	else {
		return nullptr;
	}*/
		
}

bool Handles::Remove_Handle(const kiv_os::THandle hnd) {
	std::lock_guard<std::mutex> guard(Handles_Guard);
	return processManager->close_handle(hnd);
}

Handles::~Handles() {

}