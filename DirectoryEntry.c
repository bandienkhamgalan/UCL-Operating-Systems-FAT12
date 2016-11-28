#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "DirectoryEntry.h"

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

bool DirectoryEntry_IsVolumeLabel(DirectoryEntry* directoryEntry)
{
	assert(directoryEntry != NULL);
	return (directoryEntry->attributes & VolumeLabel) > 0;
}

bool DirectoryEntry_IsSubdirectory(DirectoryEntry* directoryEntry)
{
	assert(directoryEntry != NULL);
	return (directoryEntry->attributes & Subdirectory) > 0;
}

void DirectoryEntry_Print(DirectoryEntry* directoryEntry)
{
	assert(directoryEntry != NULL);
	bool isDirectory = DirectoryEntry_IsSubdirectory(directoryEntry);
	printf(isDirectory ? "Directory" : "File");
	if(directoryEntry->filename && strlen(directoryEntry->filename) > 0)
	{
		printf(" named %s", directoryEntry->filename);
		if(directoryEntry->extension && strlen(directoryEntry->extension) > 0)
			printf(".%s", directoryEntry->extension);
	}
	if(!isDirectory)
		printf(" of size %zd bytes", directoryEntry->fileSize);
	printf(" starting at cluster %zd\n", directoryEntry->startCluster);
}