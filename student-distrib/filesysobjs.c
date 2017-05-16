#include "filesysobjs.h"
#include "lib.h"
#include "keyboard_driver.h"
#include "syscalls.h"
/* FUNCTION: 	read_dentry_by_name
   INPUTS: 		fname -> pointer to a buffer that contain a filename
   				dentry -> pointer to directory entry struct
   OUTPUTS: 	0 -> success
   				-1 -> file name not found in the file system


*/


//extern int32_t directoryIndex = 0;
// how to get value of length of bytes to pass around
int32_t length_in_bytes_glb = 0;

 dentry_t * fs_dentries;
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{

	if(fname == NULL || fname[0] == NULL || fname[0] == '\0') return -1;

	//get start address of file system
	//uint8_t * start_addr = (uint8_t *)START_ADDR_FS;
	uint8_t * start_addr = (uint8_t *)start_fs;	
	//get the number of directories total
	uint32_t num_dir = *start_addr;
/*	printf("NUM DIRECTORY %d\n", num_dir);
*/	//offset to the start of the first directory
	int offset = SIZE_STAT_DIRS;
	start_addr += offset;

	fs_dentries = (dentry_t *)(start_fs + SIZE_STAT_DIRS); 
	//cycle through directory entries. Compare file names 
	//and if they match copy the data over to the dentry 
	//ptr passed in
	uint8_t * dir_addr = (uint8_t *)start_addr;
	int n = 0;
	for(n = 0,dir_addr = (uint8_t *) start_addr; n < num_dir; dir_addr += SIZE_STAT_DIRS, n++ )
	{
		//printf("READ DENTRY-INSIDE FOR LOOP");
		//int i = 0;
		//uint32_t * test = (uint32_t *)fname;
		uint32_t * s_dir_addr = (uint32_t *)dir_addr;
		char x = 0;
		int flag = 0; // a flag that will help in moving to another directory when the filename doesn't match
		//a loop that will compare byte by byte the two filenames to check if they are the same

		// printf("%s ", s_dir_addr);
		 if((x = strncmp((int8_t *) s_dir_addr, (int8_t *)fname, 
			                  strlen((int8_t *) fname))) == 0) { 
		 	    //  printf("%s ", s_dir_addr);
		 		 // printf("FOUND\n");
		 		 // printf("%s", fname);
		 } else { 
		 		//printf("%s\n", fs_dentries[n].filename);
		 		//printf("%s\n", fname);
		 		flag = 1; 
		 }
		// 	for(i = 0; i < 32; i++,s_dir_addr++,test++)
		// 	{


		// 			if(!(*s_dir_addr == *test))
		// 			{
		// 				flag = 1; //set the flag true if a difference exists
		// 				break;
		// 			}	
		// }

		// printf("%d", i); 
		// printf("%d", s_dir_addr);
		// printf("%d", test);
		//if the flag is set then move on to the next directory
		if(flag == 1)
		{
			continue;
		}

		//while(1);
		//otherwise, the filename has matched and we copy over the data to the directory entry
		memcpy(dentry, dir_addr, SIZE_STAT_DIRS);
		//copy_dentry(dir_addr, dentry);
		return 0;
	}
	return -1;
}

/* FUNCTION: 	read_dentry_by_index
   INPUTS: 		index -> index of the file in file system
   				dentry -> pointer to directory entry struct
   OUTPUTS: 	0 -> success
   				-1 -> index is invalid
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
	//get the start address and then get the num of dirs
	//uint8_t * start_addr = (uint8_t *)START_ADDR_FS;
	uint8_t * start_addr = (uint8_t *)start_fs;
	
	uint32_t num_dir = *start_addr + (*(start_addr + 1)<<8) + (*(start_addr + 2)<<16) + (*(start_addr + 3)<<24);

	//offset to the start of the directory entries array
	start_addr += SIZE_STAT_DIRS;

	//check if the index breaks any required parameters	
	if((index >= num_dir) || (index < 0))
	{
		//printf("BAD INDEX");
		return -1;
	}

	//get the address of the directory that is to be copied
	uint8_t * dir_addr = (uint8_t *)(start_addr + SIZE_STAT_DIRS * index);

	//copy the directory
	copy_dentry(dir_addr, dentry);

	return 0;
}

/******************************************************************************************************************************
 FUNCTION: 	read_data
   INPUTS:		inode -> the inode number
   				offset -> the offset into the datablock
   				buf -> the buffer that is meant to hold the data
   				length -> the number of bytes to be read
   OUTPUTS: 	number of bytes read -> success
   				0 -> invlaid data block num
*******************************************************************************************************************************/
int32_t read_data (int32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	//create a new directory entry and the copy the dentry data over
	//dentry_t dentry;
	
	//get the starting address of the filesystem and the number of directories
	uint8_t * start_addr = (uint8_t *)start_fs;
	uint32_t * long_start_addr = (uint32_t *)start_fs;

	//get the num of inodes from the boot block
	uint32_t num_inodes = *(long_start_addr+1);

	//get the num of datablocks
	uint32_t num_data_blocks = *(long_start_addr + 2);

	//get the starting address of the inode req
	uint8_t * start_node_addr = (uint8_t*)(start_addr + FS_BLOCK_SIZE + (inode * FS_BLOCK_SIZE));
	uint32_t * long_node_addr = (uint32_t *)(long_start_addr + (FS_BLOCK_SIZE/4) + (inode * (FS_BLOCK_SIZE/4)));
	//Get the length in bytes of the data to be read
	uint32_t length_in_bytes = *long_node_addr;

	//Check if there is no data to be read and return -1 if so
	if(length_in_bytes == 0)
	{
		return -1;
	} 
	//Get the number of data blocks to be read so we don't read unneccessary blocks

	// if(offset >= length) { 

	// 	return -1;  
	// }

	if(length + offset >= length_in_bytes) { 
		length = length_in_bytes - offset; 
	}

	int i;
	uint32_t block_offset = 4 *((offset / FS_BLOCK_SIZE) + 1); 
	uint32_t file_length = *(long_node_addr); 
	//printf("FILE LENGTH %d", file_length); 
//	printf("offset %d", offset);
	uint32_t *data_block_ptr = (long_node_addr + block_offset / 4); 
	uint32_t block_size_counter = (BLOCK_SIZE - offset % BLOCK_SIZE); 
	uint8_t *block_addr = start_fs + FS_BLOCK_SIZE + (num_inodes * FS_BLOCK_SIZE) + (*(data_block_ptr) * FS_BLOCK_SIZE); 
	block_addr += (offset % FS_BLOCK_SIZE);

	for(i = 0; i < length; i++, block_addr++) {

		if(block_size_counter == 0) { 
			data_block_ptr += 1; //get next data block number 
			block_addr = start_fs + FS_BLOCK_SIZE + (num_inodes * FS_BLOCK_SIZE) + (*(data_block_ptr) * FS_BLOCK_SIZE); 
			block_size_counter = BLOCK_SIZE; 
		} 

		buf[i] = *(block_addr); 
		block_size_counter--; 
	}

//printf("BUFFER IS: %s\n", buf);
  //  printf("is is %d\n", i);
	return i;
}

void copy_dentry(uint8_t * dir_addr, dentry_t * dentry)
{
	//Get the file name and store in the directory entry
	int m = 0;
	for(m = 0; m < SIZE_BUFF; m++,dir_addr++)
	{
		dentry->filename[m] = *dir_addr;
	}

	//Get the file type
	dentry -> file_type = *dir_addr;
	dir_addr += 4;

	//Get the inode num
	dentry -> inode_num = *dir_addr;
	dir_addr += 4;

	//Get the reserved bits in the directory entry
	int n = 0;
	for(n = 0; n < 24; n++, dir_addr++)
	{
		dentry -> reserved[n] = *dir_addr;
	}

}

void copy_boot_block(boot_block_t * boot_block)
{
	//get the starting address 
	//uint8_t * start_addr = (uint8_t *)START_ADDR_FS;
	uint32_t * start_addr = (uint32_t *)start_fs;

	//get the num of directories and copy it into the param
	uint32_t num_dir = *start_addr;
	boot_block -> num_dir_entries = num_dir;
	start_addr += 1;

	//get the num of inodes and copy it into the param
	uint32_t num_inodes = *start_addr;
	boot_block -> num_inodes = num_inodes;
	start_addr += 1;

	//fet the num of data blocks and copy it into the param
	uint32_t num_data_blocks = *start_addr;
	boot_block -> num_data_blocks = num_data_blocks;

	return;
}


uint32_t get_num_bytes(int32_t index)
{
	//get the starting address 
	//uint8_t * start_addr = (uint8_t *)START_ADDR_FS;
	uint32_t * start_addr = (uint32_t *)start_fs;
	//get to the start of the directories array
	start_addr += SIZE_STAT_DIRS/4;

	//get to the required directory
	start_addr += (index * (SIZE_STAT_DIRS/4));

	//get the inode num
	start_addr += (8 + 1);
	uint32_t inode_num = *start_addr;

	//get to the required inode
	start_addr = (uint32_t *)start_fs;
	start_addr += FS_BLOCK_SIZE/4;
	start_addr += (inode_num * (FS_BLOCK_SIZE/4));

	//get the number of bytes in the file
	uint32_t num_bytes = *start_addr;
	return num_bytes; 
}


//should never happen
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes)
{
	//printf("dir write return -1");

	return -1;
}

//0 on success, question does anything happen?
int32_t dir_open (const uint8_t* filename)
{
	//printf("dir open return 0\n");

    return 0;
}


//does anything happen for directory close?
int32_t dir_close (int32_t fd)
{
	//printf("dir close return 0\n");


    return 0;
}


//aamann2
int32_t dir_read (int32_t fd, const void* buf, int32_t nbytes)
{
     dentry_t dentry;
	//printf("fd in dir_read %d\n", fd);
     static int directoryIndex = 0; 
     if (read_dentry_by_index(directoryIndex, &dentry) == 0)
     {
         //clear buffer
     	//tells us what index we are reading from 
     	//printf("directoryIndex: %d\n", directoryIndex );
     	directoryIndex = directoryIndex +1;
   
     	 int i;

         for (i = 0; i < SIZE_BUFF+1; i++)
             ((int8_t*)(buf))[i] = '\0';

         //gets file name
         int32_t name = strlen((int8_t*)dentry.filename);

         if(name > SIZE_BUFF)
         	name = SIZE_BUFF;


         strncpy((int8_t*)buf, (int8_t*)dentry.filename, name);

         return name;
     }
     else
     {

     	directoryIndex = 0;
     	// printf("dir read return -1\n");

        return 0;
     }

}

//what to do for file_open
//return 0 on success
int32_t file_open (const uint8_t* filename)
{
	//printf("file open return 0");

    return 0;
}

//read only file system
// returns -1 on failure
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes)
{
	//printf("file write return -1");

    return -1;
}

//return zero on success

int32_t file_close (int32_t fd)
{
	//printf("file close return 0");

    return 0;
}


//aamann2
int32_t file_read (int32_t fd, const void* buf, int32_t nbytes)
{
	/* Error check: Buf cannot be a null pointer */
	if(buf == NULL) return 0;

	/* Make certain that the buffer is clear */
	memset(buf, '\0', nbytes);

    /* Get current PCB Pointer */
    pcb_t *pcb = terms[current_terminal].running_pcb;
    int32_t bytes_read;

    if ((bytes_read = read_data(pcb->fd_array[fd].info,
         pcb->fd_array[fd].file_pos, (uint8_t*)buf, nbytes)) > 0 ){

    	/* Increment the file position offset */
    	pcb->fd_array[fd].file_pos += bytes_read;

    	return bytes_read;
    }
    
    return 0;
}





