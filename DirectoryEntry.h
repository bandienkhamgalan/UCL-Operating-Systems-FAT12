/** @file DirectoryEntry.h
 *	@author Bandi Enkh-Amgalan
 *  @brief Declaration of DirectoryEntry struct and supporting functions
 *
 *  DirectoryEntry struct encapsulates the information parsed from a directory entry
 *  in a File Allocation Table. 
 *
 */

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

/** @brief	Check if a DirectoryEntry is a volume label
 *
 *	@param	directoryEntry
 *  @return true if DirectoryEntry is a volume label, false otherwise
 */
bool DirectoryEntry_IsVolumeLabel(DirectoryEntry* directoryEntry);

/** @brief	Check if a DirectoryEntry is a subdirectory
 *
*	@param	directoryEntry
 *  @return true if DirectoryEntry is a subdirectory, false otherwise
 */
bool DirectoryEntry_IsSubdirectory(DirectoryEntry* directoryEntry);

/** @brief	(For debug purposes) Print DirectoryEntry contents to stdout 
 *
*	@param	directoryEntry
 */
void DirectoryEntry_Print(DirectoryEntry* directoryEntry);