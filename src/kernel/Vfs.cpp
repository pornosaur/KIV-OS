#include "Vfs.h"
#include <stdlib.h>


Vfs::Vfs()
{
}


Vfs::~Vfs()
{
}


void Vfs::init_super_block(
	unsigned long s_blocksize,
	bool s_dirt,
	unsigned long long s_maxbytes,
	struct Vfs::dentry *s_root,
	unsigned int s_count,
	char s_id[32])
{
	Vfs::sb.s_blocksize = s_blocksize;
	Vfs::sb.s_dirt = s_dirt;
	Vfs::sb.s_maxbytes = s_maxbytes;
	Vfs::sb.s_root = s_root;
	Vfs::sb.s_count = s_count;
	strcpy(Vfs::sb.s_id, s_id);
}

struct Vfs::dentry* Vfs::init_dentry(
	struct Vfs::super_block *d_sb,
	struct Vfs::dentry *d_parent,
	char d_name[32],
	unsigned int d_count,
	bool d_mounted,
	unsigned long d_position,
	unsigned long d_file_position,
	unsigned int d_file_type,
	unsigned long d_size,
	unsigned int d_blocks,
	unsigned char d_dirt,
	struct Vfs::dentry *d_subdirectories,
	struct Vfs::dentry *d_next_subdir)
{
	struct Vfs::dentry *d_entry = (struct Vfs::dentry*)malloc(sizeof(struct Vfs::dentry));
	d_entry->d_sb = d_sb;
	d_entry->d_parent = d_parent;
	strcpy(d_entry->d_name, d_name);
	d_entry->d_count = d_count;
	d_entry->d_mounted = d_mounted;
	d_entry->d_position = d_position;
	d_entry->d_file_position = d_file_position;
	d_entry->d_file_type = d_file_type;
	d_entry->d_size = d_size;
	d_entry->d_blocks = d_blocks;
	d_entry->d_dirt = d_dirt;
	d_entry->d_subdirectories = d_subdirectories;
	d_entry->d_next_subdir = d_next_subdir;


	
	return d_entry;
}

struct Vfs::file *Vfs::init_file(
	struct Vfs::dentry *f_dentry,
	unsigned int f_count,
	unsigned long position)
{
	struct Vfs::file *file = (struct Vfs::file*)malloc(sizeof(struct Vfs::file));
	file->f_dentry = f_dentry;
	file->f_count = f_count;
}

int Vfs::sb_remove_file(struct Vfs::file *file) {
	if (file == NULL || file->f_dentry == NULL) {
		return -1;
	}

	file->f_dentry->d_count--;
	Vfs::sb_remove_dentry(file->f_dentry); // nesmaze se kdyz na nej nekdo odkazuje
}

int Vfs::sb_remove_dentry(struct Vfs::dentry *mDentry) {

	if (mDentry == NULL || mDentry->d_count > 0 || mDentry->d_mounted == 1) {
		return -1;
	}

	struct Vfs::dentry *parent = mDentry->d_parent;
	
	if (parent->d_count > 1)
	{
		if (parent->d_subdirectories == mDentry)
		{
			parent->d_subdirectories == mDentry->d_next_subdir;
		}
		else
		{
			struct Vfs::dentry *subdir = parent->d_subdirectories;
			while (subdir != NULL) {

				if (subdir->d_next_subdir == mDentry) {
					subdir->d_next_subdir = mDentry->d_next_subdir;
					break;
				}
				subdir = subdir->d_next_subdir;
			}
		}
	}
	free(mDentry);

	parent->d_count--;
	Vfs::sb_remove_dentry(parent);
	return 0;
}

struct Vfs::dentry *Vfs::sb_find_dentry_in_dentry(struct Vfs::dentry * fDentry, char name[]) {
	
	if (fDentry == NULL || fDentry->d_file_type != 0 || fDentry->d_subdirectories == NULL) {
		return NULL;
	}

	struct Vfs::dentry *mDentry = fDentry->d_subdirectories;
	while (mDentry != NULL) {
		
		if (strcmp(mDentry->d_name, name) == 0) {
			return mDentry;
		}
		mDentry = mDentry->d_next_subdir;
	}

	return NULL;
}
