#include "Vfs.h"


Vfs::Vfs()
{
}


Vfs::~Vfs()
{
}


struct Vfs::super_block *Vfs::init_super_block(
	unsigned long s_blocksize,
	bool s_dirt,
	unsigned long long s_maxbytes,
	struct Vfs::dentry *s_root,
	unsigned int s_count,
	std::string s_id)
{	
	struct Vfs::super_block *s_block = new struct Vfs::super_block();
	s_block -> s_blocksize = s_blocksize;
	s_block -> s_dirt = s_dirt;
	s_block -> s_maxbytes = s_maxbytes;
	s_block -> s_root = s_root;
	s_block -> s_count = s_count;
	s_block -> s_id = s_id;
	s_block -> s_next = Vfs::sb;

	Vfs::sb = s_block;
	return s_block;
}

struct Vfs::dentry* Vfs::init_dentry(
	struct Vfs::super_block *d_sb,
	struct Vfs::dentry *d_parent,
	std::string d_name,
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
	struct Vfs::dentry *d_entry = new struct Vfs::dentry();
	d_entry->d_sb = d_sb;
	d_entry->d_parent = d_parent;
	d_entry->d_name = d_name;
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
	struct Vfs::file *file = new struct Vfs::file();
	file->f_dentry = f_dentry;
	file->f_count = f_count;

	return file;
}

Vfs::super_block * Vfs::find_super_block_by_name(std::string name)
{
	struct Vfs::super_block *sb = Vfs::sb;

	while (sb != NULL) {

		if(sb->s_root->d_name == name)
		{
			return sb;
		}
		sb = sb->s_next;
	}

	return NULL;
}

int Vfs::sb_remove_file(struct Vfs::file **file) {
	if ((*file) == NULL) {
		return -1;
	}

	if ((*file)->f_dentry != NULL)
	{
		(*file)->f_dentry->d_count--;
		Vfs::sb_remove_dentry((*file)->f_dentry); // nesmaze se kdyz na nej nekdo odkazuje
		(*file)->f_dentry = NULL;
	}

	delete *file;
	(*file) = NULL;

	return 0;
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
			parent->d_subdirectories = mDentry->d_next_subdir;
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
	delete mDentry;

	parent->d_count--;
	Vfs::sb_remove_dentry(parent);
	return 0;
}

struct Vfs::dentry *Vfs::sb_find_dentry_in_dentry(struct Vfs::dentry * fDentry, std::string name, int file_type) {
	
	if (fDentry == NULL || fDentry->d_file_type != 0 || fDentry->d_subdirectories == NULL) {
		return NULL;
	}

	struct Vfs::dentry *mDentry = fDentry->d_subdirectories;
	while (mDentry != NULL) {
		
		if (name.compare(mDentry->d_name) == 0 && mDentry->d_file_type == file_type) {
			return mDentry;
		}
		mDentry = mDentry->d_next_subdir;
	}

	return NULL;
}

void Vfs::set_file_position(struct Vfs::file * file, unsigned long position)
{
	if (file != NULL || file->f_dentry == NULL) {
		if (file->f_dentry->d_size < position) {
			file->position = file->f_dentry->d_size;
		}
		else if (position < 0) {
			file->position = 0;
		} 
		else {
			file->position = position;
		}
	}
}

unsigned long Vfs::get_file_position(Vfs::file * file)
{
	if (file == NULL) {
		return -1;
	}
	else {
		return file->position;
	}
}
