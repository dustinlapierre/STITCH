#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

int main(int argc, char* argv[])
{
	int shm_id;
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

	//print out path from shared memory
	int i;
	for(i = 0;path[i] != '\0';i++ )
	{
		printf("%c", path[i]);
	}

	printf("\n");

	//disconnect from shared memory
	int del_shm = shmdt(path);
	if(del_shm == -1)
	{
		perror("Error detaching from shared memory");
		exit(1);
	}

	return 0;
}
