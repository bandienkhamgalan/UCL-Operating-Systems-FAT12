#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "DirectoryEntry.h"

struct ClusterChain;

typedef struct ClusterChainNode
{
	size_t index;
	struct ClusterChainNode* next;
	struct ClusterChain* chain;
} ClusterChainNode;

typedef struct ClusterChain
{
	ClusterChainNode* head;
	ClusterChainNode* tail;
	size_t length;
	DirectoryEntry* directoryEntry;
} ClusterChain;

ClusterChain* ClusterChain_Make();
void ClusterChain_Free(ClusterChain* chain);
void ClusterChain_FreeNodes(ClusterChain* chain);
void ClusterChain_Append(ClusterChain* chain, size_t index);
void ClusterChain_Truncate(ClusterChain* chain, size_t newLength);

bool ClusterChain_SizeMatchesDirectoryEntry(ClusterChain* chain, size_t sectorSize);