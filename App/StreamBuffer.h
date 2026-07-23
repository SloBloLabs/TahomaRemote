#include <cstdint>

#define STREAM_BUFFER_CAPACITY 10

typedef struct {
    char buffer[STREAM_BUFFER_CAPACITY]; // FIFO buffer
    uint16_t head;           // Write position
    uint16_t tail;           // Read position
    uint16_t count;          // Number of valid bytes
    uint16_t size;           // Total buffer size
} StreamBuffer_t;

// Function declarations
void streamBufferInit(StreamBuffer_t* sb);
void streamBufferPush(StreamBuffer_t* sb, char c);
char streamBufferPop(StreamBuffer_t* sb);
bool streamBufferContains(StreamBuffer_t* sb, const char* token);
// Checks for two tokens in a single linearization pass (cheaper than two
// separate streamBufferContains() calls when both need to be checked per byte).
void streamBufferContainsTokens(StreamBuffer_t* sb, const char* tokenA, bool* foundA, const char* tokenB, bool* foundB);
void streamBufferFlush(StreamBuffer_t* sb);
uint16_t streamBufferGetCount(StreamBuffer_t* sb);