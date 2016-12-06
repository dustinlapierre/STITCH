/*
Authors: Dustin Lapierre, Albert Sebastian
to Class: CSI-385-02
Assignment: FAT12 Filesystem
Created: 11.06.2016
LS command
Prints the contents of the specified folder

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
#include "fatSupport.h"
#include "helperFunctions.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;
unsigned char* FAT;

void printRoot();
void printOther(int);
void printFile(int*, char*);

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

	int i;

	//store FAT table
	FAT = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char) * 9);

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
		if (*currentFLC == 0)
		{
			printRoot();
		}
		else
		{
			printOther(*currentFLC);
		}
		return 0;
	}

	//saving the path and flc for reset at the end
	int savedFLC = *currentFLC;
	char savedPath[1024];
	char* pathEnd;
	strcpy(savedPath, path);

	//parse the argument path into tokens
	for (i = 0; argument[i] != '\0'; i++)
	{
		argument[i] = toupper(argument[i]);
	}

	argument[i + 1] = '\0';
	char** pathArray = parsePath(argument);
	int pathLength;

	for (pathLength = 0; pathArray[pathLength] != NULL; pathLength++)
	{
		//just counts the # of paths
	}
	//save the last token in a variable then drop it

	for (i = 0; pathArray[i + 1] != '\0'; i++)
		;
	pathEnd = pathArray[i];
	pathArray[i] = '\0';

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
		printFile(currentFLC, pathEnd);
	}
	else
	{
		printFile(currentFLC, pathEnd);
	}

	//reset variables
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

void printRoot()
{
	printf("Name		Type		File Size		Flc 	\n");
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int filesize;
	int i;
	int sector = 19;

	while (sector < 33)
	{

		read_sector(sector, buffer); //read the first root sector

		int entryNum;
		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{

			if (buffer[32 * entryNum] == 0x00)
			{

				break;
			}

			if (buffer[32 * entryNum] != 0xE5
					&& buffer[32 * entryNum + 11] != 0x0f)
			{

				//prints filename
				for (i = 0; i < 8; i++)
				{
					printf("%c", buffer[32 * entryNum + i]);
				}

				//prints the . before the extension if necessary
				if (buffer[32 * entryNum + 8] != 0x20)
				{
					printf(".");
				}

				//prints the file extension

				for (i = 8; i < 11; i++)
				{
					printf("%C", buffer[32 * entryNum + i]);
				}

				//testing directory vs files
				if (buffer[32 * entryNum + 11] == 0x10)
				{
					printf("	DIR	");
				}
				else
				{
					printf("	FILE 	");
				}
				int filesize = buffer[32 * entryNum + 31]
						| buffer[32 * entryNum + 30]
						| buffer[32 * entryNum + 29]
						| buffer[32 * entryNum + 28];

				printf("	%d			", filesize);

				//printing the FLC least sig bit first
				printf("%d",
						buffer[32 * entryNum + 27]
								| buffer[32 * entryNum + 26]);
				printf("\n");
			}

		}
		sector += 1;
	}

}

void printOther(int currentFLC)
{

	printf("Name		Type		File Size		Flc 	\n");
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int filesize;
	int i;
	int sector = 33 + currentFLC - 2;

	while (1)
	{
		read_sector(sector, buffer); //read the first root sector

		int entryNum;
		for (entryNum = 0; entryNum < 512 / 32; entryNum++)
		{

			if (buffer[32 * entryNum] == 0x00)
			{

				break;
			}
			//prints filename and extension
			if (buffer[32 * entryNum] != 0xE5
					&& buffer[32 * entryNum + 11] != 0x0f)
			{
				for (i = 0; i < 8; i++)
				{
					printf("%c", buffer[32 * entryNum + i]);
				}
				if (buffer[32 * entryNum + 8] != 0x20)
				{
					printf(".");
				}
				else
				{
					printf(" ");
				}
				for (i = 8; i < 11; i++)
				{
					printf("%C", buffer[32 * entryNum + i]);
				}
				//prints filetype
				if (buffer[32 * entryNum + 11] == 0x10)
				{
					printf("	DIR	");
				}
				else
				{
					printf("	FILE 	");
				}
				int filesize = buffer[32 * entryNum + 31]
						| buffer[32 * entryNum + 30]
						| buffer[32 * entryNum + 29]
						| buffer[32 * entryNum + 28];

				printf("	%d			", filesize);

				//printing the FLC least sig bit first
				printf("%d",
						buffer[32 * entryNum + 27]
								| buffer[32 * entryNum + 26]);
				printf("\n");
			}

		}
		if(get_fat_entry(currentFLC, FAT) == 0xfff)
		{
			break;
		}
		currentFLC = get_fat_entry(currentFLC, FAT);
		sector = 33 + currentFLC - 2;
	}

}

void printFile(int* currentFLC, char* pathEnd)
{

	int i;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	if (*currentFLC == 0)
	{
		read_sector(19, buffer);
	}
	else
	{
		read_sector((33 + *currentFLC - 2), buffer);
	}

	char* newPath;
	char filename[8];

	int flc;

	int entryNum;

	for (entryNum = 0; entryNum < 512 / 32; entryNum++)
	{
		if (buffer[32 * entryNum] == 0x00)
		{
			printf("Error: file or directory searched does not exist. \n");
			return;
		}

		if (buffer[32 * entryNum] != 0xE5 && buffer[32 * entryNum + 11] != 0x0f)
		{
			for (i = 0; i < 8; i++)
			{
				if (buffer[32 * entryNum + i] != ' ')
				{
					filename[i] = buffer[32 * entryNum + i];
					//printf("%c" , filename[i]);
				}
				else
				{
					filename[i] = '\0';
				}
			}

			if (strcmp(filename, pathEnd) == 0)
			{

				*currentFLC = buffer[32 * entryNum + 27]
						| buffer[32 * entryNum + 26];
				break;
			}
		}
	}

	if (buffer[32 * entryNum + 11] == 0x10)
	{
		if (buffer[32 * entryNum + 27] | buffer[32 * entryNum + 26] == 0)
		{
			printRoot();
		}
		else
		{
			printOther(*currentFLC);
		}
	}
	else
	{
		printf("Name		Type		File Size		Flc 	\n");
		for (i = 0; i < 8; i++)
		{
			printf("%c", buffer[32 * entryNum + i]);
		}
		if (buffer[32 * entryNum + 8] != 0x20)
		{
			printf(".");
		}
		else
		{
			printf(" ");
		}
		for (i = 8; i < 11; i++)
		{
			printf("%C", buffer[32 * entryNum + i]);
		}

		if (buffer[32 * entryNum + 11] == 0x10)
		{
			printf(" DIR	");
		}
		else
		{
			printf(" FILE	");
		}
		int filesize = buffer[32 * entryNum + 31] | buffer[32 * entryNum + 30]
				| buffer[32 * entryNum + 29] | buffer[32 * entryNum + 28];

		printf("	%d			", filesize);

		//printing the FLC least sig bit first
		printf("%d", buffer[32 * entryNum + 27] | buffer[32 * entryNum + 26]);
		printf("\n");
	}

}
