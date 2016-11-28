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

TEST ClusterChain_SizeMatchesDirectoryEntry_NoEntryTrue()
{
	ClusterChain* chain = ClusterChain_Make();
	ASSERT_EQ(true, ClusterChain_SizeMatchesDirectoryEntry(chain, 512));
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_SizeMatchesDirectoryEntry_Entry0BytesLength1_True()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 42);
	DirectoryEntry entry;
	entry.fileSize = 0;
	chain->directoryEntry = &entry;
	ASSERT_EQ(true, ClusterChain_SizeMatchesDirectoryEntry(chain, 512));
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_SizeMatchesDirectoryEntry_Entry512BytesLength1_True()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 42);
	DirectoryEntry entry;
	entry.fileSize = 512;
	chain->directoryEntry = &entry;
	ASSERT_EQ(true, ClusterChain_SizeMatchesDirectoryEntry(chain, 512));
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_SizeMatchesDirectoryEntry_Entry513BytesLength1_False()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 42);
	DirectoryEntry entry;
	entry.fileSize = 513;
	chain->directoryEntry = &entry;
	ASSERT_EQ(false, ClusterChain_SizeMatchesDirectoryEntry(chain, 512));
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_SizeMatchesDirectoryEntry_Entry517BytesLength2_True()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 42);
	ClusterChain_Append(chain, 42);
	DirectoryEntry entry;
	entry.fileSize = 517;
	chain->directoryEntry = &entry;
	ASSERT_EQ(true, ClusterChain_SizeMatchesDirectoryEntry(chain, 512));
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_Truncate_EqualToCurrentLength_Noop()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 1);
	ClusterChain_Append(chain, 2);
	ClusterChain_Append(chain, 3);
	ClusterChain_Truncate(chain, 3);
	ASSERT_EQ(chain->length, 3);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->head->next->index, 2);
	ASSERT_EQ(chain->head->next->next->index, 3);
	ASSERT_EQ(chain->tail->index, 3);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_Truncate_GreaterThanCurrentLength_Noop()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 1);
	ClusterChain_Append(chain, 2);
	ClusterChain_Append(chain, 3);
	ClusterChain_Truncate(chain, 5);
	ASSERT_EQ(chain->length, 3);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->head->next->index, 2);
	ASSERT_EQ(chain->head->next->next->index, 3);
	ASSERT_EQ(chain->tail->index, 3);
	ClusterChain_Free(chain);
	PASS();
}

TEST ClusterChain_Truncate_LessThanCurrentLength_Success()
{
	ClusterChain* chain = ClusterChain_Make();
	ClusterChain_Append(chain, 1);
	ClusterChain_Append(chain, 2);
	ClusterChain_Append(chain, 3);
	ClusterChain_Append(chain, 4);
	ClusterChain_Append(chain, 5);
	ClusterChain_Truncate(chain, 2);
	ASSERT_EQ(chain->length, 2);
	ASSERT_EQ(chain->head->index, 1);
	ASSERT_EQ(chain->head->next->index, 2);
	ASSERT_EQ(chain->head->next->next, NULL);
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

	RUN_TEST(ClusterChain_SizeMatchesDirectoryEntry_NoEntryTrue);
	RUN_TEST(ClusterChain_SizeMatchesDirectoryEntry_Entry0BytesLength1_True);
	RUN_TEST(ClusterChain_SizeMatchesDirectoryEntry_Entry512BytesLength1_True);
	RUN_TEST(ClusterChain_SizeMatchesDirectoryEntry_Entry513BytesLength1_False);
	RUN_TEST(ClusterChain_SizeMatchesDirectoryEntry_Entry517BytesLength2_True);

	RUN_TEST(ClusterChain_Truncate_EqualToCurrentLength_Noop);
	RUN_TEST(ClusterChain_Truncate_GreaterThanCurrentLength_Noop);
	RUN_TEST(ClusterChain_Truncate_LessThanCurrentLength_Success);
}