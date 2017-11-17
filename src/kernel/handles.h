#pragma once

#include "..\api\api.h"

#include <Windows.h>
#include "Handler.h"
#include <map>
#include <mutex>

#include "console.h"

#define HANDLE_TABLE_SIZE		1024

class Handles {


public:
	Handles();
	~Handles();

	void init_console_handles();
	kiv_os::THandle add_handle(std::shared_ptr<Handler> hnd);
	std::shared_ptr<Handler> get_handle_object(const kiv_os::THandle hnd);
	bool Remove_Handle(const kiv_os::THandle hnd);
	
private:
	std::map<kiv_os::THandle, std::shared_ptr<Handler>> handle_map;
	std::mutex Handles_Guard;
	kiv_os::THandle next_handle = 4;
};

