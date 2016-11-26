#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct IndexChain;

typedef struct IndexChainNode
{
	size_t index;
	struct IndexChainNode* next;
	struct IndexChain* chain;
} IndexChainNode;

typedef struct IndexChain
{
	IndexChainNode* head;
	IndexChainNode* tail;
	size_t length;
} IndexChain;

IndexChain* IndexChain_Make();
void IndexChain_Free(IndexChain* chain);
void IndexChain_FreeNodes(IndexChain* chain);
void IndexChain_Append(IndexChain* chain, size_t index);