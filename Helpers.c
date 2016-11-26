#include "Helpers.h"
#include <assert.h>

void Read12ByteLittleEndianSequence(uint8_t* source, size_t sourceLength, uint16_t* destination, size_t destinationLength)
{
	assert(source != NULL);
	assert(destination != NULL);
	size_t arrayIndex = 0;
	size_t byteIndex = 0;
	uint16_t temp;
	while(1)
	{
		// get 8 bits of current element and lower 4 bits of next element
		if(byteIndex >= sourceLength)
			break;
		temp = source[byteIndex];
		++byteIndex;
		if(byteIndex >= sourceLength)
			break;
		temp = (((uint16_t)source[byteIndex] & 0x0F) << 8) | temp;
		if(arrayIndex >= destinationLength)
			break;
		destination[arrayIndex++] = temp;

		// get higher 4 bits of current element and 8 bits next element 
		temp = ((uint16_t)source[byteIndex] & 0xF0) >> 4;
		++byteIndex;
		if(byteIndex >= sourceLength)
			break;
		temp = ((uint16_t)source[byteIndex] << 4) | temp;
		if(arrayIndex >= destinationLength)
			break;
		destination[arrayIndex++] = temp;
		++byteIndex;
	}
}