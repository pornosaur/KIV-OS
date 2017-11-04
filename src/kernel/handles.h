#pragma once

#include "..\api\api.h"

#include <Windows.h>
#include "IHandleObject.h"
#include <map>
#include <mutex>
#include "console.h"
class Handles {

public:
	Handles();
	~Handles();

	void init_console_handles();
	kiv_os::THandle add_handle(std::shared_ptr<IHandleObject> hnd);
	std::shared_ptr<IHandleObject> get_handle_object(const kiv_os::THandle hnd);
	bool Remove_Handle(const kiv_os::THandle hnd);
private:
	std::map<kiv_os::THandle, std::shared_ptr<IHandleObject>> handles;
	std::mutex Handles_Guard;
	kiv_os::THandle Last_Handle = 3;
};

