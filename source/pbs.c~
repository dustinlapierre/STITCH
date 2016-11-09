/*
 * pbs.c
 *
 *  Created on: Sep 26, 2016
 *      Author: ajsebastian
 */

#include <stdio.h>
#include "pbsStruct.h"


extern FILE* FILE_SYSTEM_ID;
extern int BYTES_PER_SECTOR;
extern bootsect_t bootsector;
extern int read_sector(int sector_number, unsigned char* buffer);





void readBootSector()
{


	//finding total number of sectors
	unsigned char* buffer;
	buffer = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char));
	read_sector(0,buffer);
	int mostSigBits;
	int leastSigBits;
	int mostSigBits2;
	int leastSigBits2;


	bootsector.NumOfFats =  buffer[16];
	printf("Number of fats: %d \n", bootsector.NumOfFats);
	bootsector.SectorsPerCluster = buffer[13];
	printf("Sectors per cluster: %d \n", bootsector.SectorsPerCluster);

	leastSigBits = buffer[14];
	mostSigBits = buffer[15];
	bootsector.NumReserved = mostSigBits | leastSigBits;
	printf("Number of reserved sectors: %d  \n", bootsector.NumReserved);

	leastSigBits = buffer[17];
	mostSigBits = buffer[18];
	bootsector.NumRoot = mostSigBits | leastSigBits;
	printf("Number of root directory entries: %d  \n", bootsector.NumRoot);

	leastSigBits = buffer[22];
	mostSigBits = buffer[23];
	bootsector.SectorsPerFat = mostSigBits | leastSigBits;
	printf("Number of sectors per fat: %d  \n", bootsector.SectorsPerFat);

	leastSigBits = buffer[24];
	mostSigBits = buffer[25];
	bootsector.SectorsPerTrack = mostSigBits | leastSigBits;
	printf("Number of sectors per Track: %d  \n", bootsector.SectorsPerTrack);


	leastSigBits = buffer[26];
	mostSigBits = buffer[27];
	bootsector.NumHeads= mostSigBits | leastSigBits;
	printf("Number of heads: %d  \n", bootsector.NumHeads);

	bootsector.BootSig = buffer[38];
	printf("Boot signature in hex: 0x%x \n", bootsector.BootSig);

	leastSigBits = buffer[39] ;
	leastSigBits2 = buffer[40] ;
	mostSigBits2 = buffer[41] ;
	mostSigBits = buffer[42] ;

	bootsector.VolumeId = mostSigBits | mostSigBits2 | leastSigBits2 | leastSigBits << 24;
	printf("Volume ID in hex: 0x%x \n",bootsector.VolumeId);

}





