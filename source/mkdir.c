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

void makeDirRoot(char*,char*);
void makeDir(int, char*,char*);
void removeExtension(char*);
char* saveExtension(char*);

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
		printf("Too few arguments: cat requires one argument \n");
	}
	int savedFLC = *currentFLC;
	char savedPath[1024];
	char* pathEnd;
	strcpy(savedPath, path);

	for (i = 0; argument[i] != '\0'; i++)
	{
		argument[i] = toupper(argument[i]);
	}

	argument[i + 1] = '\0';
	char** pathArray = parsePath(argument);
	int pathLength;
	int pathEndLength = 0;

	for (pathLength = 0; pathArray[pathLength] != NULL; pathLength++)
		;
	//just counts the # of paths

	//save the last token in a variable then drop it
	for (i = 0; pathArray[i + 1] != '\0'; i++)
		;
	pathEnd = pathArray[i];
	pathArray[i] = '\0';
	//checks to see if the filename is over 8 if so it quits touch prematurely

	//saves any extensions then removes for processing
  char* extension;
	extension = saveExtension(pathEnd);
  removeExtension(pathEnd);
  while (pathEnd[pathEndLength] != '\0')
	{
		pathEndLength++;
	}

	if (pathEndLength > 8)
	{
		printf("Filename too large! \n");
		return 0;
	}

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
			makeDirRoot(pathEnd, extension);
		}
		else
		{
			makeDir(*currentFLC, pathEnd, extension);
		}
	}
	else
	{
		if (*currentFLC == 0)
		{
			makeDirRoot(pathEnd, extension);
		}
		else
		{
			makeDir(*currentFLC, pathEnd, extension);
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

void makeDir(int FLC, char* pathEnd, char* ext)
{
	int i;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int sector = 33 + FLC - 2;
	char filename[8];
	int flc = -1;
	char attr;
	int entryNum;
	int exists = 0;
	int openFLC;
	int openSpace;
	int exitLoop = 0;
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

			if (buffer[32 * entryNum] == 0x00 || buffer[32 * entryNum] == 0xE5)
			{
				exitLoop = 1;
				openSpace = entryNum;
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
				if (strcmp(filename, pathEnd) == 0 && attr != 0x10)
				{
					//file is found
					exists = 1;
					printf("Error: directory already exists. \n");
					break;
				}

			}

		}
		if (get_fat_entry(FLC, FAT) == 0xfff)
		{
			break;
		}
		FLC = get_fat_entry(FLC, FAT);
		sector = 33 + FLC - 2;
	}
	if (exitLoop == 0)
	{
		printf("No space in sector for new Directory. \n");
		return;
	}
	if (exists == 1)
	{
		printf("Error: Directory already exists \n");
	}
	else
	{
		int pad = 0;
		//changes FLC to whatever first open FAT entry is
		//changes filename and sets the first byte away from 0xe5
		//pads end of filename with spaces
		for (i = 0; i < 8; i++)
		{
			if (pathEnd[i] == '\0' || pad == 1)
			{
				buffer[32 * openSpace + i] = ' ';
				pad = 1;
			}
			else
				buffer[32 * openSpace + i] = pathEnd[i];
			//printf("%c", buffer[32 * openSpace + i]);
		}

		//writing changes to FAT table
		//searches whole fat table for first available 0x00 entry
		//takes the index of that FAT entry and sets it to 0xfff
		for (i = 2; i < 2880; i++)
		{
			if (get_fat_entry(i, FAT) == 0x0)
			{
				openFLC = i;
				set_fat_entry(openFLC, 0xfff, FAT);
				break;
			}
		}

		//sets flc for LS
		buffer[32 * openSpace + 27] = openFLC << 8;
		buffer[32 * openSpace + 26] = openFLC;

		//changing filesize to 0
		buffer[32 * openSpace + 28] = 0;
		buffer[32 * openSpace + 29] = 0 << 8;
		buffer[32 * openSpace + 30] = 0 << 16;
		buffer[32 * openSpace + 31] = 0 << 24;

		//sets attribute to directory
		buffer[32 * openSpace + 11] = 0x10;

    //if no extensions clear junk from the ext area
    //if extensions then set the extensions
    if (ext[0] == '\0')
    {
		    buffer[32 * openSpace + 8] = 0x20;
        buffer[32 * openSpace + 9] = ' ';
        buffer[32 * openSpace + 10] = ' ';
    }
    else
    {
      buffer[32 * openSpace + 8] = ext[0];
      buffer[32 * openSpace + 9] = ext[1];
      buffer[32 * openSpace + 10] = ext[2];
    }
		//writing changes back to disk
		if (write_sector(sector, buffer) == -1)
		{
			printf("Problem writing to disk \n");
		}

		//writing changes back to the fat table
		for (i = 0; i < 9; i++)
		{
			write_sector(i + 1, &FAT[i * BYTES_PER_SECTOR]);
		}
	}

	//overwriting the data in that sector
	read_sector((openFLC + 33 - 2), buffer);
	//creating . and .. entries in the new directory
	buffer[0] = '.';
	for (i = 1; i < 8; i++)
	{
		buffer[i] = ' ';
	}
	buffer[8] = 0x20;
	buffer[11] = 0x10;
	buffer[27] = openFLC << 8;
	buffer[26] = openFLC;

	buffer[28] = 0;
	buffer[29] = 0 << 8;
	buffer[30] = 0 << 16;
	buffer[31] = 0 << 24;


	for (i = 2; i < 512/32; i++)
	{
		buffer[32 * i] = 0x00;
	}

	//setting the .. entry
	buffer[32] = '.';
	buffer[33] = '.';
	for (i = 34; i < 40; i++)
	{
		buffer[i] = ' ';
	}
	buffer[8 + 32] = 0x20;
	buffer[11 + 32] = 0x10;
	buffer[27 + 32] = FLC << 8;
	buffer[26 + 32] = FLC;

	buffer[28 + 32] = 0;
	buffer[29 + 32] = 0 << 8;
	buffer[30 + 32] = 0 << 16;
	buffer[31 + 32] = 0 << 24;


	for (i = 2; i < 512/32; i++)
	{
		buffer[32 * i] = 0x00;
	}

	write_sector((openFLC + 33 - 2), buffer);
	free(buffer);

}

void makeDirRoot(char* pathEnd, char* ext)
{
	int i;
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

	int sector = 19;
	char filename[8];
	int flc = -1;
	char attr;
	int entryNum;
	int exists = 0;
	int openSpace;
	int openFLC;
	int exitloop = 0;
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
				//printf("breaks from while in sector %d  0x00\n", sector);
				openSpace = entryNum;
				exitloop = 1;
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
				if (strcmp(filename, pathEnd) == 0 && attr != 0x10)
				{

					exists = 1;
					break;
				}

			}
			else if (buffer[32 * entryNum] == 0xE5)
			{
				//printf("breaks from while in sector %d in else\n", sector);
				openSpace = entryNum;
				exitloop = 1;
				break;
			}

		}
		if (exitloop == 1)
		{
			break;
		}
		sector += 1;
	}
	if (exitloop = 0)
	{
		printf("No space for directory! \n");
		return;
	}
	if (exists == 1)
	{
		printf("Directory already exists! \n");
	}
	else
	{

		int pad = 0;
		//changes FLC to whatever first open FAT entry is
		//changes filename and sets the first byte away from 0xe5
		for (i = 0; i < 8; i++)
		{
			if (pathEnd[i] == '\0' || pad == 1)
			{
				buffer[32 * openSpace + i] = ' ';
				pad = 1;
			}
			else
				buffer[32 * openSpace + i] = pathEnd[i];
			//printf("%c", buffer[32 * openSpace + i]);
		}
		//printf("\n");

		//writing changes to FAT table
		//searches whole fat table for first available 0x00 entry
		//takes the index of that FAT entry and sets it to 0xfff
		for (i = 2; i < 2847; i++)
		{
			if (get_fat_entry(i, FAT) == 0x00)
			{
				//saves first available space in FAT and saves the value then sets FAT to 0xfff
				openFLC = i;
				set_fat_entry(openFLC, 0xFFF, FAT);
				break;
			}
		}
		//sets filesize of new entry to openFLC
		buffer[32 * openSpace + 28] = 0;
		buffer[32 * openSpace + 29] = 0 << 8;
		buffer[32 * openSpace + 30] = 0 << 16;
		buffer[32 * openSpace + 31] = 0 << 24;

		//sets flc of new file
		buffer[32 * openSpace + 27] = openFLC << 8;
		buffer[32 * openSpace + 26] = openFLC;

		//setting attribute to something that's not directory
		buffer[32 * openSpace + 11] = 0x10;

    //if no extensions clear junk from the ext area
    //if extensions then set the extensions
    if (ext[0] == '\0')
    {
      buffer[32 * openSpace + 8] = 0x20;
      buffer[32 * openSpace + 9] = ' ';
      buffer[32 * openSpace + 10] = ' ';
    }
    else
    {
      buffer[32 * openSpace + 8] = ext[0];
      buffer[32 * openSpace + 9] = ext[1];
      buffer[32 * openSpace + 10] = ext[2];
    }

		//writes the edited buffer back to the floppy
		if (write_sector(sector, buffer) == -1)
		{
			printf("Problem writing to disk \n");
		}
		read_sector(sector, buffer);


		for (i = 0; i < 9; i++)
		{
			write_sector(i + 1, &FAT[i * BYTES_PER_SECTOR]);
		}
	}


	//overwriting the data in that sector
	read_sector((openFLC + 33 - 2), buffer);
	//creating . and .. entries in the new directory
	buffer[0] = '.';
	for (i = 1; i < 8; i++)
	{
		buffer[i] = ' ';
	}
	buffer[8] = 0x20;
	buffer[11] = 0x10;
	buffer[27] = openFLC << 8;
	buffer[26] = openFLC;

	buffer[28] = 0;
	buffer[29] = 0 << 8;
	buffer[30] = 0 << 16;
	buffer[31] = 0 << 24;


	for (i = 2; i < 512/32; i++)
	{
		buffer[32 * i] = 0x00;
	}

	//setting the .. entry
	buffer[32] = '.';
	buffer[33] = '.';
	for (i = 34; i < 40; i++)
	{
		buffer[i] = ' ';
	}
	buffer[8 + 32] = 0x20;
	buffer[11 + 32] = 0x10;
	buffer[27 + 32] = 0 << 8;
	buffer[26 + 32] = 0;

	buffer[28 + 32] = 0;
	buffer[29 + 32] = 0 << 8;
	buffer[30 + 32] = 0 << 16;
	buffer[31 + 32] = 0 << 24;


	for (i = 2; i < 512/32; i++)
	{
		buffer[32 * i] = 0x00;
	}

	write_sector((openFLC + 33 - 2), buffer);
	free(buffer);

}
/*
saves extension by collecting everythign after period
if there is no period then sets the ext to null
*/
char* saveExtension(char* filename)
{

	int i;
  int extCount = 0;
  int check = 0;
  char* extension;
	for (i = 0; filename[i] != '\0'; i++)
	{
    extCount++;
		if (filename[i] == '.')
		{
      extCount = 0;
      check = 1;
		}
    if (check == 1)
    {
      extension[extCount] = filename[i+1];
    }

	}
  if (check == 0)
  {
    extension[0] = '\0';
  }
  return extension;
}

void removeExtension(char* filename)
{
	//replaces the period with a null terminator
	int i;
	for(i = 0;filename[i] != '\0';i++)
	{
		if(filename[i] == '.')
		{
			filename[i] = '\0';
		}
	}
}
