#include "Helpers.h"

TEST Read12BitLittleEndianSequence_Success()
{
	uint8_t values[12] = { 0x03, 0x40, 0x00, 0x05, 0x60, 0x00, 0x07, 0x80, 0x00, 0xFF, 0xAF, 0x00};
	uint16_t converted[8];
	Read12BitLittleEndianSequence(values, 12, converted, 8);
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

TEST NumberFrom8BitLittleEndianSequence_Success()
{
	uint8_t values[] = { 0x05, 0x00, 0x0F, 0x05, 0x60, 0xFF };
	ASSERT_EQ(NumberFrom8BitLittleEndianSequence(values + 0, 2), 0x0005);
	ASSERT_EQ(NumberFrom8BitLittleEndianSequence(values + 1, 2), 0x0F00);
	ASSERT_EQ(NumberFrom8BitLittleEndianSequence(values + 3, 2), 0x6005);
	ASSERT_EQ(NumberFrom8BitLittleEndianSequence(values + 3, 3), 0xFF6005);
	PASS();
}

TEST NumberTo8BitLittleEndianSequence_Success()
{
	uint8_t values[4] = { 0x00 };
	NumberTo8BitLittleEndianSequence(0x123456, values, 4);
	ASSERT_EQ(values[0], 0x56);
	ASSERT_EQ(values[1], 0x34);
	ASSERT_EQ(values[2], 0x12);
	ASSERT_EQ(values[3], 0x00);
	ASSERT_EQ(NumberFrom8BitLittleEndianSequence(values, 4), 0x123456);
	PASS();
}

TEST Write12BitLittleEndianSequence_Success()
{
	uint8_t values[] = { 0xDE, 0xAD, 0xBE, 0xEE, 0xDE, 0xAD };
	Write12BitLittleEndianSequence(0x345, values, 0);
	ASSERT_EQ(values[0], 0x45);
	ASSERT_EQ(values[1], 0xA3);
	Write12BitLittleEndianSequence(0x789, values, 1);
	ASSERT_EQ(values[1], 0x93);
	ASSERT_EQ(values[2], 0x78);
	Write12BitLittleEndianSequence(0xABC, values, 2);
	ASSERT_EQ(values[3], 0xBC);
	ASSERT_EQ(values[4], 0xDA);
	Write12BitLittleEndianSequence(0x123, values, 3);
	ASSERT_EQ(values[4], 0x3A);
	ASSERT_EQ(values[5], 0x12);
	uint16_t converted[4]; 
	Read12BitLittleEndianSequence(values, 6, converted, 4);
	ASSERT_EQ(converted[0], 0x345);
	ASSERT_EQ(converted[1], 0x789);
	ASSERT_EQ(converted[2], 0xABC);
	ASSERT_EQ(converted[3], 0x123);
	PASS();
}

SUITE(HelpersTest)
{
	RUN_TEST(Read12BitLittleEndianSequence_Success);
	RUN_TEST(CopyUntilFirstSpace_AllSpaces);
	RUN_TEST(CopyUntilFirstSpace_OneWord);
	RUN_TEST(CopyUntilFirstSpace_TwoWords);
	RUN_TEST(NumberFrom8BitLittleEndianSequence_Success);
	RUN_TEST(NumberTo8BitLittleEndianSequence_Success);
	RUN_TEST(Write12BitLittleEndianSequence_Success);
}