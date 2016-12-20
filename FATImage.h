/** @file FATImage.h
 *	@author Bandi Enkh-Amgalan
 *  @brief Declaration of data structures and functions for reading, checking and repairing FAT12 floppy images
 *
 *	This function declares various structs for encapsulating the information parsed from a FAT12 floppy image,
 *  such as directory entries, cluster chains found in the file alocation talbe and boot sector information.
 *  It also provides various tools for checking and repairing FAT12 file system images. */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "ClusterChain.h"
#include "DirectoryEntry.h"

/* FAT12 disk information (as parsed from boot sector) */
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

/* Cluster status (as parsed from file allocation table) */
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

/* Cluster information (parsed from file allocation table) */
typedef struct
{
	ClusterChain* clusterChain;
	uint16_t rawTableValue;
	ClusterStatus status;
} Cluster;

/* Encapsulation of a FAT12 floppy disk image, and any parsed clusters and directory entries */
typedef struct
{
	Cluster* clusters;
	size_t clustersLength;

	ClusterChain* clusterChains;
	size_t clusterChainsLength;
	size_t clusterChainsCapacity;

	DirectoryEntry* directoryEntries;
	size_t directoryEntriesLength;
	size_t directoryEntriesCapacity;
	
	uint8_t* lastRootDirectoryEntry;

	uint8_t* image;
	size_t imageSize;
	int imageFileDescriptor; 
	FATDiskInformation information;
} FATImage;

/** @brief	Initialize a FATImage struct with an image file
 *
 *			This function will dynamically allocate a FATImage struct. Caller must free the FATImage
 *			struct by calling FATImage_Free(). Simply calling free() will cause memory leaks!
 *
 *			If any errors are encountered when reading and memory mapping the image file,
 *			the function will print out error information to stdout and return NULL.
 *			On successful initialization, caller should call FATImage_UpdateDiskInformation(),
 *			FATImage_ReadFileAllocationTable() and FATImage_ReadDirectoryEntries() in succession,
 *			before any file system check and repair operations. 
 *
 *  @return pointer to dynamically allocated FATImage struct on success, NULL otherwise */
FATImage* FATImage_Initialize(char* imageFile);

/** @brief	Free a dynamically allocated FATImage struct
 *
 *  @param disk */
void FATImage_Free(FATImage* disk);

/** @brief	Read boot sector information and load information into FATImage struct
 *
 *			This function loads essential information from the boot sector of the disk
 *			into the FATImage struct and must be called before file system checks and repair operations. 
 *
 *  @param 	disk */
void FATImage_UpdateDiskInformation(FATImage* disk);

/** @brief	Read file allocation table and load information into FATImage struct
 *
 *			This function requires boot sector information to have been parsed with a call to
 *			FATImage_UpdateDiskInformation(). 
 *
 *			This function loads essential information from the file allocation tables
 *			into the FATImage struct, and must be called before file system checks and repair operations. 
 *			
 *
 *  @param 	disk */
void FATImage_ReadFileAllocationTable(FATImage* disk);

/** @brief	Read all directory entries and load information into FATImage struct
 *
 *			This function requires boot sector information and file allocation table to have been
 *			parsed with a call to FATImage_UpdateDiskInformation() and FATImage_ReadFileAllocationTable(). 
 *
 *			This function loads essential information from the file allocation tables
 *			into the FATImage struct, and must be called before file system checks and repair operations. 
 *			
 *  @param 	disk */
void FATImage_ReadDirectoryEntries(FATImage* disk);

/** @brief	Print (to stdout) indices of clusters that have not been referenced by any directory entries
 *
 *			This function requires boot sector information, file allocation table and directory entries
 *			to have been parsed with calls to FATImage_UpdateDiskInformation(), FATImage_ReadFileAllocationTable()
 *			and FATImage_ReadDirectoryEntries() functions. 
 *			
 *  @param 	disk */
void FATImage_PrintUnreferencedClusters(FATImage* disk);

/** @brief	Print (to stdout) files/cluster chains that have not been referenced by any directory entries
 *
 *			This function requires boot sector information, file allocation table and directory entries
 *			to have been parsed with calls to FATImage_UpdateDiskInformation(), FATImage_ReadFileAllocationTable()
 *			and FATImage_ReadDirectoryEntries() functions. 
 *			
 *  @param 	disk */
void FATImage_PrintLostFiles(FATImage* disk);

/** @brief	Recover lost files by writing new directory entries for unreferenced files
 *
 *			This function writes new root directory entries referencing each lost file. The lost files
 *			are named "FOUND1.DAT", "FOUND2.DAT" and so on. Each file is specified to have a size
 *			of sector_size (usually 512 bytes) * length of unreferenced cluster index chain. 
 *
 *			This function modifies the mapped image, but changes will not be flushed to disk until  
 *			FATImage_SaveChanges() is called. 
 *
 *			This function requires boot sector information, file allocation table and directory entries
 *			to have been parsed with calls to FATImage_UpdateDiskInformation(), FATImage_ReadFileAllocationTable()
 *			and FATImage_ReadDirectoryEntries() functions. 
 *			
 *  @param 	disk */
void FATImage_RecoverLostFiles(FATImage* disk);

/** @brief	Display files where the directory entry size mismatches the file allocation table cluster chain length
 *
 *			This function requires boot sector information, file allocation table and directory entries
 *			to have been parsed with calls to FATImage_UpdateDiskInformation(), FATImage_ReadFileAllocationTable()
 *			and FATImage_ReadDirectoryEntries() functions. 
 *			
 *  @param 	disk */
void FATImage_PrintSizeInconsistencies(FATImage* disk);

/** @brief	Resolve size inconsistencies by freeing clusters that are past the end of the file according to directory entry
 *
 *			This function writes new cluster statuses in the file allocation table. 
 *
 *			This function modifies the mapped image, but changes will not be flushed to disk until  
 *			FATImage_SaveChanges() is called. 
 *
 *			This function requires boot sector information, file allocation table and directory entries
 *			to have been parsed with calls to FATImage_UpdateDiskInformation(), FATImage_ReadFileAllocationTable()
 *			and FATImage_ReadDirectoryEntries() functions. 
 *			
 *  @param 	disk */
void FATImage_ResolveSizeInconsistencies(FATImage* disk);

/** @brief	Flush any pending write operations to disk 
 *
 *			This function must be called after FATImage_ResolveSizeInconsistencies() and FATImage_RecoverLostFiles()
 *			for the writes to be flushed to disk. 
 *			
 *  @param 	disk */
void FATImage_SaveChanges(FATImage* disk);