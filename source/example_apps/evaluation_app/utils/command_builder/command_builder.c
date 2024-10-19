#include "command_builder.h"

uint8_t CalcBCC(const char* data, int startPos, int len) {
    uint8_t bcc = (uint8_t)data[startPos];
    for (int i = 1; i < len; i++) {
        bcc ^= (uint8_t)data[startPos + i];
    }
    return bcc;
}


CommandResult BuildReadCommand(uint16_t regID, int len) {
    CommandResult result;
    result.command[0] = '\0'; // Initialize command array
    result.length = 0;

    if (len > 0xf) {
        len = 0;
    }

    char regIDHex[5]; // 4 digits + null terminator
    if (snprintf(regIDHex, sizeof(regIDHex), "%04X", regID) >= (int) sizeof(regIDHex)) {
        // Handle error if regIDHex is truncated
        // Return an empty result or some error indicator
        return result;
    }

    char lenHex[2]; // 1 digit + null terminator
    if (snprintf(lenHex, sizeof(lenHex), "%X", len) >= (int) sizeof(lenHex)) {
        // Handle error if lenHex is truncated
        return result;
    }

    char cmnd[MAX_METER_COMMAND_SIZE]; // Adjust size as needed
    int written = snprintf(cmnd, sizeof(cmnd), "R%c%s%s%c", S_STX, regIDHex, lenHex, S_ETX);

    if (written < 0 || written >= (int) sizeof(cmnd)) {
        // Handle error if cmnd was truncated
        return result;
    }

    int chksLen = strlen(cmnd);
    uint8_t chkSum = CalcBCC(cmnd, 0, chksLen);

    written = snprintf(result.command, sizeof(result.command), "%c%s%c", S_SOH, cmnd, chkSum);

    if (written < 0 || written >= (int) sizeof(result.command)) {
        // Handle error if result.command was truncated
        return result;
    }

    result.length = strlen(result.command);
    return result;
}

CommandResult BuildWriteCommand(uint16_t regID, char* wrData) {
    CommandResult result;
    result.command[0] = '\0'; // Initialize command array
    result.length = 0;

    char regIDHex[5]; // 4 digits + null terminator
    if (snprintf(regIDHex, sizeof(regIDHex), "%04X", regID) >= (int) sizeof(regIDHex)) {
        // Handle error if regIDHex is truncated
        return result;
    }

    char cmnd[MAX_METER_COMMAND_SIZE]; // Adjust size as needed
    int written = snprintf(cmnd, sizeof(cmnd), "W%c%s(%s)%c", S_STX, regIDHex, wrData, S_ETX);

    if (written < 0 || written >= (int) sizeof(cmnd)) {
        // Handle error if cmnd was truncated
        return result;
    }

    int chksLen = strlen(cmnd);
    uint8_t chkSum = CalcBCC(cmnd, 0, chksLen);

    written = snprintf(result.command, sizeof(result.command), "%c%s%c", S_SOH, cmnd, chkSum);

    if (written < 0 || written >= (int) sizeof(result.command)) {
        // Handle error if result.command was truncated
        return result;
    }

    result.length = strlen(result.command);
    return result;
}