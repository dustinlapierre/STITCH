#include <stdio.h>
#include <stdlib.h>
#include "fatSupport.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;

//return of 1 indicates the range is valid
int checkRange(int x, int y)
{
	if(x > y)
	{
		printf("Error: invalid range! \n");
		return 0;
	}
	else if(x < 2)
	{
		printf("Error: invalid range! \n");
		return 0;
	}
	else
	{
		return 1;
	}
}

unsigned char* readFAT12Table(int table)
{
	unsigned char* buffer;
	buffer = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char) * 9);
	int i;


	for(i = 0;i < 9; i++)
	{
		read_sector(i+1, &buffer[i * BYTES_PER_SECTOR]);
	}

	return buffer;
}

void pfe(int x, int y)
{
	if(checkRange(x,y))
	{
		unsigned char* buffer = readFAT12Table(1);
		int i;
		for(i = x;i <= y;i++)
		{
			printf("%X \n", get_fat_entry(i, buffer));
			//print fat entries
		}
	}

}

int main(int argc, char* argv[])
{
	int arg1 = 0;
	int arg2 = 0;

	//converting arguments into ints
	if(argv[1] == NULL || argv[2] == NULL)
	{
		printf("Error missing arguments \n");
		exit(1);
	}
	else
	{
		arg1 = atoi(argv[1]);
		arg2 = atoi(argv[2]);
	}

   // Use this for an image of a floppy drive
   FILE_SYSTEM_ID = fopen("floppy1", "r+");

   if (FILE_SYSTEM_ID == NULL)
   {
      printf("Could not open the floppy drive or image.\n");
      exit(1);
   }

   pfe(arg1,arg2);

   return 0;
}
