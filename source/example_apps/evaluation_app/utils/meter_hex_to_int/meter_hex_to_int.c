#include <meter_hex_to_int.h>

unsigned long long extract_hex_value(const char *str) {
    // Find the opening and closing brackets
    const char *start = strchr(str, '(');
    const char *end = strchr(str, ')');

    if (!start || !end || start >= end) {
        // If we don't find the brackets or they are in the wrong order
        return -1; // Error indicator
    }

    // Move the start pointer to the first character after '('
    start++;

    // Calculate the length of the hex substring
    size_t hex_len = end - start;

    // Create a buffer to hold the hex string and add a null terminator
    char hex_value[hex_len + 1];
    strncpy(hex_value, start, hex_len);
    hex_value[hex_len] = '\0';

    // Convert the hex string to an unsigned long long integer
    unsigned long long result = strtoull(hex_value, NULL, 16);

    return result;
}

unsigned long long hex_string_to_int(const char* str){

    return strtoull(str, NULL, 16);   
}