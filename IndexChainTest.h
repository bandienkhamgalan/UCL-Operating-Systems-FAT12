#include "greatest/greatest.h"
#include "IndexChain.h"

TEST IndexChainMake_ReturnsZeroedOutStruct()
{
	IndexChain* chain = IndexChain_Make();
	ASSERT_EQ(chain->head, NULL);
	ASSERT_EQ(chain->tail, NULL);
	ASSERT_EQ(chain->length, 0);
	IndexChain_Free(chain);
	PASS();
}

TEST IndexChainAppend_SetsChainPropertyOfNewNode()
{
	IndexChain* chain = IndexChain_Make();
	IndexChain_Append(chain, 5);
	ASSERT_EQ(chain->head->chain, chain);
	IndexChain_Free(chain);
	PASS();
}

TEST IndexChainAppend_IncrementsLength()
{
	IndexChain* chain = IndexChain_Make();
	ASSERT_EQ(chain->length, 0);
	IndexChain_Append(chain, 5);
	ASSERT_EQ(chain->length, 1);
	IndexChain_Free(chain);
	PASS();
}

TEST IndexChainAppend_FirstElement_SetsHeadAndTail()
{
	IndexChain* chain = IndexChain_Make();
	IndexChain_Append(chain, 5);
	ASSERT_EQ(chain->head->index, 5);
	ASSERT_EQ(chain->tail->index, 5);
	IndexChain_Free(chain);
	PASS();
}

TEST IndexChainAppend_UpdatesAllReferences()
{
	IndexChain* chain = IndexChain_Make();
	IndexChain_Append(chain, 1);
	IndexChain_Append(chain, 2);
	IndexChain_Append(chain, 3);
	ASSERT_EQ(chain->length, 3);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->head->next->index, 2);
	ASSERT_EQ(chain->head->next->next->index, 3);
	ASSERT_EQ(chain->tail->index, 3);
	IndexChain_Free(chain);
	PASS();
}

TEST IndexChainFreeNodes_UpdatesLengthAndHeadAndTail()
{
	IndexChain* chain = IndexChain_Make();
	IndexChain_Append(chain, 1);
	IndexChain_Append(chain, 2);
	IndexChain_Append(chain, 3);
	ASSERT_EQ(chain->length, 3);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->tail->index, 3);
	IndexChain_FreeNodes(chain);
	ASSERT_EQ(chain->length, 0);
	ASSERT_EQ(chain->head, NULL);
	ASSERT_EQ(chain->tail, NULL);
	IndexChain_Free(chain);
	PASS();
}

SUITE(IndexChainTest)
{
	RUN_TEST(IndexChainMake_ReturnsZeroedOutStruct);
	RUN_TEST(IndexChainAppend_IncrementsLength);
	RUN_TEST(IndexChainAppend_SetsChainPropertyOfNewNode);
	RUN_TEST(IndexChainAppend_FirstElement_SetsHeadAndTail);
	RUN_TEST(IndexChainAppend_UpdatesAllReferences);
	RUN_TEST(IndexChainFreeNodes_UpdatesLengthAndHeadAndTail);
}