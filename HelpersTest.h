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

SUITE(HelpersTest)
{
	RUN_TEST(Read12ByteLittleEndianSequence_Success);
}