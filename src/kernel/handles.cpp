#include "handles.h"



Handles::Handles() {
}

void Handles::init_console_handles(){
	std::shared_ptr<Console> conIn = std::make_shared<Console>(kiv_os::stdInput);
	handles.insert(std::pair<kiv_os::THandle, std::shared_ptr<IHandleObject>>(kiv_os::stdInput, conIn));

	std::shared_ptr<Console> conOut = std::make_shared<Console>(kiv_os::stdOutput);
	handles.insert(std::pair<kiv_os::THandle, std::shared_ptr<IHandleObject>>(kiv_os::stdOutput, conOut));

	std::shared_ptr<Console> conError = std::make_shared<Console>(kiv_os::stdError);
	handles.insert(std::pair<kiv_os::THandle, std::shared_ptr<IHandleObject>>(kiv_os::stdError, conError));
}

kiv_os::THandle Handles::add_handle(std::shared_ptr<IHandleObject> hnd) {
	std::lock_guard<std::mutex> guard(Handles_Guard);

	Last_Handle++;

	handles.insert(std::pair<kiv_os::THandle, std::shared_ptr<IHandleObject>>(Last_Handle, hnd));
	return Last_Handle;
}

std::shared_ptr<IHandleObject> Handles::get_handle_object(const kiv_os::THandle hnd) {
	std::lock_guard<std::mutex> guard(Handles_Guard);

	auto resolved = handles.find(hnd);
	if (resolved != handles.end()) {
		return resolved->second;
	}
	else
		return nullptr;
}

bool Handles::Remove_Handle(const kiv_os::THandle hnd) {
	std::lock_guard<std::mutex> guard(Handles_Guard);
	return handles.erase(hnd) == 1;
}

Handles::~Handles() {

}