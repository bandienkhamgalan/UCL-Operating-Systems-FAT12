#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "IndexChain.h"

typedef struct
{
	size_t sectorSize;
	size_t sectorCount;
	size_t sectorsPerCluster;
	size_t fileAllocationTableStartSector;
	size_t fileAllocationTableSectorCount;
	size_t fileAllocationTableCopies;
	size_t rootDirectorySectorCount;
	size_t rootDirectoryStartSector;
	size_t dataSectorCount;
	size_t dataSectorStartSector;
} FATDiskInformation;

typedef enum
{
	MIN = 0,
	Unused,
	Reserved,
	Bad,
	File,
	FileLast,
	MAX
} ClusterStatus;

typedef struct
{
	IndexChain* fileChain;
	uint16_t rawTableValue;
	ClusterStatus status;
} Cluster;

typedef enum
{
	ReadOnly 		= 0x01,
	Hidden	 		= 0x02,
	System 			= 0x04,
	VolumeLabel		= 0x08,
	Subdirectory	= 0x10,
	Archive			= 0x20,
	NotUsed			= 0x40,
	NotUsed2		= 0x80
} DirectoryEntryAttributes;

typedef struct DirectoryEntry
{
	struct DirectoryEntry* parent;
	char* filename;
	char* extension;
	DirectoryEntryAttributes attributes;
	size_t fileSize;
	size_t startCluster;
} DirectoryEntry;

typedef struct
{
	Cluster* clusters;
	size_t clustersLength;

	IndexChain* fileChains;
	size_t fileChainsLength;
	size_t fileChainsCapacity;

	DirectoryEntry* directoryEntries;
	size_t directoryEntriesLength;
	size_t directoryEntriesCapacity;
	
	uint8_t* image;
	size_t imageSize;
	int imageFileDescriptor; 
	FATDiskInformation information;
} FATImage;

FATImage* FATImage_Make();
FATImage* FATImage_Initialize(char* imageFile);
void FATImage_Free(FATImage* disk);
void FATImage_UpdateDiskInformation(FATImage* disk);
void FATImage_ReadFileAllocationTable(FATImage* disk);
void FATImage_ReadDirectoryEntries(FATImage* disk);