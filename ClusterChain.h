/** @file ClusterChain.h
 *	@author Bandi Enkh-Amgalan
 *  @brief Declaration of ClusterChain struct and supporting functions
 *
 *  ClusterChain is a linked list specialized for storing chains of cluster indices
 *  parsed from File Allocation Tables. */

#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "DirectoryEntry.h"

struct ClusterChain;

/* Node in a CLusterChain linked list */
typedef struct ClusterChainNode
{
	size_t index;
	struct ClusterChainNode* next;
	struct ClusterChain* chain;
} ClusterChainNode;

/* Specialized singly-linked list for storing chains of FAT12 cluster indices */
typedef struct ClusterChain
{
	ClusterChainNode* head;
	ClusterChainNode* tail;
	size_t length;
	DirectoryEntry* directoryEntry;
} ClusterChain;

/** @brief	Allocate and initialize an empty ClusterChain linked list on the heap
 *
 *			This function will dynamically allocate a ClusterChain struct. Caller must free the HashTable
 *			struct by calling ClusterChain_Free(ClusterChain*). Simply calling free() will cause memory leaks!
 *
 *  @return pointer to dynamically allocated ClusterChain struct
 */
ClusterChain* ClusterChain_Make();

/** @brief	Free a dynamically allocated ClusterChain struct
 *
 *			Call this function if the ClusterChain was created using ClusterChain_Make()
 *
 *  @param 	chain */
void ClusterChain_Free(ClusterChain* chain);

/** @brief	Free the nodes appended onto a ClusterChain linked list struct
 *
 *			Call this function if you manually initialized ClusterChain on the stack or heap
 *			and any indices were appended onto the linked list using ClusterChain_Append()
 *
 *  @param 	chain */
void ClusterChain_FreeNodes(ClusterChain* chain);

/** @brief	Append a new index onto a ClusterChain linked list
 *
 *			This function will append an index onto the tail of the linked list,
 *			allocating memory for the new node on the heap. Caller must call 
 *			ClusterChain_FreeNodes() or ClusterChain_Free() afterwards. 
 *
 *  @param 	chain
 *  @param 	index 	value to append */
void ClusterChain_Append(ClusterChain* chain, size_t index);

/** @brief	Truncate the ClusterChain to be the specified length
 *
 *			This function will remove any nodes past the desired length and
 *			free the memory allocated for the nodes. 
 *
 *  @param 	chain
 *  @param 	newLength */
void ClusterChain_Truncate(ClusterChain* chain, size_t newLength);

/** @brief	Check if ClusterChain length is consistent with DirectoryEntry assuming specified sector size 
 *
 *			This function will check if the length of a ClusterChain is consistent with the 
 *			fileSize of the directoryEntry field, assuming the specified sector size
 *
 *			This function will return true if the directoryEntry field is NULL
 *
 *  @param 	chain
 *  @param 	sectorSize	size of sector in bytes
 *  @return true if length matches DirectoryEntry size, false otherwise */
bool ClusterChain_SizeMatchesDirectoryEntry(ClusterChain* chain, size_t sectorSize);