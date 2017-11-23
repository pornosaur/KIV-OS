#include "rtl.h"

#include <atomic>

extern "C" __declspec(dllimport) void __stdcall Sys_Call(kiv_os::TRegisters &context);


std::atomic<uint16_t> Last_Error;

uint16_t kiv_os_rtl::Get_Last_Error() {
	return Last_Error;
}

kiv_os::TRegisters Prepare_SysCall_Context(uint8_t major, uint8_t minor) {
	kiv_os::TRegisters regs;
	regs.rax.h = major;
	regs.rax.l = minor;
	return regs;
}

bool Do_SysCall(kiv_os::TRegisters &regs) {
	Sys_Call(regs);

	if (regs.flags.carry) Last_Error = static_cast<uint16_t>(regs.rax.r);
		else Last_Error = kiv_os::erSuccess;

	return !regs.flags.carry;
}

kiv_os::THandle kiv_os_rtl::Create_File(const char* file_name, size_t flags, uint8_t atributes) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scCreate_File);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(file_name);
	regs.rcx.r = static_cast<decltype(regs.rcx.r)>(flags);
	regs.rdi.r = static_cast<decltype(regs.rdi.r)>(atributes);
	regs.flags.carry = 0;

	const bool result = Do_SysCall(regs);

	if (result) {
		return static_cast<kiv_os::THandle>(regs.rax.x);
	}
		
	return 0;
}


bool kiv_os_rtl::Write_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scWrite_File);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(buffer);
	regs.rcx.r = static_cast<decltype(regs.rcx.r)>(buffer_size);	
	regs.flags.carry = 0;

	const bool result = Do_SysCall(regs);
	written = static_cast<size_t>(regs.rax.r);
	return result;
}

bool kiv_os_rtl::Close_File(const kiv_os::THandle file_handle) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scClose_Handle);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.flags.carry = 0;

	return Do_SysCall(regs);
}

bool kiv_os_rtl::Read_File(const kiv_os::THandle file_handle, void *buffer, const size_t buffer_size, size_t &read) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scRead_File);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(buffer);
	regs.rcx.r = static_cast<decltype(regs.rcx.r)>(buffer_size);
	regs.flags.carry = 0;

	const bool result = Do_SysCall(regs);
	read = static_cast<size_t>(regs.rax.r);
	return result;
}

bool kiv_os_rtl::Remove_File(const char* file_name) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scDelete_File);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(file_name);
	regs.flags.carry = 0;

	return Do_SysCall(regs);
}

bool kiv_os_rtl::Set_File_Position(const kiv_os::THandle file_handle, long &position, uint8_t origin, uint16_t set_size) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scSet_File_Position);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = static_cast<decltype(regs.rdi.r)>(position);
	regs.rcx.l = static_cast<decltype(regs.rcx.l)>(origin);
	regs.rcx.h = static_cast<decltype(regs.rcx.h)>(set_size);
	regs.flags.carry = 0;

	return Do_SysCall(regs);
}

bool kiv_os_rtl::Get_File_Position(const kiv_os::THandle file_handle, size_t &position) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scGet_File_Position);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.flags.carry = 0;

	const bool result = Do_SysCall(regs);
	position = static_cast<size_t>(regs.rax.r);
	return result;
}

bool kiv_os_rtl::Set_Current_Directory(const char *path) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scSet_Current_Directory);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(path);
	regs.flags.carry = 0;

	return Do_SysCall(regs);
}

bool kiv_os_rtl::Get_Current_Direcotry(const void *buffer, const size_t buffer_size, size_t &read) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scGet_Current_Directory);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(buffer);
	regs.rcx.r = static_cast<decltype(regs.rcx.r)>(buffer_size);
	regs.flags.carry = 0;

	const bool result = Do_SysCall(regs);
	read = static_cast<size_t>(regs.rax.r);
	return result;
}

bool kiv_os_rtl::Create_Process(const char *program_name, kiv_os::TProcess_Startup_Info tsi, kiv_os::THandle &process_handle) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scProc, kiv_os::scClone);
	regs.rcx.l = kiv_os::clCreate_Process;
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(program_name);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(&tsi);
	regs.flags.carry = 0;
	
	const bool result = Do_SysCall(regs);
	process_handle = static_cast<kiv_os::THandle>(regs.rax.x);
	return result;
}

bool kiv_os_rtl::Create_Thread(kiv_os::TThread_Proc ttp, void *data, kiv_os::THandle &process_handle) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scProc, kiv_os::scClone);
	regs.rcx.l = kiv_os::clCreate_Thread;
	regs.flags.carry = 0;

	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(ttp);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)> (data);

	const bool result = Do_SysCall(regs);
	process_handle = static_cast<kiv_os::THandle>(regs.rax.x);
	return result;


}

bool kiv_os_rtl::Wait_For(std::vector<kiv_os::THandle> proc_handles, const size_t count) {
	if (count != 0) {
		kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scProc, kiv_os::scWait_For);
		kiv_os::THandle *proc_handles_arr = &proc_handles[0];
		regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(proc_handles_arr);
		regs.rcx.r = static_cast<decltype(regs.rcx.r)>(proc_handles.size());
		regs.flags.carry = 0;

		return Do_SysCall(regs);
	}
	
	return true;
}
	
bool kiv_os_rtl::Create_Pipe(kiv_os::THandle pipe_handles[2]) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scCreate_Pipe);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(pipe_handles);
	regs.flags.carry = 0;

	return Do_SysCall(regs);
}

void kiv_os_rtl::print_error()
{
	switch (Last_Error) {
		case kiv_os::erInvalid_Handle:
			print_error("Internal error. (Invalid Handle)");
			break;

		case kiv_os::erInvalid_Argument:
			print_error("Invalid input arugments.");
			break;

		case kiv_os::erFile_Not_Found:
			print_error("System can not find path.");
			break;

		case kiv_os::erDir_Not_Empty:
			print_error("Directory is not empty.");
			break;

		case kiv_os::erNo_Left_Space:
			print_error("Out of disk space.");
			break;

		case kiv_os::erPermission_Denied:
			print_error("Operation is not permitted.");
			break;

		case kiv_os::erOut_Of_Memory:
			print_error("Out of memory.");
			break;

		case kiv_os::erIO:
			print_error("Disk error.");
			break;
		}
}

void kiv_os_rtl::print_error(std::string msg)
{
	size_t writen = 0;

	msg.append("\n\n");

	bool res = kiv_os_rtl::Write_File(kiv_os::stdError, msg.c_str(), msg.size(), writen);
	if (!res) {
		return;
	}
}