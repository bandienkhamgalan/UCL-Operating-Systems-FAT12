#include "Helpers.h"
#include <assert.h>
#include <ctype.h>

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

void WriteTo12ByteLittleEndianSequence(uint16_t number, uint8_t* destination, size_t index)
{
	assert(destination != NULL);
	
	uint8_t* firstByte = destination + (index * 3 / 2);
	uint8_t* secondByte = destination + ((index * 3 / 2) + 1);

	if(index % 2 == 0)
	{
		*firstByte = number & 0x0FF;
		*secondByte = *secondByte & 0xF0;
		*secondByte = *secondByte | ((number & 0xF00) >> 8);
	}
	else
	{
		*firstByte = *firstByte & 0x0F;
		*firstByte = *firstByte | ((number & 0x00F) << 4);
		*secondByte = (number & 0xFF0) >> 4;
	}
}

void CopyUntilFirstSpace(char* source, size_t sourceLength, char* destination)
{
	assert(source != NULL);
	assert(destination != NULL);

	size_t index = 0;
	for( ; index < sourceLength ; ++index)
	{
		char character = source[index];
		if(isspace(character))
			break;
		destination[index] = character;
	}
	destination[index] = '\0';
}

long NumberFrom8ByteLittleEndianSequence(uint8_t* source, size_t sourceLength)
{
	assert(source != NULL);

	long toReturn = 0;
	uint8_t* current = source + (sourceLength - 1);
	for(size_t index = 0 ; index < sourceLength ; ++index)
	{
		toReturn = toReturn << 8 | *current;
		--current;
	}
	return toReturn;
}

void NumberTo8ByteLittleEndianSequence(uint32_t number, uint8_t* destination, size_t destinationLength)
{
	assert(destination != NULL);

	uint8_t* current = destination;
	for(size_t index = 0 ; index < destinationLength ; ++index)
	{
		current[index] = number & 0xFF;
		number >>= 8;
	}
}