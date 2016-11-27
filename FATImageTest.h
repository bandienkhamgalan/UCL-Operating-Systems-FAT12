#include <stdint.h>
#include "greatest/greatest.h"
#include "FATImage.h"

TEST FATImage_Make_ReturnsZeroedOutStructWithZeroedOutFileChains()
{
	FATImage* disk = FATImage_Make();
	ASSERT_EQ(disk->clusters, NULL);
	ASSERT_EQ(disk->clustersLength, 0);
	ASSERT_EQ(disk->image, NULL);
	ASSERT_EQ(disk->imageSize, 0);
	ASSERT_EQ(disk->imageFileDescriptor, 0);

	ASSERT_EQ(disk->fileChainsLength, 0);
	if(disk->fileChainsCapacity > 0)
	{
		for(size_t index = 0 ; index < disk->fileChainsCapacity ; ++index)
		{
			IndexChain* chain = disk->fileChains + index;
			ASSERT_EQ(chain->head, NULL);
			ASSERT_EQ(chain->tail, NULL);
			ASSERT_EQ(chain->length, 0);
		}
	} else {
		ASSERT_EQ(disk->fileChains, NULL);
	}

	ASSERT_EQ(disk->directoryEntriesLength, 0);
	if(disk->directoryEntriesCapacity > 0)
	{
		for(size_t index = 0 ; index < disk->directoryEntriesCapacity ; ++index)
		{
			DirectoryEntry* entry = disk->directoryEntries + index;
			ASSERT_EQ(entry->parent, NULL);
			ASSERT_EQ(entry->filename, NULL);
			ASSERT_EQ(entry->extension, NULL);
			ASSERT_EQ(entry->attribute, 0);
			ASSERT_EQ(entry->fileSize, 0);
			ASSERT_EQ(entry->startCluster, 0);
		}
	} else {
		ASSERT_EQ(disk->directoryEntries, NULL);
	}

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_Initialize_ReturnsStructWithImageFileInfo()
{
	FATImage* disk = FATImage_Initialize("images/floppy.img");
	ASSERT_EQ(disk->imageSize, 1474560);
	ASSERT(disk->image != NULL);

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_UpdateDiskInformation_Success()
{
	FATImage* disk = FATImage_Initialize("images/floppy.img");
	FATImage_UpdateDiskInformation(disk);
	ASSERT_EQ(disk->information.sectorSize, 512);
	ASSERT_EQ(disk->information.sectorCount, 2880);
	ASSERT_EQ(disk->information.sectorsPerCluster, 1);

	ASSERT_EQ(disk->information.fileAllocationTableStartSector, 1);
	ASSERT_EQ(disk->information.fileAllocationTableSectorCount, 9);
	ASSERT_EQ(disk->information.fileAllocationTableCopies, 2);

	ASSERT_EQ(disk->information.rootDirectoryStartSector, 19);
	ASSERT_EQ(disk->information.rootDirectorySectorCount, 14);

	ASSERT_EQ(disk->information.dataSectorStartSector, 33);
	ASSERT_EQ(disk->information.dataSectorCount, 2847);

	FATImage_Free(disk);
	PASS();
}

IndexChain* FATImage_GetNewFileChain(FATImage* disk);

TEST FATImage_GetNewFileChain_ReturnsNewItemAndUpdatesLength()
{
	FATImage* disk = FATImage_Make();

	IndexChain* one = FATImage_GetNewFileChain(disk);
	IndexChain_Append(one, 42);

	IndexChain* two = FATImage_GetNewFileChain(disk);
	IndexChain_Append(two, 24);
	IndexChain_Append(two, 42);

	// Check length is updated
	ASSERT_EQ(disk->fileChainsLength, 2);

	// Check if two new/distinct items were actually returned
	ASSERT_EQ(one->length, 1);
	ASSERT_EQ(one->head->index, 42);

	ASSERT_EQ(two->length, 2);
	ASSERT_EQ(two->head->index, 24);
	ASSERT_EQ(two->head->next->index, 42);

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_GetNewFileChain_GrowsArrayAsNeeded()
{
	FATImage* disk = FATImage_Make();

	size_t capacity = disk->fileChainsCapacity;
	for(size_t index = 0 ; index < capacity ; ++index)
	{
		IndexChain* chain = FATImage_GetNewFileChain(disk);
		IndexChain_Append(chain, index);
	}
	ASSERT_EQ(disk->fileChainsLength, capacity);


	IndexChain* chain = FATImage_GetNewFileChain(disk);
	IndexChain_Append(chain, capacity);

	ASSERT_EQ(disk->fileChainsLength, capacity + 1);
	ASSERT(disk->fileChainsCapacity > capacity); 

	for(size_t index = 0 ; index < disk->fileChainsLength ; ++index)
	{
		IndexChain* chain = disk->fileChains + index;
		ASSERT_EQ(chain->length, 1);
		ASSERT_EQ(chain->head->index, index);
	}

	for(size_t index = disk->fileChainsLength ; index < disk->fileChainsCapacity ; ++index)
	{
		IndexChain* chain = disk->fileChains + index;
		ASSERT_EQ(chain->head, NULL);
		ASSERT_EQ(chain->tail, NULL);
		ASSERT_EQ(chain->length, 0);
	}

	FATImage_Free(disk);
	PASS();
}

DirectoryEntry* FATImage_GetNewDirectoryEntry(FATImage* disk);

TEST FATImage_GetNewDirectoryEntry_ReturnsNewItemAndUpdatesLength()
{
	FATImage* disk = FATImage_Make();

	DirectoryEntry* one = FATImage_GetNewDirectoryEntry(disk);
	one->filename = "one";

	DirectoryEntry* two = FATImage_GetNewDirectoryEntry(disk);
	two->filename = "two";

	// Check length is updated
	ASSERT_EQ(disk->directoryEntriesLength, 2);

	// Check if two new/distinct items were actually returned
	ASSERT_STR_EQ(one->filename, "one");
	ASSERT_STR_EQ(two->filename, "two");

	// set to NULL to prevent bad free() call
	one->filename = NULL;
	two->filename = NULL;

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_GetNewDirectoryEntry_GrowsArrayAsNeeded()
{
	FATImage* disk = FATImage_Make();

	size_t capacity = disk->directoryEntriesCapacity;
	for(size_t index = 0 ; index < capacity ; ++index)
	{
		DirectoryEntry* entry = FATImage_GetNewDirectoryEntry(disk);
		entry->fileSize = index;
	}
	ASSERT_EQ(disk->directoryEntriesLength, capacity);


	DirectoryEntry* entry = FATImage_GetNewDirectoryEntry(disk);
	entry->fileSize = capacity;
	ASSERT_EQ(disk->directoryEntriesLength, capacity + 1);
	ASSERT(disk->directoryEntriesCapacity > capacity); 

	for(size_t index = 0 ; index < disk->directoryEntriesLength ; ++index)
	{
		DirectoryEntry* entry = disk->directoryEntries + index;
		ASSERT_EQ(entry->fileSize, index);
	}

	for(size_t index = disk->directoryEntriesLength ; index < disk->directoryEntriesCapacity ; ++index)
	{
		DirectoryEntry* entry = disk->directoryEntries + index;
		ASSERT_EQ(entry->parent, NULL);
		ASSERT_EQ(entry->filename, NULL);
		ASSERT_EQ(entry->extension, NULL);
		ASSERT_EQ(entry->attribute, 0);
		ASSERT_EQ(entry->fileSize, 0);
		ASSERT_EQ(entry->startCluster, 0);
	}

	FATImage_Free(disk);
	PASS();
}

void CopyTableValuesToClusterArray(Cluster* clusters, uint16_t* values, size_t length)
{
	for(size_t index = 0 ; index < length ; ++index)
		clusters[index].rawTableValue = values[index];
}

void FATImage_ReadClusterIndexSequenceAndCreateFileChains(FATImage* disk);

TEST FATImage_ReadClusterIndexSequenceAndCreateFileChains_UnusedBadReserved()
{
	FATImage* disk = FATImage_Make();
	disk->clusters = calloc(7, sizeof(Cluster)); disk->clustersLength = 7;
	CopyTableValuesToClusterArray(disk->clusters, (uint16_t[]){ 0x000, 0x000, 0x000, 0xFF0, 0xFF3, 0xFF6, 0xFF7}, 7);
	FATImage_ReadClusterIndexSequenceAndCreateFileChains(disk);

	ASSERT_EQ(disk->fileChainsLength, 0);
	for(size_t index = 2 ; index < 6 ; ++index)
	{
		ASSERT_EQ(disk->clusters[index].fileChain, NULL);
	}

	ASSERT_EQ(disk->clusters[2].status, Unused);
	ASSERT_EQ(disk->clusters[3].status, Reserved);
	ASSERT_EQ(disk->clusters[4].status, Reserved);
	ASSERT_EQ(disk->clusters[5].status, Reserved);
	ASSERT_EQ(disk->clusters[6].status, Bad);

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_ReadClusterIndexSequenceAndCreateFileChains_FourSeparateFiles()
{
	FATImage* disk = FATImage_Make();
	disk->clusters = calloc(6, sizeof(Cluster)); disk->clustersLength = 6;
	CopyTableValuesToClusterArray(disk->clusters, (uint16_t[]){ 0x000, 0x000, 0xFF8, 0xFFB, 0xFFC, 0xFFF}, 6);
	FATImage_ReadClusterIndexSequenceAndCreateFileChains(disk);

	ASSERT_EQ(disk->fileChainsLength, 4);
	for(size_t index = 2 ; index < 6 ; ++index)
	{
		ASSERT_EQ(disk->clusters[index].fileChain->length, 1);
		ASSERT_EQ(disk->clusters[index].fileChain->head->index, index);
		ASSERT_EQ(disk->clusters[index].status, FileLast);
	}

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_ReadClusterIndexSequenceAndCreateFileChains_OneBigFile()
{
	FATImage* disk = FATImage_Make();
	disk->clusters = calloc(6, sizeof(Cluster)); disk->clustersLength = 6; disk->information.dataSectorCount = 4;
	CopyTableValuesToClusterArray(disk->clusters, (uint16_t[]){ 0x000, 0x000, 0x003, 0x004, 0x005, 0xFFF}, 6);
	FATImage_ReadClusterIndexSequenceAndCreateFileChains(disk);

	IndexChain* chain = disk->fileChains;
	ASSERT_EQ(disk->fileChainsLength, 1);
	ASSERT_EQ(chain->length, 4);
	ASSERT_EQ(chain->head->index, 2);
	ASSERT_EQ(chain->head->next->index, 3);
	ASSERT_EQ(chain->head->next->next->index, 4);
	ASSERT_EQ(chain->tail->index, 5);
	for(size_t index = 2 ; index < 6 ; ++index)
	{
		ASSERT_EQ(disk->clusters[index].fileChain, chain);
		ASSERT_EQ(disk->clusters[index].status, index == 5 ? FileLast : File);
	}

	FATImage_Free(disk);
	PASS();
}

TEST FATImage_ReadClusterIndexSequenceAndCreateFileChains_ThreeFragmentedFiles()
{
	FATImage* disk = FATImage_Make();
	disk->clusters = calloc(8, sizeof(Cluster)); disk->clustersLength = 8; disk->information.dataSectorCount = 6;
	CopyTableValuesToClusterArray(disk->clusters, (uint16_t[]){ 0x000, 0x000, 0x004, 0x005, 0xFFF, 0x006, 0xFFF, 0xFFF }, 8);
	FATImage_ReadClusterIndexSequenceAndCreateFileChains(disk);

	ASSERT_EQ(disk->fileChainsLength, 3);
	IndexChain* one = disk->fileChains;
	ASSERT_EQ(one->length, 2);
	ASSERT_EQ(one->head->index, 2);
	ASSERT_EQ(one->tail->index, 4);
	ASSERT_EQ(disk->clusters[2].status, File); ASSERT_EQ(disk->clusters[2].fileChain, one);
	ASSERT_EQ(disk->clusters[4].status, FileLast); ASSERT_EQ(disk->clusters[4].fileChain, one);

	IndexChain* two = disk->fileChains + 1;
	ASSERT_EQ(two->length, 3);
	ASSERT_EQ(two->head->index, 3);
	ASSERT_EQ(two->head->next->index, 5);
	ASSERT_EQ(two->tail->index, 6);
	ASSERT_EQ(disk->clusters[3].status, File); ASSERT_EQ(disk->clusters[3].fileChain, two);
	ASSERT_EQ(disk->clusters[5].status, File); ASSERT_EQ(disk->clusters[5].fileChain, two);
	ASSERT_EQ(disk->clusters[6].status, FileLast); ASSERT_EQ(disk->clusters[6].fileChain, two);

	IndexChain* three = disk->fileChains + 2;
	ASSERT_EQ(three->length, 1);
	ASSERT_EQ(three->head->index, 7);
	ASSERT_EQ(disk->clusters[7].status, FileLast); ASSERT_EQ(disk->clusters[7].fileChain, three);

	FATImage_Free(disk);
	PASS();
}

SUITE(FATImageTest)
{
	RUN_TEST(FATImage_Make_ReturnsZeroedOutStructWithZeroedOutFileChains);
	RUN_TEST(FATImage_Initialize_ReturnsStructWithImageFileInfo);
	RUN_TEST(FATImage_UpdateDiskInformation_Success);
	RUN_TEST(FATImage_GetNewFileChain_ReturnsNewItemAndUpdatesLength);
	RUN_TEST(FATImage_GetNewFileChain_GrowsArrayAsNeeded);
	RUN_TEST(FATImage_GetNewDirectoryEntry_ReturnsNewItemAndUpdatesLength);
	RUN_TEST(FATImage_GetNewDirectoryEntry_GrowsArrayAsNeeded);
	RUN_TEST(FATImage_ReadClusterIndexSequenceAndCreateFileChains_UnusedBadReserved);
	RUN_TEST(FATImage_ReadClusterIndexSequenceAndCreateFileChains_FourSeparateFiles);
	RUN_TEST(FATImage_ReadClusterIndexSequenceAndCreateFileChains_OneBigFile);
	RUN_TEST(FATImage_ReadClusterIndexSequenceAndCreateFileChains_ThreeFragmentedFiles);
}