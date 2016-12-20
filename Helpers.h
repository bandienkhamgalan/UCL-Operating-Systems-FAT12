/** @file Helpers.h
 *	@author Bandi Enkh-Amgalan
 *  @brief Supporting functions for reading and writing FAT12 file system data */

#pragma once
#include <stdlib.h>
#include <stdint.h>

/** @brief	Parse stream of data holding 12-bit Little endian numbers
 *
 *	This function will read the source buffer as 12-bit Little endian numbers
 *	storing the parsed numbers in the destination buffer, until either all of the
 *	source is parsed or the destination array is filled. 
 *
 *  In order to ensure all of the data in the source buffer is read, caller should
 *	make sure that the destination buffer is large enough.   
 *
 *	@param source array of 8-bit unsigned integers
 *	@param sourceLength length of source array
 *  @param destination array of 16-bit unsigned integers
 *	@param sourceLength length of destination array
 */
void Read12BitLittleEndianSequence(uint8_t* source, size_t sourceLength, uint16_t* destination, size_t destinationLength);

/** @brief	Write a number into a buffer holding 12-bit Little endian numbers at the specified index 
 *
 *	The index i refers to the i-th 12-bit number in the destination buffer.
 *  This function does not do any buffer overflow checks, so caller must make sure not to specify
 *  an index that is out of bounds.  
 *
 *	@param number number to write
 *	@param destination destination array (buffer to write number in)
 *  @param index index to overwrite
 */
void Write12BitLittleEndianSequence(uint16_t number, uint8_t* destination, size_t index);

/** @brief	Copies a FAT directory entry string into a new string
 *
 *  A FAT directory entry string is not NULL terminated, so this function will copy
 *  all the characters in the source string until the first space character
 *  or until the specified length is reached. 
 *
 *  This function does not do any buffer overflow checks, so caller must make sure
 *  that the destination string is large enough to hold the parsed string. 
 *
 *	@param source
 *	@param sourceLength
 *  @param destination
 */
void CopyUntilFirstSpace(char* source, size_t sourceLength, char* destination);

/** @brief	Parse a buffer holding a Little endian number
 *
 *	Note that this function returns a uint32_t so the longest 8-bit sequence that can be parsed
 * 	without integer overflow is four (8 * 4 = 32). 
 *
 *	@param source
 *	@param sourceLength
 *	@return parsed integer
 */
uint32_t NumberFrom8BitLittleEndianSequence(uint8_t* source, size_t sourceLength);

/** @brief	Overwrite a buffer with a Little endian number
 *
 *  This function does not do any buffer overflow checks, so caller must make sure
 *  that the destination buffer is large enough. 
 *
 *	@param number new number to write
 *	@param destination buffer holding a Little endian number
 *	@param destinationLength length of Little endian number in bytes
 */
void NumberTo8BitLittleEndianSequence(uint32_t number, uint8_t* destination, size_t destinationLength);