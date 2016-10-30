#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <inttypes.h>
#include "fatSupport.h"
#include "helperFunctions.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;

void printRoot();
int findFirstDir(char *fname);
int findNextDir(int currentFLC, char* fname);

int main(int argc, char* argv[])
{
	int shm_id;
	int shm_id2;
	key_t key;
	key = 9876;

	//request access to shared memory
	shm_id = shmget(key, 128*sizeof(char), 0666);
	if(shm_id < 0)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//connect this process to shared memory
	char *path = shmat(shm_id, NULL, 0);
	if(path == (char *) -1)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	key = 9880;

	//request access to shared memory
	shm_id2 = shmget(key, sizeof(int), 0666);
	if(shm_id < 0)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//connect this process to shared memory
	int* currentFLC = shmat(shm_id2, NULL, 0);
	if(currentFLC == (int *) -1)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	printf("%i \n", *currentFLC);

	//tokenize path
	char** pathArray = parsePath(path);

	//open floppy
	FILE_SYSTEM_ID = fopen("floppy1", "r+");

	if (FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}

	//current directory is root
	if(pathArray[1] == NULL)
	{
		printRoot();
	}
	//current directory is not root
	else
	{
		//reads root to find flc of first dir in path
		int flc = findFirstDir(pathArray[1]);
		printf("%i \n", flc);
		if(flc == -1)
		{
			printf("Error invalid path \n");
		}
		else
		{
			int i;
			for(i = 0;pathArray[i] != NULL;i++)
			{
				//using the fat table piece the folder together then read from it to get
				//the next flc in the path
				flc = findNextDir(flc, pathArray[i]);
			}
			//using the final flc print the directory
		}
	}


	//disconnect from shared memory
	int del_shm = shmdt(path);
	if(del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}

	//disconnect from shared memory
	del_shm = shmdt(currentFLC);
	if(del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}

	return 0;
}

void printRoot()
{
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

	int sector = 19;
	int i;
	int entryNum;

	//loop through all root sectors
	while(sector < 33)
	{
		read_sector(sector, buffer);

		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{
			if (buffer[32 * entryNum] == 0x00)
			{
				break;
			}

			if (buffer[32 * entryNum] != 0xE5 && buffer[32 * entryNum + 11] != 0x0f)
			{
				for (i = 0; i < 8; i++)
				{
					printf("%c", buffer[32 * entryNum + i]);
				}
				printf("\n");
			}

		}
		sector += 1;
	}
	free(buffer);
}

int findFirstDir(char* fname)
{
	int flc = -1;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

	int sector = 19;
	int i;
	char filename[8];
	char attr;
	int entryNum;

	//loop through all root sectors
	while (sector < 33)
	{
		read_sector(sector, buffer);

		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{

			if (buffer[32 * entryNum] == 0x00)
			{
				break;
			}

			if (buffer[32 * entryNum] != 0xE5 && buffer[32 * entryNum + 11] != 0x0f)
			{
				for (i = 0; i < 8; i++)
				{
					if (buffer[32 * entryNum + i] != ' ')
					{
						filename[i] = buffer[32 * entryNum + i];
					}
				}
				attr = buffer[32 * entryNum + 11];
				if (strcmp(filename, fname) == 0)
				{
					flc = (buffer[32 * entryNum + 27] << 8) + buffer[32 * entryNum + 26];
				}

			}

		}
		sector += 1;
	}
	free(buffer);
	return flc;
}

int findNextDir(int currentFLC, char* fname)
{
	int flc = -1;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

	int sector = 33 + currentFLC - 2;
	int i;
	char filename[8];
	char attr;
	int entryNum;

	//loop through all root sectors
	read_sector(sector, buffer);

	for (entryNum = 0; entryNum < 512 / 32; entryNum++)
	{

		if (buffer[32 * entryNum] == 0x00)
		{
			break;
		}

		if (buffer[32 * entryNum] != 0xE5 && buffer[32 * entryNum + 11] != 0x0f)
		{
			for (i = 0; i < 8; i++)
			{
				if (buffer[32 * entryNum + i] != ' ')
				{
					filename[i] = buffer[32 * entryNum + i];
				}
			}
			attr = buffer[32 * entryNum + 11];
			//if (strcmp(filename, fname) == 0)
			//{
				//flc = (buffer[32 * entryNum + 27] << 8) + buffer[32 * entryNum + 26];
			//}

		}

	}
	free(buffer);
	return flc;
}
