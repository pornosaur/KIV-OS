#include "FS.h"

FS::FS()
{
}


FS::~FS()
{
	if (sb != NULL) {
		FS::sb_remove_all_dentry(&(sb->s_root));
	}
	delete sb;
	sb = NULL;
}

struct super_block *FS::init_super_block(
	unsigned long s_blocksize,
	bool s_dirt,
	unsigned long s_maxbytes,
	struct dentry *s_root,
	unsigned int s_count,
	const std::string &s_id)
{
	struct super_block *s_block = new struct super_block();
	s_block->s_blocksize = s_blocksize;
	s_block->s_dirt = s_dirt;
	s_block->s_maxbytes = s_maxbytes;
	s_block->s_root = s_root;
	s_block->s_count = s_count;
	s_block->s_id = s_id;
	s_block->s_next = FS::sb;

	FS::sb = s_block;
	return s_block;
}

struct dentry* FS::init_dentry(
	struct dentry *d_parent,
	const std::string &d_name,
	unsigned long d_position,
	unsigned long d_dentry_position,
	unsigned int d_file_type,
	unsigned long d_size,
	unsigned long d_blocks)
{
	struct dentry *d_entry = new struct dentry();
	d_entry->d_fs = this;
	d_entry->d_parent = d_parent;
	d_entry->d_name = d_name;
	d_entry->d_count = 0;
	d_entry->d_mounted = 0;
	d_entry->d_position = d_position;
	d_entry->d_dentry_position = d_dentry_position;
	d_entry->d_file_type = d_file_type;
	d_entry->d_size = d_size;
	d_entry->d_blocks = d_blocks;
	d_entry->d_dirt = 0;
	d_entry->d_subdirectories = NULL;
	d_entry->d_next_subdir = NULL;

	if (d_parent != NULL) {
		d_parent->d_count++;
		d_entry->d_next_subdir = d_parent->d_subdirectories;
		d_parent->d_subdirectories = d_entry;
	}

	return d_entry;
}

int FS::sb_remove_dentry(struct dentry *m_dentry) {

	if (m_dentry == NULL || m_dentry->d_count > 0 || m_dentry->d_mounted == 1) {
		return -1;
	}

	struct dentry *parent = m_dentry->d_parent;

	if (m_dentry->d_next_subdir != NULL) {
		parent->d_subdirectories = m_dentry->d_next_subdir;
		delete m_dentry;
		parent->d_count--;
		return 0;
	}

	if (parent->d_count > 1)
	{
		if (parent->d_subdirectories == m_dentry)
		{
			parent->d_subdirectories = m_dentry->d_next_subdir;
		}
		else
		{
			struct dentry *subdir = parent->d_subdirectories;
			while (subdir != NULL) {

				if (subdir->d_next_subdir == m_dentry) {
					subdir->d_next_subdir = m_dentry->d_next_subdir;
					break;
				}
				subdir = subdir->d_next_subdir;
			}
		}
	}
	delete m_dentry;

	parent->d_count--;
	FS::sb_remove_dentry(parent);
	return 0;
}

void FS::sb_remove_all_dentry(struct dentry **d_entry) {

	if (*d_entry == NULL) {
		return;
	}

	FS::sb_remove_all_dentry(&((*d_entry)->d_subdirectories));
	FS::sb_remove_all_dentry(&((*d_entry)->d_next_subdir));

	delete *d_entry;
	*d_entry = NULL;
}

struct dentry *FS::sb_find_dentry_in_dentry(struct dentry * f_dentry, const std::string &name, unsigned int file_type) {

	if (name == "" && f_dentry != NULL && f_dentry->d_file_type == file_type) {
		return f_dentry;
	}

	if (f_dentry == NULL || f_dentry->d_file_type != 0 || f_dentry->d_subdirectories == NULL) {
		return NULL;
	}

	struct dentry *m_dentry = f_dentry->d_subdirectories;
	while (m_dentry != NULL) {

		if (name.compare(m_dentry->d_name) == 0 && m_dentry->d_file_type == file_type) {
			return m_dentry;
		}
		m_dentry = m_dentry->d_next_subdir;
	}

	return NULL;
}

uint16_t FS::translate_return_codes(int fs_ret_code)
{
	switch (fs_ret_code) {

	case FS::ERR_SUCCESS:
		return kiv_os::erSuccess;

	case FS::ERR_FILE_NOT_FOUND:
	case FS::ERR_INVALID_PATH:
		return kiv_os::erFile_Not_Found;

	case FS::ERR_DIRECTORY_IS_NOT_EMPTY:
		return kiv_os::erDir_Not_Empty;

	case FS::ERR_DIRECTORY_IS_FULL:
	case FS::ERR_DISK_IS_FULL:
		return kiv_os::erNo_Left_Space;

	case FS::ERR_FILE_OPEN_BY_OTHER:
	case FS::ERR_PERMISSION_DENIED:
		return kiv_os::erPermission_Denied;

	case FS::ERR_INVALID_ARGUMENTS:
	case FS::ERR_FS_EXISTS:
		return kiv_os::erInvalid_Argument;

	case FS::ERR_DISK_ERROR:
		return kiv_os::erIO;

	case FS::ERR_OUT_OF_MEMORY:
		return kiv_os::erOut_Of_Memory;
	}

	return kiv_os::erInvalid_Argument;
}