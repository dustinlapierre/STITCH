/*
 * pbsStruct.h
 *
 *  Created on: Sep 26, 2016
 *      Author: ajsebastian
 */


typedef struct bootsect
{
	unsigned char Name[8];
	int BytesPerSector;
	int SectorsPerCluster;
	int NumOfFats;
	int NumReserved;
	int NumRoot;
	int SectCount;
	int SectorsPerFat;
	int SectorsPerTrack;
	int NumHeads;
	int BootSig;
	long int VolumeId;
	char* VolumeLabel;
	char* FileSysType;
	//boot sig in hex/ volume id in hex/ volume label and file system type


} __attribute__ ((packed)) bootsect_t;

