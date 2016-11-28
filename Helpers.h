#pragma once
#include <stdlib.h>
#include <stdint.h>

void Read12ByteLittleEndianSequence(uint8_t* source, size_t sourceLength, uint16_t* destination, size_t destinationLength);
void WriteTo12ByteLittleEndianSequence(uint16_t number, uint8_t* destination, size_t index);

// assume destination can hold sourceLength + NULL terminator
void CopyUntilFirstSpace(char* source, size_t sourceLength, char* destination);

long NumberFrom8ByteLittleEndianSequence(uint8_t* source, size_t sourceLength);

void NumberTo8ByteLittleEndianSequence(uint32_t number, uint8_t* destination, size_t destinationLength);