#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <stdexcept>
#include <sys/socket.h>

#define SERVER_PORT 8080

enum Command : uint32_t {
    CMD_HELLO = 0,    // Handshake
    CMD_CONFIG = 1,   // Set configuration
    CMD_DATA = 2,     // Send data
    CMD_START = 3,    // Start processing
    CMD_STATUS = 4,   // Checking status
    CMD_RESULT = 5,   // Requesting result
    CMD_ERROR = 6     // Error indicator
};

#pragma pack(push, 1)
struct PacketHeader {
    uint32_t total_size;
    uint32_t command;
};

struct ConfigPayload {
    uint32_t N;
    uint32_t num_threads;
};
#pragma pack(pop)

// Utility function to read exactly 'n' bytes from socket
inline void recv_full(int sock, void* buf, size_t n) {
    char* p = static_cast<char*>(buf);
    size_t total = 0;
    while (total < n) {
        int bytes = recv(sock, p + total, (int)(n - total), 0);
        if (bytes <= 0) throw std::runtime_error("Connection closed or recv failed during recv_full");
        total += bytes;
    }
}

#endif // PROTOCOL_H
