/*
Authors: Dustin Lapierre, Albert Sebastian
to Class: CSI-385-02
Assignment: FAT12 Filesystem
Created: 11.06.2016
RMDIR command
Removes the specified folder

Certification of Authenticity:
I certify that this assignment is entirely my own work.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include "helperFunctions.h"
#include "fatSupport.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;
unsigned char* FAT;

void removeFolder(int, char*);
void removeFolderRoot(char*);
void updateFAT(int);
void removeExtension(char*);
int checkEmpty(int FLC);

int main(int argc, char* argv[])
{
	char *argument;

	if (argc == 2)
	{
		argument = argv[1];
	}
	if (argc > 2)
	{
		printf("Error: too many arguments! \n");
		exit(0);
	}

	FILE_SYSTEM_ID = fopen(getenv("CURRENT_FLOPPY"), "r+");
	if (FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}

	//store FAT table
	FAT = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char) * 9);
	int i;
	for (i = 0; i < 9; i++)
	{
		read_sector(i + 1, &FAT[i * BYTES_PER_SECTOR]);
	}

	int shm_id;
	int shm_id2;
	key_t key;
	key = 9876;

	//request access to shared memory
	shm_id = shmget(key, 128 * sizeof(char), 0666);
	if (shm_id < 0)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//connect this process to shared memory
	char *path = shmat(shm_id, NULL, 0);
	if (path == (char *) -1)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	key = 9880;

	//request access to shared memory
	shm_id2 = shmget(key, sizeof(int), 0666);
	if (shm_id < 0)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//connect this process to shared memory
	int* currentFLC = shmat(shm_id2, NULL, 0);
	if (currentFLC == (int *) -1)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	if (argc == 1)
	{
		printf("Too few arguments: rm requires one argument \n");
	}
	int savedFLC = *currentFLC;
	char savedPath[1024];
	char* pathEnd;
	strcpy(savedPath, path);

	//Convert argument to upper case

	for (i = 0; argument[i] != '\0'; i++)
	{
		argument[i] = toupper(argument[i]);
	}

	argument[i + 1] = '\0';
	char** pathArray = parsePath(argument);
	int pathLength;

	for (pathLength = 0; pathArray[pathLength] != NULL; pathLength++)
		;
	//just counts the # of paths

	//save the last token in a variable then drop it
	for (i = 0; pathArray[i + 1] != '\0'; i++)
		;
	pathEnd = pathArray[i];
	pathArray[i] = '\0';

	//remove any extensions
	removeExtension(pathEnd);

	//build path for cd
	char cdPath[1024];
	if (pathLength > 1)
	{
		if(savedPath[0] == '/')
		{
			cdPath[0] = '/';
		}

		strcat(cdPath, pathArray[0]);

		for (i = 1; pathArray[i] != '\0'; i++)
		{
			strcat(cdPath, "/");
			strcat(cdPath, pathArray[i]);
		}

		int pid = fork();
		if (pid == -1)
		{
			perror("Error creating process\n");
		}
		else if (pid == 0)
		{
			if (execlp("./commands/cd", "cd", cdPath, (char *) NULL) == -1)
			{
				perror("Error loading program into memory");
				exit(EXIT_FAILURE);
			}
		}

		//waiting until child is finished running
		pid_t waitpid;
		waitpid = wait(NULL);
		if (waitpid == -1)
		{
			perror("Error on wait");
			exit(0);
		}

		if (*currentFLC == 0)
		{
			removeFolderRoot(pathEnd);
		}
		else
		{
			removeFolder(*currentFLC, pathEnd);
		}
	}
	else
	{
		if (*currentFLC == 0)
		{
			removeFolderRoot(pathEnd);
		}
		else
		{
			removeFolder(*currentFLC, pathEnd);
		}
	}

	strcpy(path, savedPath);
	*currentFLC = savedFLC;

	//disconnect from shared memory
	int del_shm = shmdt(path);
	if (del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}

	//disconnect from shared memory
	del_shm = shmdt(currentFLC);
	if (del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}
	return 0;
}

//locates the file and checks if it's valid
void removeFolder(int currentFLC, char* pathEnd)
{
	int i;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

	int sector = 33 + currentFLC - 2;
	char filename[8];
	int flc = -1;
	char attr;
	int entryNum;
	int wrongType = 0;

	//loop through FAT entries for the folder
	while (1)
	{
		//if sector can't be read break
		if (read_sector(sector, buffer) == -1)
		{
			break;
		}

		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{

			if (buffer[32 * entryNum] == 0x00)
			{
				break;
			}

			if (buffer[32 * entryNum] != 0xE5
					&& buffer[32 * entryNum + 11] != 0x0f)
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
				if (strcmp(filename, pathEnd) == 0 && attr == 0x10)
				{
					if (strcmp(filename, "..") == 0 || strcmp(filename, ".") == 0)
					{
						//can't delete . and ..
						printf("Error: invalid folder!\n");
						break;
					}
					else
					{
						//folder is found
						flc = (buffer[32 * entryNum + 27] << 8)
								+ buffer[32 * entryNum + 26];
						if (checkEmpty(flc) == 1)
						{
							buffer[32 * entryNum] = 0xE5;
							if (write_sector(sector, buffer) == -1)
							{
								printf("Problem writing to disk \n");
							}
							break;
						}
						else
						{
							printf("Error: folder is not empty!\n");
							break;
						}
					}
				}
				else if (strcmp(filename, pathEnd) == 0 && attr != 0x10)
				{
					//not a folder
					printf("Error: not a folder!\n");
					wrongType = 1;
					break;
				}

			}

		}
		if (get_fat_entry(currentFLC, FAT) == 0xfff)
		{
			break;
		}
		currentFLC = get_fat_entry(currentFLC, FAT);
		sector = 33 + currentFLC - 2;
	}

	if(wrongType == 0)
	{
		updateFAT(flc);
	}
	free(buffer);
}

//locates the file and checks if it's valid in root
void removeFolderRoot(char* pathEnd)
{
	int i;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int sector = 19;
	char filename[8];
	int flc = -1;
	char attr;
	int entryNum;
	int wrongType = 0;

	//loop through FAT entries for the folder
	while (sector < 33)
	{
		//if sector can't be read break
		if (read_sector(sector, buffer) == -1)
		{
			break;
		}

		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{

			if (buffer[32 * entryNum] == 0x00)
			{
				break;
			}

			if (buffer[32 * entryNum] != 0xE5
					&& buffer[32 * entryNum + 11] != 0x0f)
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
				if (strcmp(filename, pathEnd) == 0 && attr == 0x10)
				{
					if (strcmp(filename, "..") == 0 || strcmp(filename, ".") == 0)
					{
						//can't delete . and ..
						printf("Error: invalid folder!\n");
						break;
					}
					else
					{
						//folder is found
						flc = (buffer[32 * entryNum + 27] << 8)
								+ buffer[32 * entryNum + 26];
						if (checkEmpty(flc) == 1)
						{
							buffer[32 * entryNum] = 0xE5;
							if (write_sector(sector, buffer) == -1)
							{
								printf("Problem writing to disk \n");
							}
							break;
						}
						else
						{
							printf("Error: folder is not empty!\n");
							break;
						}
					}
				}
				else if (strcmp(filename, pathEnd) == 0 && attr != 0x10)
				{
					//not a folder
					printf("Error: not a folder!\n");
					wrongType = 1;
					break;
				}

			}

		}
		sector += 1;
	}

	if(wrongType == 0)
	{
		updateFAT(flc);
	}
	free(buffer);
}

//deletes all FAT entries contained in the file
void updateFAT(int flc)
{
	int temp;
	int i;

	if (flc == -1)
	{
		printf("Error: invalid argument or file not found \n");
	}
	else
	{
		while (1)
		{
			if (get_fat_entry(flc, FAT) == 0xfff)
			{
				set_fat_entry(flc, 0x00, FAT);
				break;
			}
			temp = get_fat_entry(flc, FAT);
			set_fat_entry(flc, 0x00, FAT);
			flc = temp;
		}

		for (i = 0; i < 9; i++)
		{
			write_sector(i + 1, &FAT[i * BYTES_PER_SECTOR]);
		}
	}
}

//removes the extension from the filename
void removeExtension(char* filename)
{
	//replaces the period with a null terminator
	int i;
	for (i = 0; filename[i] != '\0'; i++)
	{
		if (filename[i] == '.')
		{
			filename[i] = '\0';
		}
	}
}

//using the flc checks whether the given folder contains only . and ..
int checkEmpty(int currentFLC)
{
	int empty = 1;
	int i;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int sector = 33 + currentFLC - 2;
	char filename[8];
	int entryNum;

	//loop through FAT entries for the folder
	while (1)
	{
		//if sector can't be read break
		if (read_sector(sector, buffer) == -1)
		{
			break;
		}

		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{

			if (buffer[32 * entryNum] == 0x00)
			{
				break;
			}

			if (buffer[32 * entryNum] != 0xE5
					&& buffer[32 * entryNum + 11] != 0x0f)
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
				if (strcmp(filename, "..") != 0 && strcmp(filename, ".") != 0)
				{
					//file or folder is found, not empty
					empty = 0;
					break;
				}

			}

		}
		if (get_fat_entry(currentFLC, FAT) == 0xfff)
		{
			break;
		}
		currentFLC = get_fat_entry(currentFLC, FAT);
		sector = 33 + currentFLC - 2;
	}

	free(buffer);
	return empty;

}
