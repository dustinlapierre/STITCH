#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <ctype.h>
#include "fatSupport.h"
#include "helperFunctions.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;
unsigned char* FAT;

int findFirstDir(char *fname, char *shm_path);
int findNextDir(int currentFLC, char* fname,  char *shm_path);

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		printf("Error: no argument passed! \n");
		exit(0);
	}
	else if (argc > 2)
	{
		printf("Error: too many arguments! \n");
		exit(0);
	}
	char *argumentPath = argv[1];
	char storedPath[128];

	int i;
	for(i = 0;argumentPath[i] != '\0';i++)
	{
		argumentPath[i] = toupper(argumentPath[i]);
		storedPath[i] = argumentPath[i];
	}
	storedPath[i+1] = '\0';

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

	//tokenize path
	char** pathArray = parsePath(argumentPath);

	//open floppy
	FILE_SYSTEM_ID = fopen("floppy1", "r+");

	if (FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}

	//store FAT table
	FAT = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char) * 9);

	for(i = 0;i < 9; i++)
	{
		read_sector(i+1, &FAT[i * BYTES_PER_SECTOR]);
	}

	int flc = *currentFLC;
	i = 0;
	//working directory is root
	if(storedPath[0] == '/' || *currentFLC == 0)
	{
		strcpy(path, "/HOME");
		*currentFLC = 0;

		if(strcmp(pathArray[0], "HOME") == 0)
		{
			i = 1;
		}

		flc = findFirstDir(pathArray[i], path);
		i = 2;
	}
	//traverse
	if(flc != -1)
	{
		for(;pathArray[i] != NULL;i++)
		{
			//using the fat table piece the folder together then read from it to get the next flc in the path
			flc = findNextDir(flc, pathArray[i], path);
		}
	}
	//if the path is valid save the current FLC to shared memory
	if(flc == -1)
	{
		printf("Error invalid path \n");
	}
	else
	{
		*currentFLC = flc;
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

	free(FAT);
	return 0;
}

int findFirstDir(char* fname, char *shm_path)
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
					else
					{
						filename[i] = '\0';
					}
				}
				attr = buffer[32 * entryNum + 11];
				if (strcmp(filename, fname) == 0 && attr == 0x10)
				{
					flc = (buffer[32 * entryNum + 27] << 8) + buffer[32 * entryNum + 26];
					strcat(shm_path, "/");
					strcat(shm_path, filename);
				}

			}

		}
		sector += 1;
	}
	free(buffer);
	return flc;
}

int findNextDir(int currentFLC, char* fname, char *shm_path)
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
	while(1)
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
					else
					{
						filename[i] = '\0';
					}
				}
				attr = buffer[32 * entryNum + 11];
				if (strcmp(filename, fname) == 0 && attr == 0x10)
				{
					flc = (buffer[32 * entryNum + 27] << 8) + buffer[32 * entryNum + 26];
					strcat(shm_path, "/");
					strcat(shm_path, filename);
				}

			}

		}
		if(get_fat_entry(currentFLC, FAT) == 0xfff)
		{
			break;
		}
		currentFLC = get_fat_entry(currentFLC, FAT);
		sector = 33 + currentFLC - 2;
	}
	free(buffer);
	return flc;
}
