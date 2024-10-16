#ifndef _METER_COMMAND_BUILDER_H
#define _METER_COMMAND_BUILDER_H


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define S_STX 0x02
#define S_ETX 0x03
#define S_SOH 0x01

#define MAX_METER_COMMAND_SIZE 256

typedef struct {
    char command[MAX_METER_COMMAND_SIZE]; // Adjust size as needed
    size_t length;
}CommandResult;

/**
 * @brief Calculates the Block Check Character (BCC) by XORing the bytes of the given string from startPos to startPos + len.
 * 
 * @param data buffer containing the data
 * @param startPos starting position of the data (mostly begining of the buffer)
 * @param len  length of the buffer
 * @return uint8_t Bcc value
 */
uint8_t CalcBCC(const char* data, int startPos, int len);

/**
 * @brief Build a command used the read the specified register from the meter
 * 
 * @param regID Register to read (HEX)
 * @param len number of registers to read 
 * @return CommandResult packaged command for sending
 */
CommandResult BuildReadCommand(uint16_t regID, int len);

/**
 * @brief Build a command to enable writing to the meter
 * 
 * @param regID Register to write to 
 * @param wrData Data to write to the meter
 * @return CommandResult  packaged command for sending
 */
CommandResult BuildWriteCommand(uint16_t regID, char* wrData) ;



#endif //_METER_COMMAND_BUILDER_H