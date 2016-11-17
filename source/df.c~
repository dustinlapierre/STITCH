#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <ctype.h>
#include "fatSupport.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;
unsigned char* FAT;

int main(int argc, char* argv[])
{
	//start the memory counter
	int memory = 2847;
	int i;

	//open the floppy
	FILE_SYSTEM_ID = fopen("floppy1", "r+");

	if (FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}

	//store the FAT
	FAT = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char) * 9);
	for(i = 0;i < 9; i++)
	{
		read_sector(i+1, &FAT[i * BYTES_PER_SECTOR]);
	}

	for(i = 2;i < 2847;i++)
	{
		if(get_fat_entry(i, FAT) != 0x00)
		{
			memory -= 1;
		}
	}

	printf("%-15s %-15s %-15s %-15s\n", "512K-blocks", "Used", "Available", "Use%");
	printf("%-15i %-15i %-15i %-15.2f\n", 2847, (2847 - memory), memory, ((2847 - memory)/2847.0) * 100);

	return 0;
}
