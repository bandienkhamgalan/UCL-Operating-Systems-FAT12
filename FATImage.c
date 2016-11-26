#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "FATImage.h"
#include "Helpers.h"

FATImage* FATImage_Make()
{
	FATImage* new = calloc(1, sizeof(FATImage));
	assert(new != NULL);

	new->fileChains = calloc(16, sizeof(IndexChain));
	assert(new->fileChains != NULL);
	new->fileChainsCapacity = 8;
	new->fileChainsLength = 0;

	return new;
}

FATImage* FATImage_Initialize(char* imageFile)
{
	int currentError;
	int fileDescriptor = open(imageFile, O_RDWR);
	if(fileDescriptor == -1)
	{
		currentError = errno;
		printf("dos_scandisk: %s\n", strerror(currentError));
		return NULL;
	}

	struct stat fileInfo;
	if(fstat(fileDescriptor, &fileInfo) == -1)
	{
		currentError = errno;
		printf("dos_scandisk: error reading image file: %s\n", strerror(currentError));
		return NULL;
	}

	size_t fileSize = fileInfo.st_size;
	uint8_t* image = mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileDescriptor, 0);
	if(image == MAP_FAILED)
	{
		currentError = errno;
		printf("dos_scandisk: error mapping image file: %s\n", strerror(currentError));
		return NULL;
	}

	FATImage* new = FATImage_Make();
	new->imageSize = fileSize;
	new->image = image;
	new->imageFileDescriptor = fileDescriptor;
	return new;
}

void FATImage_Free(FATImage* toFree)
{
	assert(toFree != NULL);
	munmap(toFree->image, toFree->imageSize);	
	close(toFree->imageFileDescriptor);
	for(size_t index = 0 ; index < toFree->fileChainsLength ; ++index)
		IndexChain_FreeNodes(toFree->fileChains + index);
	
	free(toFree->fileChains);
	free(toFree->clusters);
	free(toFree);
}

long FATImage_ReadLittleEndian(FATImage* disk, size_t sector, size_t offset, size_t length)
{
	assert(disk != NULL);
	long toReturn = 0;
	uint8_t* end = &(disk->image[sector * disk->information.sectorSize + offset + length - 1]);
	for(size_t index = 0 ; index < length ; ++index)
	{
		toReturn = toReturn << 8 | *end;
		--end;
	}
	return toReturn;
}

void FATImage_UpdateDiskInformation(FATImage* disk)
{
	assert(disk != NULL);
	FATDiskInformation* info = &(disk->information);
	info->sectorSize = FATImage_ReadLittleEndian(disk, 0, 11, 2);
	info->sectorCount = FATImage_ReadLittleEndian(disk, 0, 19, 2);
	info->sectorsPerCluster = FATImage_ReadLittleEndian(disk, 0, 13, 1);

	size_t reservedSectors = FATImage_ReadLittleEndian(disk, 0, 14, 2);
	info->fileAllocationTableStartSector = reservedSectors;
	info->fileAllocationTableSectorCount = ceil((info->sectorCount - reservedSectors - info->rootDirectorySectorCount) * 3.0 / 2.0 / info->sectorSize);
	info->fileAllocationTableCopies = FATImage_ReadLittleEndian(disk, 0, 16, 1);
	
	info->rootDirectoryStartSector = info->fileAllocationTableStartSector + info->fileAllocationTableCopies * info->fileAllocationTableSectorCount;
	info->rootDirectorySectorCount = FATImage_ReadLittleEndian(disk, 0, 17, 2) * 32 / info->sectorSize;

	info->dataSectorStartSector = info->rootDirectoryStartSector + info->rootDirectorySectorCount;
	info->dataSectorCount = info->sectorCount - info->dataSectorStartSector;
}

IndexChain* FATImage_GetNewFileChain(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->fileChains != NULL);
	
	if(disk->fileChainsLength >= disk->fileChainsCapacity)
	{
		// realloc
		disk->fileChains = realloc(disk->fileChains, 2 * disk->fileChainsCapacity * sizeof(IndexChain));
		memset(disk->fileChains + disk->fileChainsCapacity, 0, disk->fileChainsCapacity * sizeof(IndexChain) / sizeof(unsigned char));
		disk->fileChainsCapacity *= 2;
	}

	disk->fileChainsLength += 1;
	return disk->fileChains + (disk->fileChainsLength - 1);
}

void FATImage_ReadClusterIndexSequenceAndCreateFileChains(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	for(size_t index = 2; index < disk->clustersLength ; ++index)
	{
		uint16_t value = disk->clusters[index].rawTableValue;
		if(value == 0x00)
			disk->clusters[index].status = Unused;
		else if(value >= 0xFF0 && value <= 0xFF6)
			disk->clusters[index].status = Reserved;
		else if(value == 0xFF7)
			disk->clusters[index].status = Bad;
		else
		{
			// part of file
			if(disk->clusters[index].fileChain != NULL)
			{
				// already traversed
				continue;
			}
				
			IndexChain* newChain = FATImage_GetNewFileChain(disk);

			size_t currentIndex = index;
			while(true)
			{
				uint16_t currentValue = disk->clusters[currentIndex].rawTableValue;
				if(currentValue >= 0xFF8 && currentValue <= 0xFFF)
				{
					IndexChain_Append(newChain, currentIndex);
					disk->clusters[currentIndex].fileChain = newChain;
					disk->clusters[currentIndex].status = FileLast;

					// last file in chain, break
					break;
				}
				else if(currentValue >= 2 && currentValue < 2 + disk->information.dataSectorCount)
				{
					IndexChain_Append(newChain, currentIndex);
					disk->clusters[currentIndex].fileChain = newChain;
					disk->clusters[currentIndex].status = File;

					// file continues, follow cluster index chain
					currentIndex = currentValue;
				}
				else
				{
					printf("encountered error traversing index chain in file allocation table...\n");
					break;
				}
			}
		}
	}

	printf("Found %zd files...\n", disk->fileChainsLength);
	for(size_t index = 0 ; index < disk->fileChainsLength ; ++index)
	{
		printf("File with %zd clusters:\n", disk->fileChains[index].length);
		if(disk->fileChains[index].length > 0)
		{
			IndexChainNode* node = disk->fileChains[index].head;
			printf("\t%zd", node->index);
			node = node->next;
			while(node)
			{
				printf(" > %zd", node->index);
				node = node->next;
			}
		}

		printf("\n");
	}
}

void FATImage_ReadDirectoryEntries(FATImage* disk)
{
	
}

void FATImage_ReadFileAllocationTable(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters == NULL);

	/* Retrieve values */
	size_t sectors = disk->information.sectorCount;
	uint16_t* tableIndices = calloc(disk->information.sectorCount, sizeof(uint16_t));
	assert(tableIndices != NULL);
	Read12ByteLittleEndianSequence(disk->image + 512, sectors * 3 / 2, tableIndices, sectors);

	disk->clusters = calloc(sectors, sizeof(Cluster));
	assert(disk->clusters != NULL);
	disk->clustersLength = sectors;

	for(size_t index = 0 ; index < sectors ; ++index)
		disk->clusters[index].rawTableValue = tableIndices[index];
	free(tableIndices);

	FATImage_ReadClusterIndexSequenceAndCreateFileChains(disk);
}