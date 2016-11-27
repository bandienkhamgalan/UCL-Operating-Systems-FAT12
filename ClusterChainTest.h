#include "greatest/greatest.h"
#include "ClusterChain.h"

TEST ClusterChainMake_ReturnsZeroedOutStruct()
{
	ClusterChain* chain = ClusterChain_Make();
	ASSERT_EQ(chain->head, NULL);
	ASSERT_EQ(chain->tail, NULL);
	ASSERT_EQ(chain->length, 0);
	ASSERT_EQ(chain->directoryEntry, NULL);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChainAppend_SetsChainPropertyOfNewNode()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 5);
	ASSERT_EQ(chain->head->chain, chain);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChainAppend_IncrementsLength()
{
	ClusterChain* chain = ClusterChain_Make();
	ASSERT_EQ(chain->length, 0);
	ClusterChain_Append(chain, 5);
	ASSERT_EQ(chain->length, 1);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChainAppend_FirstElement_SetsHeadAndTail()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 5);
	ASSERT_EQ(chain->head->index, 5);
	ASSERT_EQ(chain->tail->index, 5);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChainAppend_UpdatesAllReferences()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 1);
	ClusterChain_Append(chain, 2);
	ClusterChain_Append(chain, 3);
	ASSERT_EQ(chain->length, 3);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->head->next->index, 2);
	ASSERT_EQ(chain->head->next->next->index, 3);
	ASSERT_EQ(chain->tail->index, 3);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChainFreeNodes_UpdatesLengthAndHeadAndTail()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 1);
	ClusterChain_Append(chain, 2);
	ClusterChain_Append(chain, 3);
	ASSERT_EQ(chain->length, 3);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->tail->index, 3);
	ClusterChain_FreeNodes(chain);
	ASSERT_EQ(chain->length, 0);
	ASSERT_EQ(chain->head, NULL);
	ASSERT_EQ(chain->tail, NULL);
	ClusterChain_Free(chain);
	PASS();
}

SUITE(ClusterChainTest)
{
	RUN_TEST(ClusterChainMake_ReturnsZeroedOutStruct);
	RUN_TEST(ClusterChainAppend_IncrementsLength);
	RUN_TEST(ClusterChainAppend_SetsChainPropertyOfNewNode);
	RUN_TEST(ClusterChainAppend_FirstElement_SetsHeadAndTail);
	RUN_TEST(ClusterChainAppend_UpdatesAllReferences);
	RUN_TEST(ClusterChainFreeNodes_UpdatesLengthAndHeadAndTail);
}