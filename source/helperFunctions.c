#include "helperFunctions.h"

//returns array of pointers to tokens
char** parsePath(char* pathname)
{
	//buffer size
	int bufsize = 24;
	//holds arg array position
	int position = 0;
	//holds each single token
	char* token;

	//allocating buffer
	char** directoryList = malloc(bufsize * sizeof(char*));
	if (!directoryList)
	{
		perror("Error allocating space\n");
		exit(EXIT_FAILURE);
	}

	//separating each segment of the line into tokens based on deliminators
	token = strtok(pathname, "/");
	while (token != NULL)
	{
		directoryList[position] = token;
		position++;

		//if buffer size is exceeded the buffer size is increased
		if (position >= bufsize)
		{
			bufsize *= 2;
			directoryList = realloc(directoryList, bufsize * sizeof(char*));
			if (!directoryList)
			{
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

int main(int argc,char* argv[])
{
	char path[] = "/home/friend/lib";
	char** pathArray;
	pathArray = parsePath(path);
	int i = 0;
	for(i = 0;pathArray[i] != NULL;i++)
	{
		printf("%s \n", pathArray[i]);
	}

	free(pathArray);

	return 0;
}
