#pragma once
#undef stdin
#undef stdout
#undef stderr
#include "..\api\api.h"
#include <cassert>
#include <string>

/** characters which are deleted from start and end of string */
#define PATH_ERASE_CHR "\\."
/** Path delimeters */
#define DELIMETER "\\"
/** Size of path deliemter */
#define DELIMETER_SIZE 1

/**
 * By the register rax.l decide which function should be call
 *
 * @param regs structure represent registres
 */
void HandleIO(kiv_os::TRegisters &regs);

/**
 * Open or create file/directory
 * IN:	rdx pointer to null-terminated ANSI char string representing file_name
 *		rcx flags to open file - api fm constants
 *		rdi new file attributes
 * OUT:	ax contains handle to open file
 *
 * On error is set flags.carry and in rax is error code
 *
 * @param regs structure represent registres
 */
void create_file(kiv_os::TRegisters &regs);

/**
* Write data to file
* IN:	dx handle of file
*		rdi pointer to buffer of written data
*		rcx size of data in buffer
* OUT:	rax number of written bytes
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void write_file(kiv_os::TRegisters &regs);

/**
* Read data from file
* IN:	dx handle of file
*		rdi pointer to buffer where will be read data
*		rcx size of buffer
* OUT:	rax number of read bytes
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void read_file(kiv_os::TRegisters &regs);

/**
* Delete file or directory
* IN:	rdx pointer to null-terminated ANSI char string representing file_name
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void delete_file(kiv_os::TRegisters &regs);

/**
* Set position in file and can set size of file
* IN:	dx handle of file
*		rdi new position in file
*		cl typ of position (api fs constant)
*		ch == 0 only set possition (api fsSet_Position)
*		ch == 1 set possition and set file size to this postion (api fsSet_Size)
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void set_file_position(kiv_os::TRegisters &regs);

/**
* Get actual position in file
* IN:	dx handle of file
* OUT:	rax new file position
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void get_file_position(kiv_os::TRegisters &regs);

/**
* Close input handle
* IN:	dx handle of arbitrary type
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void close_handle(kiv_os::TRegisters &regs);

/**
* Return current working directory
* IN:	rdx pointer to ANSI char buffer
*		rcx size of buffer
* OUT:	rax number of written chars to buffer
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void get_current_directory(kiv_os::TRegisters &regs);

/**
* Set current working directory. If directory do not exists -> return error in rax
* IN:	rdx pointer to null-terminated ANSI char string representing new directory (Can be relative path)
*
* On error is set flags.carry and in rax is error code
*
* @param regs structure represent registres
*/
void set_current_directory(kiv_os::TRegisters &regs);

/**
* TODO
*/
void create_pipe(kiv_os::TRegisters &regs);

/**
 * Set to regs.rax.r error code and set carry bit
 *
 * @param regs structure represent registres
 * @param code error codes defined in api.h
 */
void set_error(kiv_os::TRegisters &regs, uint16_t code);

/**
 * Transform absolute path with ".." to valid path.
 * C:\Users\Tomas\..\Honza => C:\Users\Honza
 * 
 * @param path which is compiled
 */
void path_compiler(std::string &path);