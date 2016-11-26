#pragma once
#include <stdlib.h>
#include <stdint.h>

void Read12ByteLittleEndianSequence(uint8_t* source, size_t sourceLength, uint16_t* destination, size_t destinationLength);