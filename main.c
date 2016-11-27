#include <stdio.h>
#include "FATImage.h"

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		FATImage* disk = FATImage_Initialize(argv[1]);
		//printf("successfully mapped %zd bytes to fd %d\n", disk->imageSize, disk->imageFileDescriptor);
		if(disk)
		{
			FATImage_UpdateDiskInformation(disk);
			FATImage_ReadFileAllocationTable(disk);
			FATImage_ReadDirectoryEntries(disk);
			FATImage_PrintUnreferencedClusters(disk);
			FATImage_PrintLostFiles(disk);
			FATImage_Free(disk);
		}
	}
	else
	{
		printf("usage: dos_scandisk image_file\n");
	}

	return 0;
}