#include "StreamBuffer.h"
#include <cstring>

void streamBufferInit(StreamBuffer_t* sb) {
    streamBufferFlush(sb);
    sb->size = sizeof(sb->buffer);
}

void streamBufferFlush(StreamBuffer_t* sb) {
    sb->head = 0;
    sb->tail = 0;
    sb->count = 0;
    memset(sb->buffer, 0, sb->size);
}

void streamBufferPush(StreamBuffer_t* sb, char c) {
    // If buffer is full, discard oldest data (overwrite tail)
    if(sb->count >= sb->size) {
        sb->tail = (sb->tail + 1) % sb->size;
    } else {
        sb->count++;
    }
    
    sb->buffer[sb->head] = c;
    sb->head = (sb->head + 1) % sb->size;
}

char streamBufferPop(StreamBuffer_t* sb) {
    if(sb->count == 0) {
        return 0;
    }
    
    char c = sb->buffer[sb->tail];
    sb->tail = (sb->tail + 1) % sb->size;
    sb->count--;
    return c;
}

bool streamBufferContains(StreamBuffer_t* sb, const char* token) {
    if(!token || sb->count == 0) {
        return false;
    }
    
    size_t tokenLen = strlen(token);
    if(tokenLen > sb->count) {
        return false;
    }
    
    // Create a temporary linear view of the buffer
    char tempBuffer[256];
    uint16_t pos = sb->tail;
    
    for(uint16_t i = 0; i < sb->count; i++) {
        tempBuffer[i] = sb->buffer[pos];
        pos = (pos + 1) % sb->size;
    }
    tempBuffer[sb->count] = 0;
    
    // Search for token
    return strstr(tempBuffer, token) != NULL;
}

uint16_t streamBufferGetCount(StreamBuffer_t* sb) {
    return sb->count;
}