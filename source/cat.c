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

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;
unsigned char* FAT;


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

	FILE_SYSTEM_ID = fopen("floppy1", "r+");
	if (FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}

  //store FAT table
	FAT = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char) * 9);
  int i;
	for(i = 0;i < 9; i++)
	{
		read_sector(i+1, &FAT[i * BYTES_PER_SECTOR]);
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

  //CAT functionality

  for (i = 0; argument[i] != '\0'; i++) {
    argument[i] = toupper(argument[i]);
  }

  argument[i + 1] = '\0';
  char** pathArray = parsePath(argument);
  int pathLength;

  for (pathLength = 0; pathArray[pathLength] != NULL; pathLength++) {
    //just counts the # of paths
  }
  //save the last token in a variable then drop it

  for (i = 0; pathArray[i + 1] != '\0'; i++);
  pathEnd = pathArray[i];
  pathArray[i] = '\0';

  //build path for cd
  char cdPath[1024];
  if (pathLength > 1)
  {
    strcat(cdPath, "/");


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
    if (execlp("./commands/cd", "cd", cdPath,(char *) NULL) == -1)
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
  /*
  printf("%s \n", cdPath);
  printf("%s \n", pathEnd);
  printf("%i \n", *currentFLC);*/
  printFile(currentFLC, pathEnd);
  }
  else
  {
  printFile(currentFLC, pathEnd);
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

void printFile(int* currentFLC,char* pathEnd)
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

  for (entryNum = 0; entryNum < 512 / 32; entryNum++) {
    if (buffer[32 * entryNum] == 0x00) {
      printf("Error: file searched does not exist. \n");
      return;
    }

    if (buffer[32 * entryNum] != 0xE5 && buffer[32 * entryNum + 11] != 0x0f)
     {


      for (i = 0; i < 8; i++) {
        if (buffer[32 * entryNum + i] != ' ') {
          filename[i] = buffer[32 * entryNum + i];
          //printf("%c" , filename[i]);
        } else {
          filename[i] = '\0';
        }
      }

      if (strcmp(filename, pathEnd) == 0)
      {

        *currentFLC = buffer[32 * entryNum + 27] | buffer[32 * entryNum + 26];
            break;
      }
    }
    }



    if (buffer[32 * entryNum + 11] == 0x10)
    {
      printf("You cannot CAT a directory! \n");
    }
    else
    {
       int flc = *currentFLC;
       int filesize = buffer[32 * entryNum + 31]
 					| buffer[32 * entryNum + 30]
 					| buffer[32 * entryNum + 29]
 					| buffer[32 * entryNum + 28];
      do
      {
        read_sector(33 + flc -2, buffer);
        flc = get_fat_entry(flc, FAT);

        for (i=0; i < 512; i++)
        {
          printf("%c", buffer[i]);

        }


      }while(get_fat_entry(flc, FAT) != 0xfff);
      printf("\n");
    }


}
