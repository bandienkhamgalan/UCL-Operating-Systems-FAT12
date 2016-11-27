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
	return (directoryEntry->attributes & VolumeLabel) > 0;
}

bool DirectoryEntry_IsSubdirectory(DirectoryEntry* directoryEntry)
{
	return (directoryEntry->attributes & Subdirectory) > 0;
}