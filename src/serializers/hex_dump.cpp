#include "json_serializers.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

// ============================================================================
// Binary Hex Dump Utility Implementation
// ============================================================================

/**
 * Generate a hexadecimal dump of a binary file
 *
 * Reads the file in chunks and formats each byte as two-digit uppercase hex.
 * Output is formatted with 16 bytes per line, space-separated.
 *
 * Example output:
 *   CF FA ED FE 07 00 00 01 03 00 00 00 02 00 00 00
 *   10 00 00 00 70 04 00 00 85 00 20 00 00 00 00 00
 *   ...
 *
 * @param executablePath Path to the binary executable file
 * @return Formatted hex dump string, or empty string on error
 */
std::string generateHexDump(const std::string& executablePath) {
    // Open file in binary mode
    std::ifstream file(executablePath, std::ios::binary | std::ios::ate);

    if (!file) {
        std::cerr << "Error: Could not open file for hex dump: " << executablePath << std::endl;
        return "";
    }

    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == 0) {
        std::cerr << "Warning: File is empty: " << executablePath << std::endl;
        return "";
    }

    std::ostringstream hexOutput;
    hexOutput << std::hex << std::uppercase << std::setfill('0');

    const int BYTES_PER_LINE = 16;
    int bytesInCurrentLine = 0;

    // Read file byte by byte
    char byte;
    while (file.get(byte)) {
        // Convert byte to unsigned to avoid sign extension issues
        unsigned char ubyte = static_cast<unsigned char>(byte);

        // Add space before byte (except at start of line)
        if (bytesInCurrentLine > 0) {
            hexOutput << ' ';
        }

        // Write byte as two-digit hex
        hexOutput << std::setw(2) << static_cast<int>(ubyte);

        bytesInCurrentLine++;

        // Start new line after 16 bytes
        if (bytesInCurrentLine == BYTES_PER_LINE) {
            hexOutput << '\n';
            bytesInCurrentLine = 0;
        }
    }

    // Add final newline if last line wasn't complete
    if (bytesInCurrentLine > 0) {
        hexOutput << '\n';
    }

    file.close();

    return hexOutput.str();
}
