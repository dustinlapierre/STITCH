/*
Authors: Dustin Lapierre, Albert Sebastian
Class: CSI-385-02
Assignment: FAT12 Filesystem
Created: 10.14.2016
Shell in C
A shell that creates processes and executes commands

Certification of Authenticity:
I certify that this assignment is entirely my own work.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>

//defining constants
const int TRUE = 1;
const int FALSE = 0;

char *getUserInput();
char **parseUserInput(char* line);
int executeCommand(char** args, char* path, int* flc);
void main_loop();

int main(int argc,char* argv[])
{
	//set floppy environment variable
	setenv("CURRENT_FLOPPY", "floppy1", 1);

	//starts the main loop
	main_loop();

	return EXIT_SUCCESS;
}

void main_loop()
{
	int shm_id;
	key_t key;
	key = 9876;

	//allocate shared memory for path
	shm_id = shmget(key, 128*sizeof(char), IPC_CREAT | 0666);
	if(shm_id < 0)
	{
		perror("Error gaining allocating shared memory");
		exit(1);
	}

	//connect to shared memory
	char *path = shmat(shm_id, NULL, 0);
	if(path == (char *) -1)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	key = 9880;
	int shm_id2;

	//allocate shared memory for flc
	shm_id2 = shmget(key, sizeof(int), IPC_CREAT | 0666);
	if(shm_id2 < 0)
	{
		perror("Error gaining allocating shared memory");
		exit(1);
	}

	//connect to shared memory
	int* flc = shmat(shm_id2, NULL, 0);
	if(flc == (int *) -1)
	{
		perror("Error gaining access to shared memory");
		exit(1);
	}

	//path initialized to root
	strcpy(path, "/HOME");

	//flc initialized to root
	*flc = 0;

	//used for exit condition
	int status = TRUE;

	//stores user input
	char* line;

	//argument list
	char **args;

	do
	{
		//1. Prompt for input
		printf("> ");

		//2. Read input
		line = getUserInput();

		//3. Parse input
		args = parseUserInput(line);

		//4. Execute command
		status = executeCommand(args, path, flc);

		free(line);
		free(args);
	}
	while(status);

	//disconnect from shared memory
	int del_shm = shmdt(path);
	if(del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}

	//disconnect from shared memory
	del_shm = shmdt(flc);
	if(del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}

	//deleting shared memory
	shmctl(shm_id, IPC_RMID, NULL);
	shmctl(shm_id2, IPC_RMID, NULL);

}

char **parseUserInput(char* line)
{
	//buffer size
	int bufsize = 24;
	//holds arg array position
	int position = 0;
	//list of possible deiliminators
	char *delim = " \t\r\n";
	//holds each single token
	char *token;

	//allocating buffer
	char **args = malloc(bufsize * sizeof(char*));
	if (!args)
	{
		perror("Error allocating space\n");
		exit(EXIT_FAILURE);
	}

	//separating each segment of the line into tokens based on deliminators
	token = strtok(line, delim);
	while (token != NULL)
	{
		args[position] = token;
		position++;

		//if buffer size is exceeded the buffer size is increased
		if (position >= bufsize)
		{
			bufsize *= 2;
			args = realloc(args, bufsize * sizeof(char*));
			if (!args)
			{
				perror("Error allocating space\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, delim);
	}

	//setting the last position after arguments to NULL
	args[position] = NULL;
	return args;


}

char *getUserInput()
{
	//stores user input
	char *line = NULL;
	//used to test how many bytes getline reads in
	int bytes_read;

	//Decalaring buffer variable
	size_t bufsize = 0;

	//Getting user input
	bytes_read = getline(&line, &bufsize, stdin);
	if(bytes_read == -1)
	{
		perror("Error reading input");
	}

	//returning input
	return line;
}

//string concatenation
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

//Uses arguments to create a process and load a program into memory
//Notifies parent when program ends
int executeCommand(char** args, char* path, int* flc)
{

	//builtin functions
	if(args[0] == NULL)
	{
		//nothing is entered
		return TRUE;
	}
	else if(strcmp(args[0], "exit") == 0)
	{
		//returns FALSE to tell status to quit
		return FALSE;
	}
	else if(strcmp(args[0], "about") == 0)
	{
		//command that prints about info
		printf("STITCH 2.0\n");
		printf("Created by Dustin Lapierre and Albert Sebastian\n");
		printf("Shell That Is Thrilling and Considerably Helpful\n");
		printf("Copyright 10/14/2016\n");
		return TRUE;
	}
	else if(strcmp(args[0], "help") == 0)
	{
		//command that prints command list
		printf("STITCH 2.0 Commands:\n");
		printf("cd\n");
		printf("Changes directory, absolute or relative path accepted\n");
		printf("cat\n");
		printf("Prints file contents, absolute or relative path accepted\n");
		printf("lilo\n");
		printf("Changes floppy, number between 1-3 accepted\n");
		printf("ls\n");
		printf("List elements of directory, absolute or relative path accepted\n");
		printf("pwd\n");
		printf("Print working directory, no arguments\n");
		printf("rm\n");
		printf("Removes a file, absolute or relative path accepted\n");
		printf("pbs\n");
		printf("Prints boot sector, no arguments\n");
		printf("df\n");
		printf("Prints memory info, no arguments\n");
		printf("pfe\n");
		printf("Prints fat entries, two numbers between 2-2847\n");
		printf("touch\n");
		printf("Creates a file, absolute or relative path accepted\n");
		printf("about\n");
		printf("Prints about info, no arguments\n");
		printf("exit\n");
		printf("Quits shell, no arguments\n");
		printf("help\n");
		printf("Prints all commands, no arguments\n");
		return TRUE;
	}
	else if(strcmp(args[0], "lilo") == 0)
	{
		//command to change floppy disks
		if(atoi(args[1]) < 4 && atoi(args[1]) > 0)
		{
			setenv("CURRENT_FLOPPY", concat("floppy",args[1]), 1);
			strcpy(path, "/HOME");
			*flc = 0;
			printf("Changed to %s\n", getenv("CURRENT_FLOPPY"));
		}
		else
		{
			printf("invalid floppy number!\n");
		}
		return TRUE;
	}
	//creating and executing a new process
	int pid = fork();
	if(pid == -1)
	{
		perror("Error creating process\n");
	}
	else if(pid == 0)
	{
		if(execvp(concat("./commands/", args[0]), args) == -1)
		{
			perror("Error loading program into memory");
			exit(EXIT_FAILURE);
		}
	}

	//waiting until child is finished running
	pid_t waitpid;
	waitpid = wait(NULL);
	if(waitpid == -1)
	{
		perror("Error on wait");
		exit(0);
	}

	return TRUE;
}
