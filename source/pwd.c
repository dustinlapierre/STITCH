#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

int main(int argc, char* argv[])
{
	//all of this needs error checking
	int shm_id;
	key_t key;
	key = 9876;

	//request access to shared memory
	shm_id = shmget(key, 128*sizeof(char), 0666);
	char *path = shmat(shm_id, NULL, 0);

	//print out path from shared memory
	int i;
	for(i = 0;i < sizeof(path);i++ )
	{
		printf("%c", path[i]);
	}

	printf("\n");

	int del_shm = shmdt(path);

	return 0;
}
