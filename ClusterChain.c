#include <assert.h>
#include "ClusterChain.h"

ClusterChain* ClusterChain_Make()
{
	ClusterChain* chain = calloc(1, sizeof(ClusterChain));
	assert(chain != NULL);

	return chain;
}

void ClusterChain_Free(ClusterChain* toFree)
{
	assert(toFree != NULL);
	ClusterChain_FreeNodes(toFree);
	free(toFree);
}

void ClusterChain_FreeNodes(ClusterChain* toFree)
{
	assert(toFree != NULL);
	ClusterChainNode* current = toFree->head;
	while(current != NULL)
	{
		ClusterChainNode* next = current->next;
		free(current);
		current = next;
	}
	toFree->head = toFree->tail = NULL;
	toFree->length = 0;
}

void ClusterChain_Append(ClusterChain* chain, size_t index)
{
	assert(chain != NULL);
	ClusterChainNode* node = calloc(1, sizeof(ClusterChainNode));
	assert(node != NULL);
	node->index = index;
	node->chain = chain;

	if(chain->length == 0)
	{
		chain->head = chain->tail = node;
	}
	else
	{
		chain->tail->next = node;
		chain->tail = node;
	}

	++(chain->length);
}

bool ClusterChain_SizeMatchesDirectoryEntry(ClusterChain* chain, size_t sectorSize)
{
	assert(chain != NULL);

	if(chain->directoryEntry == NULL)
		return true;

	size_t minSize = (chain->length - 1) * sectorSize;
	size_t maxSize = chain->length * sectorSize;

	return chain->directoryEntry->fileSize >= minSize && chain->directoryEntry->fileSize <= maxSize;
}

void ClusterChain_Truncate(ClusterChain* chain, size_t newLength)
{
	assert(chain != NULL);

	if(newLength < chain->length)
	{
		ClusterChainNode* node = chain->head;
		size_t traversed = 1;
		while(node)
		{
			if(traversed == newLength)
			{
				ClusterChainNode* next = node->next;
				node->next = NULL;
				node = next;
			}
			else if(traversed > newLength)
			{
				ClusterChainNode* next = node->next;
				free(node);
				node = next;
			}
			else
			{
				node = node->next;
			}
			
			traversed++;
		}

		chain->length = newLength;
	}
}