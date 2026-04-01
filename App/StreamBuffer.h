#include <cstdint>

typedef struct {
    char buffer[10];        // FIFO buffer
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
void streamBufferFlush(StreamBuffer_t* sb);
uint16_t streamBufferGetCount(StreamBuffer_t* sb);