#include <stdio.h>
#include "FATImage.h"

#define NONE 0
#define INFO 1
#define DETAIL 2
#define DEBUG 3
int log_level = NONE;

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		FATImage* disk = FATImage_Initialize(argv[1]);
		if(disk)
		{
			FATImage_UpdateDiskInformation(disk);
			FATImage_ReadFileAllocationTable(disk);
			FATImage_ReadDirectoryEntries(disk);

			FATImage_PrintUnreferencedClusters(disk);
			FATImage_PrintLostFiles(disk);
			FATImage_RecoverLostFiles(disk);
			FATImage_PrintSizeInconsistencies(disk);
			FATImage_ResolveSizeInconsistencies(disk);
			FATImage_SaveChanges(disk);

			FATImage_Free(disk);
		}
	}
	else
	{
		printf("usage: dos_scandisk image_file\n");
	}

	return 0;
}