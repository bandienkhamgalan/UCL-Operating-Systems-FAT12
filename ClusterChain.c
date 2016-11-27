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