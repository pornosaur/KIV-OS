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
	const std::string s_id)
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
	const std::string d_name,
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

struct dentry *FS::sb_find_dentry_in_dentry(struct dentry * f_dentry, const std::string name, unsigned int file_type) {

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