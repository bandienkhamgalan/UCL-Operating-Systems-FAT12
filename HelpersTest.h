#include "Helpers.h"

TEST Read12ByteLittleEndianSequence_Success()
{
	uint8_t values[12] = { 0x03, 0x40, 0x00, 0x05, 0x60, 0x00, 0x07, 0x80, 0x00, 0xFF, 0xAF, 0x00};
	uint16_t converted[8];
	Read12ByteLittleEndianSequence(values, 12, converted, 8);
	ASSERT_EQ(converted[0], 3);
	ASSERT_EQ(converted[1], 4);
	ASSERT_EQ(converted[2], 5);
	ASSERT_EQ(converted[3], 6);
	ASSERT_EQ(converted[4], 7);
	ASSERT_EQ(converted[5], 8);
	ASSERT_EQ(converted[6], 4095);
	ASSERT_EQ(converted[7], 10);
	PASS();
}

TEST CopyUntilFirstSpace_AllSpaces()
{
	char source[] = "       ";
	char destination[8];
	CopyUntilFirstSpace(source, 8, destination);
	ASSERT_EQ(strlen(destination), 0);
	ASSERT_STR_EQ(destination, "");
	PASS();
}

TEST CopyUntilFirstSpace_OneWord()
{
	char source[] = "hello  ";
	char destination[8];
	CopyUntilFirstSpace(source, 8, destination);
	ASSERT_EQ(strlen(destination), 5);
	ASSERT_STR_EQ(destination, "hello");
	PASS();
}

TEST CopyUntilFirstSpace_TwoWords()
{
	char source[] = "hel  pot";
	char destination[8];
	CopyUntilFirstSpace(source, 8, destination);
	ASSERT_EQ(strlen(destination), 3);
	ASSERT_STR_EQ(destination, "hel");
	PASS();
}

TEST NumberFrom8ByteLittleEndianSequence_Success()
{
	uint8_t values[] = { 0x05, 0x00, 0x0F, 0x05, 0x60, 0xFF };
	ASSERT_EQ(NumberFrom8ByteLittleEndianSequence(values + 0, 2), 0x0005);
	ASSERT_EQ(NumberFrom8ByteLittleEndianSequence(values + 1, 2), 0x0F00);
	ASSERT_EQ(NumberFrom8ByteLittleEndianSequence(values + 3, 2), 0x6005);
	ASSERT_EQ(NumberFrom8ByteLittleEndianSequence(values + 3, 3), 0xFF6005);
	PASS();
}

SUITE(HelpersTest)
{
	RUN_TEST(Read12ByteLittleEndianSequence_Success);
	RUN_TEST(CopyUntilFirstSpace_AllSpaces);
	RUN_TEST(CopyUntilFirstSpace_OneWord);
	RUN_TEST(CopyUntilFirstSpace_TwoWords);
	RUN_TEST(NumberFrom8ByteLittleEndianSequence_Success);
}