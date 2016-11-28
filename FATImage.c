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
#include <stdlib.h>
#include "FATImage.h"
#include "Helpers.h"

#define NONE 0
#define INFO 1
#define DETAIL 2
#define DEBUG 3
extern int log_level;
#define LOG(level, ...) if (level <= log_level) printf(__VA_ARGS__);

#define LOG_LEVEL NONE

FATImage* FATImage_Make()
{
	FATImage* new = calloc(1, sizeof(FATImage));
	assert(new != NULL);

	new->clusterChains = calloc(16, sizeof(ClusterChain));
	assert(new->clusterChains != NULL);
	new->clusterChainsCapacity = 16;
	new->clusterChainsLength = 0;

	new->directoryEntries = calloc(16, sizeof(DirectoryEntry));
	assert(new->directoryEntries != NULL);
	new->directoryEntriesCapacity = 16;
	new->directoryEntriesLength = 0;

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
	uint8_t* image = mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
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
	LOG(DETAIL, "successfully mapped %zd bytes to fd %d\n", new->imageSize, new->imageFileDescriptor);
	return new;
}

void FATImage_Free(FATImage* toFree)
{
	assert(toFree != NULL);
	munmap(toFree->image, toFree->imageSize);	
	close(toFree->imageFileDescriptor);

	for(size_t index = 0 ; index < toFree->clusterChainsLength ; ++index)
		ClusterChain_FreeNodes(toFree->clusterChains + index);
	free(toFree->clusterChains);

	for(size_t index = 0 ; index < toFree->directoryEntriesLength ; ++index)
	{
		free(toFree->directoryEntries[index].filename);
		free(toFree->directoryEntries[index].extension);
	}
	free(toFree->directoryEntries);
	
	free(toFree->clusters);
	free(toFree);
}

long FATImage_ReadLittleEndian(FATImage* disk, size_t sector, size_t offset, size_t length)
{
	assert(disk != NULL);
	return NumberFrom8ByteLittleEndianSequence(&(disk->image[sector * disk->information.sectorSize + offset]), length);
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

ClusterChain* FATImage_GetNewFileChain(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusterChains != NULL);
	
	if(disk->clusterChainsLength >= disk->clusterChainsCapacity)
	{
		// realloc
		disk->clusterChains = realloc(disk->clusterChains, 2 * disk->clusterChainsCapacity * sizeof(ClusterChain));
		assert(disk->clusterChains != NULL);
		memset(disk->clusterChains + disk->clusterChainsCapacity, 0, disk->clusterChainsCapacity * sizeof(ClusterChain) / sizeof(unsigned char));
		disk->clusterChainsCapacity *= 2;
	}

	disk->clusterChainsLength += 1;
	return disk->clusterChains + (disk->clusterChainsLength - 1);
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
			if(disk->clusters[index].clusterChain != NULL)
			{
				// already traversed
				continue;
			}
				
			ClusterChain* newChain = FATImage_GetNewFileChain(disk);

			size_t currentIndex = index;
			while(true)
			{
				uint16_t currentValue = disk->clusters[currentIndex].rawTableValue;
				if(currentValue >= 0xFF8 && currentValue <= 0xFFF)
				{
					ClusterChain_Append(newChain, currentIndex);
					disk->clusters[currentIndex].clusterChain = newChain;
					disk->clusters[currentIndex].status = FileLast;

					// last file in chain, break
					break;
				}
				else if(currentValue >= 2 && currentValue < 2 + disk->information.dataSectorCount)
				{
					ClusterChain_Append(newChain, currentIndex);
					disk->clusters[currentIndex].clusterChain = newChain;
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

	LOG(INFO, "Found %zd files...\n", disk->clusterChainsLength);
	for(size_t index = 0 ; index < disk->clusterChainsLength ; ++index)
	{
		LOG(DETAIL, "File with %zd clusters:\n", disk->clusterChains[index].length);
		if(disk->clusterChains[index].length > 0)
		{
			ClusterChainNode* node = disk->clusterChains[index].head;
			LOG(DEBUG, "\t%zd", node->index);
			node = node->next;
			while(node)
			{
				LOG(DEBUG, " > %zd", node->index);
				node = node->next;
			}
		}
		LOG(DEBUG, "\n");
	}
}

DirectoryEntry* FATImage_GetNewDirectoryEntry(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->directoryEntries != NULL);
	
	if(disk->directoryEntriesLength >= disk->directoryEntriesCapacity)
	{
		// realloc
		disk->directoryEntries = realloc(disk->directoryEntries, 2 * disk->directoryEntriesCapacity * sizeof(DirectoryEntry));
		assert(disk->directoryEntries != NULL);
		memset(disk->directoryEntries + disk->directoryEntriesCapacity, 0, disk->directoryEntriesCapacity * sizeof(DirectoryEntry) / sizeof(unsigned char));
		disk->directoryEntriesCapacity *= 2;
	}

	disk->directoryEntriesLength += 1;
	return disk->directoryEntries + (disk->directoryEntriesLength - 1);
}

DirectoryEntry* FATImage_InitializeNewDirectoryEntry(FATImage* disk, uint8_t* directoryEntry, size_t directoryEntrySize)
{
	assert(disk != NULL);
	assert(disk->directoryEntries != NULL);
	assert(directoryEntrySize == 32);

	DirectoryEntry* entry = FATImage_GetNewDirectoryEntry(disk);

	entry->filename = calloc(9, sizeof(char));
	assert(entry->filename != NULL);
	CopyUntilFirstSpace((char*)directoryEntry, 8, entry->filename);

	entry->extension = calloc(4, sizeof(char));
	assert(entry->extension != NULL);
	CopyUntilFirstSpace((char*)directoryEntry + 8, 3, entry->extension);

	entry->attributes = directoryEntry[11];
	entry->startCluster = NumberFrom8ByteLittleEndianSequence(directoryEntry + 26, 2);
	entry->fileSize = NumberFrom8ByteLittleEndianSequence(directoryEntry + 28, 4);
	
	return entry;
}

void FATImage_ReadDirectoryEntries_Internal(FATImage* disk, size_t sector, DirectoryEntry* parent)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);	

	size_t sectorSize = disk->information.sectorSize;
	size_t directoryEntrySize = 32;
	size_t directoryEntryCount = sectorSize / directoryEntrySize;
	for(size_t index = 0 ; index < directoryEntryCount ; ++index) 
	{
		uint8_t* rawDirectoryEntry = &(disk->image[sectorSize * sector + index * directoryEntrySize]);
		uint8_t firstByte = rawDirectoryEntry[0];
		if(firstByte == 0xE5)
		{
			// nothing in this directory entry
			continue;
		}
		else if(firstByte == 0x00)
		{
			// last directory entry in sector
			if(parent == NULL && disk->lastRootDirectoryEntry == NULL)
			{
				LOG(DEBUG, "last root directory entry is %zd\n", sectorSize * sector + index * directoryEntrySize);
				disk->lastRootDirectoryEntry = rawDirectoryEntry;
			}
			break;
		}
		else
		{
			DirectoryEntry* entry = FATImage_InitializeNewDirectoryEntry(disk, rawDirectoryEntry, directoryEntrySize);
			entry->parent = parent;

			char* filename = entry->filename;
			bool delete = true;
			while(true)
			{
				if(*filename == '\0')
					break;
				if(*filename != '.')
				{
					delete = false;
					break;
				}
				filename++;
			}

			if(delete || DirectoryEntry_IsVolumeLabel(entry))
			{
				free(entry->filename);
				free(entry->extension);
				memset(entry, 0, sizeof(DirectoryEntry) / sizeof(unsigned char));
				disk->directoryEntriesLength -= 1;
				continue;
			}
			
			Cluster* cluster  = disk->clusters + entry->startCluster;
			if(cluster->clusterChain && cluster->clusterChain->head->index == entry->startCluster)
			{
				LOG(DETAIL, "found matching cluster chain of length %zd!\n", cluster->clusterChain->length);
				cluster->clusterChain->directoryEntry = entry;
			}

			if(DirectoryEntry_IsSubdirectory(entry))
			{
				LOG(DETAIL, "following directory %s to %zd = cluster %zd\n", entry->filename, entry->startCluster, disk->information.dataSectorStartSector - 2 + entry->startCluster);
				FATImage_ReadDirectoryEntries_Internal(disk, disk->information.dataSectorStartSector - 2 + entry->startCluster, entry);
			}

			LOG(INFO, "found file %s.%s of size %zd\n", entry->filename, entry->extension, entry->fileSize);
		}
	}
}

void FATImage_ReadDirectoryEntries(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);	

	for(size_t index = 0 ; index < disk->information.rootDirectorySectorCount ; ++index)
		FATImage_ReadDirectoryEntries_Internal(disk, disk->information.rootDirectoryStartSector + index, NULL);
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

void FATImage_PrintUnreferencedClusters(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	printf("Unreferenced:");
	size_t unreferenced = 0;
	for(size_t index = 0 ; index < disk->clusterChainsLength ; ++index)
	{
		ClusterChain* chain = disk->clusterChains + index;
		if(chain->directoryEntry == NULL)
		{
			ClusterChainNode* node = chain->head;
			while(node)
			{
				unreferenced += 1;
				printf(" %zd", node->index);
				node = node->next;
			}
		}
	}
	if(unreferenced == 0)
		printf(" None");
	putchar('\n');
}

void FATImage_PrintLostFiles(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	for(size_t index = 0 ; index < disk->clusterChainsLength ; ++index)
	{
		ClusterChain* chain = disk->clusterChains + index;
		if(chain->directoryEntry == NULL)
		{
			printf("Lost file: %zd %zd\n", chain->head->index, chain->length);
		}
	}	
}

DirectoryEntry* FATImage_WriteNewRootDirectoryEntry(FATImage* disk, char* filename, char* extension, size_t fileSize, size_t startCluster)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	uint8_t* lastRootDirectoryEntry = disk->lastRootDirectoryEntry;

	for(size_t index = 0 ; index < 8 ; ++index)
	{
		if(index < strlen(filename))
			lastRootDirectoryEntry[index] = (uint8_t)filename[index];
		else
			lastRootDirectoryEntry[index] = ' ';
	}

	for(size_t index = 0 ; index < 3 ; ++index)
	{
		if(index < strlen(extension))
			lastRootDirectoryEntry[8 + index] = (uint8_t)extension[index];
		else
			lastRootDirectoryEntry[index] = ' ';
	}

	NumberTo8ByteLittleEndianSequence(fileSize, lastRootDirectoryEntry + 28, 4);
	NumberTo8ByteLittleEndianSequence(startCluster, lastRootDirectoryEntry + 26, 2);

	DirectoryEntry* toReturn = FATImage_InitializeNewDirectoryEntry(disk, lastRootDirectoryEntry, 32);
	if(INFO < log_level)
	{
		printf("Wrote new root directory entry:\n");
		DirectoryEntry_Print(toReturn);
	}

	disk->lastRootDirectoryEntry = disk->lastRootDirectoryEntry + 32;
	disk->lastRootDirectoryEntry[0] = 0x00;

	return toReturn;
}

void FATImage_RecoverLostFiles(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	int lost = 0;
	for(size_t index = 0 ; index < disk->clusterChainsLength ; ++index)
	{
		ClusterChain* chain = disk->clusterChains + index;
		if(chain->directoryEntry == NULL)
		{
			++lost;

			char* filename = calloc(9, sizeof(char));
			assert(filename != NULL);
			sprintf(filename, "FOUND%d", lost);

			char* extension = calloc(4, sizeof(char));
			assert(extension != NULL);
			strncpy(extension, "DAT", 3);

			size_t fileSize = chain->length * disk->information.sectorSize;

			DirectoryEntry* newEntry = FATImage_WriteNewRootDirectoryEntry(disk, filename, extension, fileSize, chain->head->index);
			chain->directoryEntry = newEntry;

			free(filename);
			free(extension);
		}
	}	
}

void FATImage_PrintSizeInconsistencies(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	size_t sectorSize = disk->information.sectorSize;
	for(size_t index = 0 ; index < disk->clusterChainsLength ; ++index)
	{
		ClusterChain* chain = disk->clusterChains + index;
		if(!ClusterChain_SizeMatchesDirectoryEntry(chain, sectorSize))
		{
			DirectoryEntry* entry = chain->directoryEntry;
			printf("%s.%s %zd %zd\n", entry->filename, entry->extension, entry->fileSize, chain->length * sectorSize);
		}
	}
}

void FATImage_TruncateClusterChain(FATImage* disk, ClusterChain* chain, size_t newLength)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);
	assert(chain != NULL);
	assert(newLength < chain->length);

	LOG(INFO, 	"truncating %s.%s to %zd clusters = %zd bytes\n", chain->directoryEntry->filename, chain->directoryEntry->extension,
				newLength, newLength * disk->information.sectorSize);

	size_t traversed = 0;
	ClusterChainNode* node = chain->head;
	while(node)
	{
		traversed++;
		if(traversed == newLength)
		{
			// mark cluster as last cluster of file
			WriteTo12ByteLittleEndianSequence(0xFFF, disk->image + 512, node->index);
		}
		else if(traversed > newLength)
		{
			// mark cluster as free
			WriteTo12ByteLittleEndianSequence(0x000, disk->image + 512, node->index);
		}
		
		node = node->next;
	}

	ClusterChain_Truncate(chain, newLength);
}

void FATImage_ResolveSizeInconsistencies(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->clusters != NULL);

	size_t sectorSize = disk->information.sectorSize;
	for(size_t index = 0 ; index < disk->clusterChainsLength ; ++index)
	{
		ClusterChain* chain = disk->clusterChains + index;
		if(!ClusterChain_SizeMatchesDirectoryEntry(chain, sectorSize))
		{
			size_t newLength = ceil(chain->directoryEntry->fileSize / 512.0);
			if(newLength == 0)
				newLength = 1;

			if(newLength < chain->length)
				FATImage_TruncateClusterChain(disk, chain, newLength);
		}
	}
}

void FATImage_SaveChanges(FATImage* disk)
{
	assert(disk != NULL);
	assert(disk->image != NULL);

	msync(disk->image, disk->imageSize, MS_SYNC);
}