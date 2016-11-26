#include <assert.h>
#include "IndexChain.h"

IndexChain* IndexChain_Make()
{
	IndexChain* chain = calloc(1, sizeof(IndexChain));
	assert(chain != NULL);

	return chain;
}

void IndexChain_Free(IndexChain* toFree)
{
	assert(toFree != NULL);
	IndexChain_FreeNodes(toFree);
	free(toFree);
}

void IndexChain_FreeNodes(IndexChain* toFree)
{
	assert(toFree != NULL);
	IndexChainNode* current = toFree->head;
	while(current != NULL)
	{
		IndexChainNode* next = current->next;
		free(current);
		current = next;
	}
	toFree->head = toFree->tail = NULL;
	toFree->length = 0;
}

void IndexChain_Append(IndexChain* chain, size_t index)
{
	IndexChainNode* node = calloc(1, sizeof(IndexChainNode));
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