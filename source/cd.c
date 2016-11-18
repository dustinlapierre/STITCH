/*
Authors: Dustin Lapierre, Albert Sebastian
to Class: CSI-385-02
Assignment: FAT12 Filesystem
Created: 10.22.2016
CD command
Navigates to the user requested path, changing both the shared path and flc variables

Certification of Authenticity:
I certify that this assignment is entirely my own work.
*/

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

int findFromRoot(char *fname, char* shm_path);
int findFromCurrent(int currentFLC, char* fname,  char* shm_path);
void removeEnd(char* shm_path);

int main(int argc, char* argv[])
{
	//testing for correct number of arguments
	if (argc > 2)
	{
		printf("Error: too many arguments! \n");
		exit(0);
	}

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

	//storing the argument
	char *argumentPath = argv[1];
	char storedPath[1024];

	//making the path uppercase
	int i;
	for(i = 0;argumentPath[i] != '\0';i++)
	{
		argumentPath[i] = toupper(argumentPath[i]);
		storedPath[i] = argumentPath[i];
	}
	storedPath[i+1] = '\0';

	//tokenize path
	char** pathArray = parsePath(argumentPath);

	//no arguments so navigate to root
	if(argc == 1 || (storedPath[0] == '/' && argumentPath[1] == '\0'))
	{
		strcpy(path, "/HOME");
		*currentFLC = 0;

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

		exit(0);
	}

	//open floppy
	FILE_SYSTEM_ID = fopen(getenv("CURRENT_FLOPPY"), "r+");

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

	//setting temp flc
	int flc = *currentFLC;
	char savedPath[1024];
	strcpy(savedPath, path);
	i = 0;

	//working directory is root
	if(storedPath[0] == '/' || *currentFLC == 0)
	{
		strcpy(path, "/HOME");
		flc = 0;

		if(strcmp(pathArray[0], "HOME") == 0)
		{
			i = 1;
		}

	}
	//Traverse directories in path
	if(flc != -1)
	{
		for(;pathArray[i] != NULL;i++)
		{
			if(flc == 0)
			{
				flc = findFromRoot(pathArray[i], path);
			}
			else
			{
				flc = findFromCurrent(flc, pathArray[i], path);
			}
			if(flc == -1)
			{
				break;
			}

		}
	}
	//if the path is valid save the current FLC to shared memory
	//otherwise reset the path back to original
	if(flc == -1)
	{
		strcpy(path, savedPath);
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

int findFromRoot(char* fname, char *shm_path)
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
					//folder is found
					flc = (buffer[32 * entryNum + 27] << 8) + buffer[32 * entryNum + 26];
					if(strcmp(filename, "..") != 0 && strcmp(filename, ".") != 0)
					{
						strcat(shm_path, "/");
						strcat(shm_path, filename);
					}
					else if(strcmp(filename, "..") == 0)
					{
						removeEnd(shm_path);
					}
				}

			}

		}
		sector += 1;
	}
	free(buffer);
	return flc;
}

int findFromCurrent(int currentFLC, char* fname, char *shm_path)
{
	int flc = -1;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

	int sector = 33 + currentFLC - 2;
	int i;
	char filename[8];
	char attr;
	int entryNum;

	//loop through FAT entries for the folder
	while(1)
	{
		//if sector can't be read break
		if(read_sector(sector, buffer) == -1)
		{
			break;
		}

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
					//folder is found
					flc = (buffer[32 * entryNum + 27] << 8) + buffer[32 * entryNum + 26];
					if(strcmp(filename, "..") != 0 && strcmp(filename, ".") != 0)
					{
						strcat(shm_path, "/");
						strcat(shm_path, filename);
					}
					else if(strcmp(filename, "..") == 0)
					{
						removeEnd(shm_path);
					}
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

//removes the last element in the path
void removeEnd(char* shm_path)
{
	char temp[1024];
	int i;

	//copy path into temp
	strcpy(temp, shm_path);

	//tokenize temp
	char ** pathTokens = parsePath(temp);

	//clear path
	strcpy(shm_path, "");

	//rebuild path without last element
	for(i = 0;pathTokens[i + 1] != '\0';i++)
	{
		strcat(shm_path, "/");
		strcat(shm_path, pathTokens[i]);
	}
}
