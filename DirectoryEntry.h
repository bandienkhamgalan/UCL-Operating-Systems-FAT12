#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct DirectoryEntry
{
	struct DirectoryEntry* parent;
	char* filename;
	char* extension;
	uint8_t attributes;
	size_t fileSize;
	size_t startCluster;
} DirectoryEntry;

bool DirectoryEntry_IsVolumeLabel(DirectoryEntry* directoryEntry);
bool DirectoryEntry_IsSubdirectory(DirectoryEntry* directoryEntry);