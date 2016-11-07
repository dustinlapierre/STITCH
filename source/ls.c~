/*
BY AJ Sebastian and Dustin Lapierre
11/2/16
LS.c LS functionality for the stitch shell
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include "fatSupport.h"
#include "helperFunctions.h"
#include <stdbool.h>

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;

void printRoot(int*);
void printOther(int*);

//array of pointers for path seperation
char** parsePath(char*);

char* concat(const char *s1, const char *s2) {
	char *result = malloc(strlen(s1) + strlen(s2) + 1); //+1 for the zero-terminator
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

int main(int argc, char* argv[]) {
//argument testing
	if (argc > 2) {
		printf("Too many arguments!");
		exit(0);
	}
  //open the floppy image
	FILE_SYSTEM_ID = fopen("floppy1", "r+");

	if (FILE_SYSTEM_ID == NULL) {
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}
  //declaring variables related to shared memory
	int testFLC;
	char* testPath;
	int* holdFLC;
	char* holdPath;
	int i;
	int shm_id;
	int shm_id2;
	key_t key;
	key = 9876;

	//request access to shared memory
	shm_id = shmget(key, 128 * sizeof(char), 0666);
	if (shm_id < 0) {
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//connect this process to shared memory
	char *path = shmat(shm_id, NULL, 0);
	if (path == (char *) -1) {
		perror("Error gaining access to shared memory");
		exit(1);
	}

	key = 9880;

	//request access to shared memory
	shm_id2 = shmget(key, sizeof(int), 0666);
	if (shm_id < 0) {
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//connect this process to shared memory
	int* currentFLC = shmat(shm_id2, NULL, 0);
	if (currentFLC == (int *) -1) {
		perror("Error gaining access to shared memory");
		exit(1);
	}
  //saving FLC and path for when we CD
	holdFLC = currentFLC;
	testFLC = *holdFLC;
	testPath = path;

	char *argumentPath = argv[1];
	char storedPath[128];

	//Begin printing of the LS choosing different sectors to LS
	if (argc == 1) {
    //either prints root or whatever sector is currently in the FLC
		if (*currentFLC == 0) {
			printRoot(currentFLC);
		} else {
			printOther(currentFLC);
		}
	}

	if (argc == 2) {

		//changes arguments to uppercase
		for (i = 0; argumentPath[i] != '\0'; i++) {
			argumentPath[i] = toupper(argumentPath[i]);
			storedPath[i] = argumentPath[i];
		}
		storedPath[i + 1] = '\0';
    //seperates path into
		char** pathArray = parsePath(argumentPath);
		char* finalEntry;
		int pathLength;

		for (pathLength = 0; pathArray[pathLength] != NULL; pathLength++) {
			//just counts the # of paths
		}
    // if there are arguments but it's just a file/ directory name then we'll just call LS on the file / DIR
		if (pathLength < 2) {
			unsigned char* buffer;
			buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
			if (*currentFLC == 0)
      {
        read_sector(19,buffer);
      }
      else
      {
			read_sector(*currentFLC, buffer);
      }

			char* newPath;
			char filename[8];
			char attr;
			int flc;

			int entryNum;

			for (entryNum = 0; entryNum < 512 / 32; entryNum++) {
				if (buffer[32 * entryNum] == 0x00) {
					break;
				}

				if (buffer[32 * entryNum] != 0xE5
						&& buffer[32 * entryNum + 11] != 0x0f) {
					for (i = 0; i < 8; i++) {
						if (buffer[32 * entryNum + i] != ' ') {
							filename[i] = buffer[32 * entryNum + i];

						} else {
							filename[i] = '\0';
						}
					}
					attr = buffer[32 * entryNum + 11];
					if (strcmp(filename, pathArray[0]) == 0) {

						*currentFLC = (buffer[32 * entryNum + 27] << 8)
								+ buffer[32 * entryNum + 26];
                break;
					}
				}
        }
				if (buffer[32 * entryNum + 11] == 0x10) {
					printOther(currentFLC);
				} else {
					printf("Name		Type		File Size		Flc 	\n");
					for (i = 0; i < 8; i++) {
						printf("%c", buffer[32 * entryNum + i]);
					}
					if (buffer[32 * entryNum + 8] != 0x20) {
						printf(".");
					} else {
						printf(" ");
					}
					for (i = 8; i < 11; i++) {
						printf("%C", buffer[32 * entryNum + i]);
					}

					if (buffer[32 * entryNum + 11] == 0x10) {
						printf(" DIR	");
					} else {
						printf(" FILE	");
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


		} else {

			 char* newPath = "\0";

			 for(i= 0; pathArray[i] != '\0';i++)
			 {
			 if (pathArray[i + 1] == NULL)
			 {
			 finalEntry = pathArray[i];
			 pathArray[i] = NULL;

			 }
			 }
       for (i= 0; pathArray[i] != '\0';i++)
       {
         newPath = concat(newPath, "/");
         newPath = concat(newPath, pathArray[i]);

       }
       //if the pathname starts with home then we can read from root
       if (strcmp(newPath, "/HOME") == 0)
       {
         unsigned char* buffer;
   			buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
         read_sector(19,buffer);
         char filename[8];
   			char attr;
   			int flc;



   			int entryNum;

   			for (entryNum = 0; entryNum < 512 / 32; entryNum++) {
   				if (buffer[32 * entryNum] == 0x00) {
   					break;
   				}

   				if (buffer[32 * entryNum] != 0xE5
   						&& buffer[32 * entryNum + 11] != 0x0f) {
   					for (i = 0; i < 8; i++) {
   						if (buffer[32 * entryNum + i] != ' ') {
   							filename[i] = buffer[32 * entryNum + i];

   						} else {
   							filename[i] = '\0';
   						}
   					}
   					attr = buffer[32 * entryNum + 11];
   					if (strcmp(filename, finalEntry) == 0) {

   						*currentFLC = (buffer[32 * entryNum + 27] << 8)
   								+ buffer[32 * entryNum + 26];
                   break;
   					}
   				}
           }
   				if (buffer[32 * entryNum + 11] == 0x10) {
   					printOther(currentFLC);
   				} else {
   					printf("Name		Type		File Size		Flc 	\n");
   					for (i = 0; i < 8; i++) {
   						printf("%c", buffer[32 * entryNum + i]);
   					}
   					if (buffer[32 * entryNum + 8] != 0x20) {
   						printf(".");
   					} else {
   						printf(" ");
   					}
   					for (i = 8; i < 11; i++) {
   						printf("%C", buffer[32 * entryNum + i]);
   					}

   					if (buffer[32 * entryNum + 11] == 0x10) {
   						printf(" DIR	");
   					} else {
   						printf(" FILE	");
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
       else
       {


         //calls CD to CD to the appropriate directory

         newPath = concat(newPath, '\0');
         int pid = fork();
       	if(pid == -1)
       	{
       		perror("Error creating process\n");
       	}
       	else if(pid == 0)
       	{
       		if(execvp("./commands/cd", newPath) == -1)
       		{
       			perror("Error loading program into memory");
       			exit(EXIT_FAILURE);
       		}


       	//waiting until child is finished running
       	pid_t waitpid;
       	waitpid = wait(NULL);
       	if(waitpid == -1)
       	{
       		perror("Error on wait");
       		exit(0);
       	}

        int flc;
        int i;

        unsigned char* buffer;
       buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

        read_sector(*currentFLC,buffer);
        char filename[8];
       char attr;


       printf("%d \n", *currentFLC);

       int entryNum;

       for (entryNum = 0; entryNum < 512 / 32; entryNum++) {
         if (buffer[32 * entryNum] == 0x00) {
           break;
         }

         if (buffer[32 * entryNum] != 0xE5
             && buffer[32 * entryNum + 11] != 0x0f) {
           for (i = 0; i < 8; i++) {
             if (buffer[32 * entryNum + i] != ' ') {
               filename[i] = buffer[32 * entryNum + i];

             } else {
               filename[i] = '\0';
             }
           }
           attr = buffer[32 * entryNum + 11];
           if (strcmp(filename, finalEntry) == 0) {

             *currentFLC = (buffer[32 * entryNum + 27] << 8)
                 + buffer[32 * entryNum + 26];
                  break;
           }
         }
          }

         if (buffer[32 * entryNum + 11] == 0x10) {
           printOther(currentFLC);
         } else {
           printf("Name		Type		File Size		Flc 	\n");
           for (i = 0; i < 8; i++) {
             printf("%c", buffer[32 * entryNum + i]);
           }
           if (buffer[32 * entryNum + 8] != 0x20) {
             printf(".");
           } else {
             printf(" ");
           }
           for (i = 8; i < 11; i++) {
             printf("%C", buffer[32 * entryNum + i]);
           }

           if (buffer[32 * entryNum + 11] == 0x10) {
             printf(" DIR	");
           } else {
             printf(" FILE	");
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
}
}
}

  //resets these variables after calls to CD
	*currentFLC = testFLC;
	*path = *testPath;

	//disconnect from shared memory
	int del_shm = shmdt(path);
	if (del_shm == -1) {
		perror("Error detaching from shared memory");
		exit(1);
	}

	//disconnect from shared memory
	del_shm = shmdt(currentFLC);
	if (del_shm == -1) {
		perror("Error detaching from shared memory");
		exit(1);
	}

	return 0;
}

void printRoot(int* currentFLC) {
	printf("Name		Type		File Size		Flc 	\n");
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int filesize;
	int i;

	read_sector(19, buffer); //read the first root sector

	int entryNum;
	for (entryNum = 0; entryNum < 512 / 32; entryNum++) {

		if (buffer[32 * entryNum] == 0x00) {

			break;
		}

		if (buffer[32 * entryNum] != 0xE5
				&& buffer[32 * entryNum + 11] != 0x0f) {

			//prints filename
			for (i = 0; i < 8; i++) {
				printf("%c", buffer[32 * entryNum + i]);
			}

			//prints the . before the extension if necessary
			if (buffer[32 * entryNum + 8] != 0x20) {
				printf(".");
			}

			//prints the file extension

			for (i = 8; i < 11; i++) {
				printf("%C", buffer[32 * entryNum + i]);
			}

			//testing directory vs files
			if (buffer[32 * entryNum + 11] == 0x10) {
				printf("	DIR	");
			} else {
				printf(" FILE	");
			}
			int filesize = buffer[32 * entryNum + 31]
					| buffer[32 * entryNum + 30] | buffer[32 * entryNum + 29]
					| buffer[32 * entryNum + 28];

			printf("	%d			", filesize);

			//printing the FLC least sig bit first
			printf("%d",
					buffer[32 * entryNum + 27] | buffer[32 * entryNum + 26]);
			printf("\n");
		}

	}

}

char** parsePath(char* pathname) {
	//buffer size
	int bufsize = 24;
	//holds arg array position
	int position = 0;
	//holds each single token
	char* token;

	//allocating buffer
	char** directoryList = malloc(bufsize * sizeof(char*));
	if (!directoryList) {
		perror("Error allocating space\n");
		exit(EXIT_FAILURE);
	}

	//separating each segment of the line into tokens based on deliminators
	token = strtok(pathname, "/");
	while (token != NULL) {
		directoryList[position] = token;
		position++;

		//if buffer size is exceeded the buffer size is increased
		if (position >= bufsize) {
			bufsize *= 2;
			directoryList = realloc(directoryList, bufsize * sizeof(char*));
			if (!directoryList) {
				perror("Error allocating space\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, "/");
	}

	//setting the last position after arguments to NULL
	directoryList[position] = NULL;
	return directoryList;
}

void printOther(int* currentFLC) {

	printf("Name		Type		File Size		Flc 	\n");
	unsigned char* buffer;
	buffer = malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	int filesize;
	int i;

	read_sector((33 + *currentFLC - 2), buffer); //read the first root sector

	int entryNum;
	for (entryNum = 0; entryNum < 512 / 32; entryNum++) {

		if (buffer[32 * entryNum] == 0x00) {

			break;
		}
    //prints filename and extension
		if (buffer[32 * entryNum] != 0xE5
				&& buffer[32 * entryNum + 11] != 0x0f) {
			for (i = 0; i < 8; i++) {
				printf("%c", buffer[32 * entryNum + i]);
			}
			if (buffer[32 * entryNum + 8] != 0x20) {
				printf(".");
			} else {
				printf(" ");
			}
			for (i = 8; i < 11; i++) {
				printf("%C", buffer[32 * entryNum + i]);
			}
      //prints filetype
			if (buffer[32 * entryNum + 11] == 0x10) {
				printf(" DIR	");
			} else {
				printf(" FILE	");
			}
			int filesize = buffer[32 * entryNum + 31]
					| buffer[32 * entryNum + 30] | buffer[32 * entryNum + 29]
					| buffer[32 * entryNum + 28];

			printf("	%d			", filesize);

			//printing the FLC least sig bit first
			printf("%d",
					buffer[32 * entryNum + 27] | buffer[32 * entryNum + 26]);
			printf("\n");
			//printf("%02x %02x\n", buffer[32*entryNum], buffer[32*entryNum + 11]);
		}

	}

}
