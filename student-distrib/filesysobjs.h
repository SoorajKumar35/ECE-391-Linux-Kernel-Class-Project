#ifndef FILESYSOBJS_H
#define FILESYSOBJS_H
#include "types.h"



#define START_ADDR_FS 0x00417000
#define END_ADDR_FS 0x00493000
#define SIZE_FS (START_ADDR_FS - END_ADDR_FS)
#define BLOCK_SIZE 4096
#define SIZE_STAT_DIRS 64
#define FS_BLOCK_SIZE 4096
#define NAME_SIZE 32 
#define RESERVED_SIZE 24
#define NUM_DIRS 17
#define SIZE_BUFF 32

typedef struct boot_block_t {
	uint32_t num_dir_entries;
	uint32_t num_inodes;
	uint32_t num_data_blocks;

} boot_block_t;

typedef struct dir_entry_struct
{
	uint8_t filename[NAME_SIZE];
	uint32_t file_type;
	uint32_t inode_num;
	uint8_t reserved[RESERVED_SIZE];

}dentry_t;

typedef struct inode_struct{
	int32_t length;
	uint8_t rest[4092];
}inode_t;


uint32_t terminal_close(uint32_t fd);
extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
extern int32_t read_data (int32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern void copy_dentry(uint8_t * dir_addr, dentry_t* dentry);
extern void copy_boot_block(boot_block_t * boot_block);
extern uint32_t get_num_bytes(int32_t index);

int32_t file_open (const uint8_t* filename);
int32_t file_close (int32_t fd);
int32_t file_write (int32_t fd, const void * buf, int32_t nbytes);
int32_t file_read (int32_t fd, const void * buf, int32_t nbytes);

int32_t dir_open (const uint8_t* filename);
int32_t dir_close (int32_t fd);
int32_t dir_write (int32_t fd, const void * buf, int32_t nbytes);
int32_t dir_read (int32_t fd, const void * buf, int32_t nbytes);


#endif /* FILESYSOBJS_H */ 
