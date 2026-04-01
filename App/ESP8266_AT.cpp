#include "ESP8266_AT.h"
#include "main.h"
#include "Dbg.h"
#include <cstdint>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "stdint.h"
#include "stm32f4xx_ll_utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string_view>
#include "System.h"
#include "cmsis_os2.h"
#include "StreamBuffer.h"
#include "config.h"

AT_ConnectionStatus espConnectionStatus = ESP8266_DISCONNECTED;
__attribute__((section(".ccmram")))        AT_Response espResponse;
__attribute__((section(".ccmram"))) static StreamParser _streamParser;
static StreamBuffer_t rxStreamBuffer;
static bool _connectionEstablished;

static void espInitResponse();
static AT_Status espHttpRequest(std::string_view method, std::string_view request, std::string_view payload = "");
static AT_Status espSendCommand(std::string_view cmd, std::string_view expectedResponse, uint32_t timeout);
static AT_Status espNextChar(char* c);
static int espFetchPacketSize();
static int espFetchHeader();
static AT_Status espCloseConnection();

// Tahoma Constants
constexpr std::string_view HTTP_PROTOCOL  = "SSL";
constexpr short            TAHOMA_PORT    = 8443;
constexpr short            TAHOMA_TIMEOUT = 7200; // in seconds

void espInit() {
    _streamParser = nullptr;
    _connectionEstablished = false;
    streamBufferInit(&rxStreamBuffer);
}

AT_Status espReset(void) {
    USER_LOG("\nInitializing ESP8266...");
    
    espInit();

    USER_LOG("OK");
    TRACE_LOG("\n[espReset] espInit: Stream parser %s at %p", _streamParser == nullptr ? "unset" : "set", &_streamParser);

    // Test AT startup
    AT_Status status = espAT();
    if (status != ESP8266_OK) {
        return status;
    }

    // Disable Echo
    status = espDisableEcho();
    if (status != ESP8266_OK) {
        return status;
    }

    //// Get and display AT version
    //char version[128];
    //memset(version, 0, sizeof(version));
    //status = espGetATVersion();
    //if (status != ESP8266_OK) {
    //    DEBUG_LOG("\n[espReset] Failed to get AT version...");
    //    return status;
    //}
//
    //// Copy version info from espRxBuffer to versionBuffer
    //char* start;
    //char* end;
    //if((start = strstr(espResponse.protocolBuffer, "Bin version:")) != NULL) {
    //    start += strlen("Bin version:");
    //    // Find the end of the line
    //    if((end = strstr(start, "\r\n")) != NULL) {
    //        strncpy(version, start, (end - start));
    //    }
    //}
    //USER_LOG("\nESP AT Version: %s", version);

    // Connect ESP
    USER_LOG("\nReset device, got IP... ");
    status = espConnectWifi();
    if (status != ESP8266_OK) {
        return status;
    }
    USER_LOG("OK");

    status = espSetSingleConnectionMode();
    if (status != ESP8266_OK) {
        return status;
    }

    return ESP8266_OK;
}

AT_Status espAT() {
    // Test AT startup
    USER_LOG("\nTesting communication with ESP... ");
    AT_Status status = espSendCommand(AT_CMD, AT_CMD_RES, 0);
    if(status != ESP8266_OK) {
        DEBUG_LOG("\n[espAT] ESP8266 not responding...");
        return status;
    }
    USER_LOG("OK");
    return ESP8266_OK;
}

AT_Status espDisableEcho() {
    USER_LOG("\nDisable echo... ");
    AT_Status status = espSendCommand(AT_DISABLE_ECHO, AT_DISABLE_ECHO_RES, 0);
    if (status != ESP8266_OK) {
    	DEBUG_LOG("\n[espDisableEcho] Disable echo Command Failed...");
        return status;
    }
    USER_LOG("OK");
    return status;
}

AT_Status espSetSingleConnectionMode() {
    USER_LOG("\nSet single connection mode... ");
    AT_Status status = espSendCommand(AT_SINGLE_CONN, AT_SINGLE_CONN_RES, 0);
    if (status != ESP8266_OK) {
    	DEBUG_LOG("\n[espSetSingleConnectionMode] Set single connection mode Command Failed...");
        return status;
    }
    USER_LOG("OK");
    return status;
}

AT_Status espNoSleep() {
    USER_LOG("\nDisable sleep mode... ");
    AT_Status status = espSendCommand(AT_NO_SLEEP, AT_NO_SLEEP_RES, 0);
    if (status != ESP8266_OK) {
    	DEBUG_LOG("\n[espNoSleep] Disable sleep mode Command Failed...");
        return status;
    }
    USER_LOG("OK");
    return status;
}

static void espInitResponse() {
    // Initialize HTTP response structure
    memset(&espResponse, 0, sizeof(espResponse));
}

void traceBuffer(const char* buffer, size_t size) {
    #define TRACE_BUFFER_SIZE 128
    char traceBuffer[TRACE_BUFFER_SIZE];
    memset(traceBuffer, 0, TRACE_BUFFER_SIZE);
    for(size_t i = 0; i < size; i += (TRACE_BUFFER_SIZE - 1)) {
        strncpy(traceBuffer, buffer + i, TRACE_BUFFER_SIZE - 1); // keep last byte for null terminator
        printf("%s", traceBuffer);
    }
}

void traceEspResponse() {
    printf("\n[traceEspResponse] **** ESP Response Details: ****");
    printf("\nFull response:\n");
    if(espResponse.protocolSize > 0) {
        printf("\nProtocol buffer\n");
        traceBuffer(espResponse.protocolBuffer, espResponse.protocolSize);
    }

    if(espResponse.headerSize > 0) {
        printf("\nHeader buffer\n");
        traceBuffer(espResponse.headerBuffer, espResponse.headerSize);
    }

    if(espResponse.bodySize > 0) {
        printf("\nBody buffer\n");
        traceBuffer(espResponse.bodyBuffer, espResponse.bodySize);
    }
    printf("\n[traceEspResponse] *******************************");
}

AT_Status espConnectWifi() {
    USER_LOG("\nReset device, got IP... ");
    AT_Status status = espSendCommand(AT_RESET, AT_RESET_RES, 5000);
    if (status != ESP8266_OK) {
    	DEBUG_LOG("\n[espConnectWifi] Reset Command Failed...");
        //osDelay(500);
        // Configure ESP Wifi mode
        // 1. Set Wifi Stationary Mode:
        // AT+CWMODE=1
        status = espSendCommand(AT_STATION_MODE, AT_STATION_MODE_RES, 0);

        //osDelay(500);
        if(status == ESP8266_OK) {
            // 2. Connect to Access Point:
            // AT+CWJAP="Lemmy2","password"
            char cmd[64];
            sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID.data(), WIFI_PASSWD.data());
            status = espSendCommand(cmd, "WIFI GOT IP\r\n\r\nOK\r\n", 0);
        }
        return status;
    }
    USER_LOG("\n[espConnectWifi] OK");
    return status;
}

AT_Status espGetATVersion() {
    AT_Status status = espSendCommand(AT_VERSION, AT_VERSION_RES, 0);
    if (status != ESP8266_OK) {
        return status;
    }
    //// Copy version info from espRxBuffer to versionBuffer
    //char* start;
    //char* end;
    //if((start = strstr(espResponse.protocolBuffer, "Bin version:")) != NULL) {
    //    start += strlen("Bin version:");
    //    // Find the end of the line
    //    if((end = strstr(start, "\r\n")) != NULL) {
    //        strncpy(versionBuffer, start, (end - start));
    //    }
    //}
    return ESP8266_OK;
}

AT_ConnectionStatus espGetConnectionStatus(void) {
    return espConnectionStatus;
}

AT_Status espGetIP() {
    AT_Status status = espSendCommand(AT_GETIP, AT_GETIP_RES, 0);
    return status;
}

AT_Status espGetTahomaVersion() {
    AT_Status status = espHttpRequest(TAHOMA_VERSION_METHOD, TAHOMA_VERSION_EP);
    
    if(status != ESP8266_OK) {
        return status;
    }

    //// Parse the JSON response to extract the protocolVersion field
    //// Example response: {"protocolVersion":"2025.4.4-5"}
//
    //char* start;
    //char* end;
    //char* startToken = "{\"protocolVersion\":\"";
    //char* endToken = "\"}";
    //if((start = strstr(versionBuffer, startToken)) != NULL) {
    //    start += strlen(startToken);
    //    // Find the end of the line
    //    if((end = strstr(start, endToken)) != NULL) {
    //        size_t versionLen = end - start;
    //        memmove(versionBuffer, start, versionLen);
    //        versionBuffer[versionLen] = 0; // Null-terminate the string
    //    }
    //}
    return ESP8266_OK;
}

AT_Status espGetTahomaSetup(IJsonParser& setupParser) {
    TRACE_LOG("\n[espGetTahomaSetup] Set stream parser in espGetTahomaSetup");
    _streamParser = [&setupParser](char c) { return setupParser.parse(c); };
    AT_Status status = espHttpRequest(
        "GET",
        "/enduser-mobile-web/1/enduserAPI/setup");
    _streamParser = nullptr;
    TRACE_LOG("\n[espGetTahomaSetup] Reset stream parser in espGetTahomaSetup");
    return status;
}

AT_Status espRegisterTahomaListener() {
    AT_Status status = espHttpRequest("POST", "/enduser-mobile-web/1/enduserAPI/events/register");
    return status;
}

AT_Status espFetchTahomaEvents(std::string_view eventListenerID, IJsonParser& eventParser) {
    char url[128];
    strcpy(url, "/enduser-mobile-web/1/enduserAPI/events/");
    strcat(url, eventListenerID.data());
    strcat(url, "/fetch");

    TRACE_LOG("\n[espFetchTahomaEvents] Set stream parser in espFetchTahomaEvents");
    _streamParser = [&eventParser](char c) { return eventParser.parse(c); };
    AT_Status status = espHttpRequest("POST", url);
    TRACE_LOG("\n[espFetchTahomaEvents] Reset stream parser in espFetchTahomaEvents");
    _streamParser = nullptr;
    return status;
}

AT_Status espSendClosureRequest(std::string_view jsonCommand) {
    AT_Status status = espHttpRequest("POST", "/enduser-mobile-web/1/enduserAPI/exec/apply", jsonCommand);
    return status;
}

static AT_Status espHttpRequest(std::string_view method, std::string_view request, std::string_view payload) {
  
    char cmd[64];
 
    AT_Status status;
    // 1. Start TCP connection (HTTP port 8443)
    // AT+CIPSTART
    if(!_connectionEstablished) {
        DEBUG_LOG("\n[espHttpRequest] Connecting to Tahoma Tahoma...");
        sprintf(cmd, "%s=\"%s\",\"%s\",%d,%d\r\n",
        /*sprintf(cmd, "%s=\"%s\",\"%s\",%d\r\n",*/
            AT_STARTREQ.data(), HTTP_PROTOCOL.data(), TAHOMA_HOST.data(), TAHOMA_PORT, TAHOMA_TIMEOUT);
        status = espSendCommand(cmd, AT_STARTREQ_RES, 0);
        if(status != ESP8266_OK) {
            DEBUG_LOG("\n[espHttpRequest] Failed to start TCP connection...");
            espCloseConnection();
            return status;
        }
        _connectionEstablished = true;
        DEBUG_LOG("\n[espHttpRequest] TCP connection established...");
    }

    // 2. Build HTTPS request
    char httpReq[4096];
    memset(httpReq, 0, sizeof(httpReq));
    
    snprintf(httpReq, sizeof(httpReq),
             "%s %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "User-Agent: ESP-AT/1.0\r\n"
             "Accept: application/json\r\n"
             "%s"
             "Authorization: %s\r\n"
             "Content-Length: %d\r\n"
             "Connection: keep-alive\r\n\r\n%.*s",
             /*"Connection: close\r\n\r\n%.*s",*/
             method.data(), request.data(), TAHOMA_HOST.data(), TAHOMA_PORT,
             payload.length() > 0 ? "Content-Type: application/json\r\n" : "", TAHOMA_AUTH.data(),
             payload.length(), static_cast<int>(payload.length()), payload.data());
    
    // 3. Tell ESP how many bytes are coming over
    // AT+CIPSEND=<bytes>
    DEBUG_LOG("\n[espHttpRequest] Sending HTTP request size...");
    snprintf(cmd, sizeof(cmd), "%s=%d\r\n", AT_REQSEND.data(), strlen(httpReq));

    status = espSendCommand(cmd, ">", 0);
    if (status != ESP8266_OK) {
        DEBUG_LOG("\n[espHttpRequest] Failed to initiate data send, error %d", status);
        return status;
    }

    // 4. Send the actual HTTP request
    // POST <endpoint> HTTP/1.1 ...
    DEBUG_LOG("\n[espHttpRequest] Sending HTTP %s request...", method.data());
    status = espSendCommand(httpReq, AT_REQSEND_RES, 30000);
    if(status != ESP8266_OK) {
        DEBUG_LOG("\n[espHttpRequest] Failed to send HTTP %s request, error %d", method.data(), status);
        TRACE_LOG("%s", espResponse.protocolBuffer);
        return status;
    }
    DEBUG_LOG("\n[espHttpRequest] HTTP %s request sent successfully.", method.data());

    return status;
}

static AT_Status espCloseConnection() {
    AT_Status status = ESP8266_OK;
    if(!_connectionEstablished) {
        status = espSendCommand("AT+CIPCLOSE\r\n", "CLOSED\r\n\r\nOK\r\n", 0);
    }
    _connectionEstablished = false;
    return status;
}

static AT_Status espSendCommand(std::string_view cmd, std::string_view expectedResponse, uint32_t timeout) {
    uint32_t startTime = System::ticks();

    TRACE_LOG("\n[espSendCommand] Stream parser %s at %p",
        _streamParser == nullptr ? "unset" : "set", &_streamParser);

    espInitResponse();
    if(cmd.length() > 0) {
        DEBUG_LOG("\n[espSendCommand] Sending command: %s", cmd.data());
        // Send command
        sendCommand(cmd.data(), cmd.length());
    }

    int isGetOrPost = strncmp(cmd.data(), "GET", 3) == 0 || strncmp(cmd.data(), "POST", 4) == 0;

    char c;
    if(isGetOrPost) {
        int packetSize = espFetchPacketSize();
        int contentLength = espFetchHeader();
        int remainingPacket = packetSize - espResponse.headerSize;
        int remainingContent = contentLength;
        uint32_t timeConsumed = System::ticks() - startTime;

        while(remainingContent > 0 && (timeout > 0 ? timeConsumed < timeout : true)) {
            // Reset body buffer
            memset(espResponse.bodyBuffer, 0, espResponse.bodySize);
            espResponse.bodySize = 0;

            // Read packet data
            while(remainingPacket > 0 && (timeout > 0 ? timeConsumed < timeout : true)) {
                AT_Status status = espNextChar(&c);
                if(status != ESP8266_OK) {
                    DEBUG_LOG("\n[espSendCommand] Failed to read character, error %d", status);
                    return status;
                }
                if(c != 0) {
                    // Store in body buffer
                    espResponse.bodyBuffer[espResponse.bodySize++] = c;
                    espResponse.bodyBuffer[espResponse.bodySize] = 0;
                    remainingPacket--;
                    remainingContent--;
                }
                timeConsumed = System::ticks() - startTime;
            };

            // Apply stream parser if set
            if(_streamParser != nullptr) {
                uint32_t parsingStartTime = System::ticks();
                for(size_t i = 0; i < espResponse.bodySize; ++i) {
                    lwjsonr_t parseResult = _streamParser(espResponse.bodyBuffer[i]);
                    if(parseResult != lwjsonSTREAMDONE && parseResult != lwjsonSTREAMINPROG) {
                        DEBUG_LOG("\n[espSendCommand] Stream parser reported error.");
                        traceEspResponse();
                        return ESP8266_INVALID_RESPONSE;
                    }
                }
                DEBUG_LOG("\n[espSendCommand] Stream parser processed %d bytes in %ld ms.",
                    espResponse.bodySize, System::ticks() - parsingStartTime);
            }

            // Check if more packets are expected
            if(remainingContent > 0) {
                packetSize = espFetchPacketSize();
                if(packetSize < 0) {
                    DEBUG_LOG("\n[espSendCommand] Failed to fetch next packet size.");
                    return ESP8266_INVALID_RESPONSE;
                }
                remainingPacket = packetSize;
            }

            timeConsumed = System::ticks() - startTime;
        };

        if(remainingContent == 0) {
            DEBUG_LOG("\n[espSendCommand] Successfully read full content.");
        } else if(remainingContent < 0) {
            DEBUG_LOG("\n[espSendCommand] Read more content than expected, %d bytes over.", -remainingContent);
            if(ENABLE_TRACE_LOG) {
                traceEspResponse();
            }
            return ESP8266_INVALID_RESPONSE;
        } else {
            DEBUG_LOG("\n[espSendCommand] Timeout reading content, %d bytes remaining.", remainingContent);
            if(ENABLE_TRACE_LOG) {
                traceEspResponse();
            }
            return ESP8266_TIMEOUT;
        }
    } else {
        // Non-HTTP command, wait for expected response

        bool doneReading = false;
        while(!doneReading && (timeout > 0 ? (System::ticks() - startTime) < timeout : true) && (espResponse.protocolSize < sizeof(espResponse.protocolBuffer) - 1)) {
            AT_Status status = espNextChar(&c);
            if(status != ESP8266_OK) {
                DEBUG_LOG("\n[espSendCommand] Failed to read character, error %d", status);
                return status;
            }
            if(c != 0) {
                espResponse.protocolBuffer[espResponse.protocolSize++] = c;

                // Check for expected response
                if(!doneReading && strstr(espResponse.protocolBuffer, expectedResponse.data()) != NULL) {
                    DEBUG_LOG("\n[espSendCommand] Received expected response: %s", expectedResponse.data());
                    doneReading = true; // mark as found
                } else if(strstr(espResponse.protocolBuffer, "ERROR") != NULL) {
                    // Check for ERROR response
                    DEBUG_LOG("\n[espSendCommand] Received ERROR response");
                    return ESP8266_ERROR;
                } else if(strstr(espResponse.protocolBuffer, "busy") != NULL) {
                    // Handle busy response
                    DEBUG_LOG("\n[espSendCommand] Received busy response, delaying before next retry...");
#ifdef osWaitForever
                    osDelay(1500);
#else
                    LL_mDelay(1500);
#endif
                }
            }
        }
    }

    return ESP8266_OK;

/*
    char c;
    bool doneReading = false;
    int isGetOrPost = strncmp(cmd.data(), "GET", 3) == 0 || strncmp(cmd.data(), "POST", 4) == 0;
    bool collectHeader = false;
    int nextChunkSize = 0;
    AT_Status status;
    
    //while(!doneReading && (timeout > 0 ? (System::ticks() - startTime) < timeout : true) && (espResponse.protocolSize < sizeof(espResponse.protocolBuffer) - 1)) {
    do {
        while((c = nextChar()) != 0) {
            if(nextChunkSize-- > 0) {
                // Consume chunk size bytes, first into header buffer, then into body buffer
                if(collectHeader) {
                    espResponse.headerBuffer[espResponse.headerSize++] = c;
                    if(espResponse.headerSize >= 4 && strncmp(&espResponse.headerBuffer[espResponse.headerSize - 4], "\r\n\r\n", 4) == 0) {
                        // End of headers, switch to body
                        collectHeader = false;
                    }
                } else {
                    espResponse.bodyBuffer[espResponse.bodySize++] = c;
                    espResponse.bodyBuffer[espResponse.bodySize] = 0;
                    
                    // Call response stream listener
                    if(_streamParser != nullptr) {
                        lwjsonr_t parseResult = _streamParser(c);
                        if(parseResult != lwjsonSTREAMDONE && parseResult != lwjsonSTREAMINPROG) {
                            DEBUG_LOG("\n[espSendCommand] Stream parser reported error.");
                            traceEspResponse();
                            return ESP8266_INVALID_RESPONSE;
                        }
                    }
                }
            } else {
                espResponse.protocolBuffer[espResponse.protocolSize++] = c;
                if(isGetOrPost) {
                    // For GET requests, look for chunked transfer encoding
                    // Example: +IPD,1234:HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nTransfer-Encoding: chunked\r\n\r\n1A3\r\n{"protocolVersion":"2025.4.4-5"}\r\n0\r\n\r\nCLOSED
                    // We need to parse the chunk size and then read that many bytes into the appropriate buffer
                    // Look for +IPD, which indicates start of data
                    if(espResponse.protocolSize >= 5 && strncmp(&espResponse.protocolBuffer[espResponse.protocolSize - 5], "+IPD,", 5) == 0) {
                        // Prepare for next IPD chunk
                        size_t chunkSizeStart = espResponse.protocolSize;
                        while((c = nextChar()) != ':') {
                            if(c != 0) {
                                if(espResponse.protocolSize == sizeof(espResponse.protocolBuffer) - 1) {
                                    DEBUG_LOG("\n[espSendCommand] Protocol Buffer Overflow");
                                    goto overflow;
                                }
                                //DEBUG_LOG(".%d\n", c);
                                espResponse.protocolBuffer[espResponse.protocolSize++] = c;
                            }
                        }
                        nextChunkSize = atoi(&espResponse.protocolBuffer[chunkSizeStart]);
                        // Consume the colon
                        espResponse.protocolBuffer[espResponse.protocolSize++] = c;
                        if(espResponse.headerSize == 0) {
                            collectHeader = 1; // First collect headers
                        }
                        espResponse.bodySize = 0;

                        TRACE_LOG("\n[espSendCommand] New chunk with size: %d", nextChunkSize);
                    }
                }
            }
        }
        
        // Check for expected response
        if(!doneReading && strstr(espResponse.protocolBuffer, expectedResponse.data()) != NULL) {
            DEBUG_LOG("\n[espSendCommand] Received expected response: %s", expectedResponse.data());
            doneReading = true; // mark as found
            status = ESP8266_OK;
        } else if(strstr(espResponse.protocolBuffer, "ERROR") != NULL) {
            // Check for ERROR response
            DEBUG_LOG("\n[espSendCommand] Received ERROR response");
            status = ESP8266_ERROR;
        } else if(strstr(espResponse.protocolBuffer, "busy") != NULL) {
            // Handle busy response
            DEBUG_LOG("\n[espSendCommand] Received busy response, delaying before next retry...");
#ifdef osWaitForever
            osDelay(1500);
#else
            LL_mDelay(1500);
#endif
        }
    } while(!doneReading && (timeout > 0 ? (System::ticks() - startTime) < timeout : true) && (espResponse.protocolSize < sizeof(espResponse.protocolBuffer) - 1));

    if(doneReading) {
        if(ENABLE_TRACE_LOG) {
            traceEspResponse();
        }
    } else {
        DEBUG_LOG("\n[espSendCommand] Timed out.");
        traceEspResponse();
        status = ESP8266_TIMEOUT;
    }
    
    if(espResponse.protocolSize == 0) {
        DEBUG_LOG("\n[espSendCommand] No response.");
        status = ESP8266_NO_RESPONSE;
    }

    return status;

overflow:
    DEBUG_LOG("\n[espSendCommand] Timeout or no ACK: %s", espResponse.protocolBuffer);
    return ESP8266_TIMEOUT;*/
}

AT_Status espNextChar(char* c) {
    *c = nextChar();
    if(*c == 0) {
        return ESP8266_OK;
    }

    streamBufferPush(&rxStreamBuffer, *c);

    // Check for special tokens
    //if(streamBufferContains(&rxStreamBuffer, "CONNECT\r\n")) {
    //    if(!_connectionEstablished) {
    //        DEBUG_LOG("\n[espNextChar] Connected to remote");
    //    }
    //    _connectionEstablished = true;
    //}

    if(streamBufferContains(&rxStreamBuffer, "CLOSED")) {
        if(_connectionEstablished) {
            DEBUG_LOG("\n[espNextChar] Connection closed by remote");
        }
        _connectionEstablished = false;
    }
    
    if(streamBufferContains(&rxStreamBuffer, "ERROR")) {
        DEBUG_LOG("\n[espNextChar] Error received");
        if(_connectionEstablished) {
            DEBUG_LOG("\n[espNextChar] Closing connection due to error");
            espCloseConnection();
        }
        return ESP8266_ERROR;
    }

    return ESP8266_OK;
}

int espFetchPacketSize() {
    char c;
    size_t sizeIndex = 0;
    bool ipdFound = false;
    int packetSize = 0;

    while(!ipdFound) {
        if(!_connectionEstablished) {
            DEBUG_LOG("\n[espFetchPacketSize] Connection closed");
            return -1;
        }
        AT_Status status = espNextChar(&c);
        if(status != ESP8266_OK) {
            DEBUG_LOG("\n[espFetchPacketSize] Failed to read character, error %d", status);
            return -1;
        }
        if(c != 0) {
            // For GET requests, look for chunked transfer encoding
            // Example: +IPD,1234:HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nTransfer-Encoding: chunked\r\n\r\n1A3\r\n{"protocolVersion":"2025.4.4-5"}\r\n0\r\n\r\nCLOSED
            // We need to parse the chunk size and then read that many bytes into the appropriate buffer
            // Look for +IPD, which indicates start of data
            espResponse.protocolBuffer[espResponse.protocolSize++] = c;
            if(espResponse.protocolSize >= 5 && strncmp(&espResponse.protocolBuffer[espResponse.protocolSize - 5], "+IPD,", 5) == 0) {
                ipdFound = true;
                // Read size
                sizeIndex = espResponse.protocolSize;
                do {
                    status = espNextChar(&c);
                    if(status != ESP8266_OK) {
                        DEBUG_LOG("\n[espFetchPacketSize] Failed to read character, error %d", status);
                        return -1;
                    }
                    if(c != 0 && (espResponse.protocolSize < sizeof(espResponse.protocolBuffer) - 1)) {
                        espResponse.protocolBuffer[espResponse.protocolSize++] = c;
                    }
                } while((c != ':'));
                packetSize = atoi(&espResponse.protocolBuffer[sizeIndex]);
            }
        }
    }
    
    TRACE_LOG("\n[espFetchPacketSize] Packet size: %d", packetSize);
    return packetSize;
}

int espFetchHeader() {
    char c;
    bool headerDone = false;

    while(!headerDone) {
        if(!_connectionEstablished) {
            DEBUG_LOG("\n[espFetchHeader] Connection is closed");
            return -1;
        }
        AT_Status status = espNextChar(&c);
        if(status != ESP8266_OK) {
            DEBUG_LOG("\n[espFetchHeader] Failed to read character, error %d", status);
            return -1;
        }
        if(c != 0) {
            espResponse.headerBuffer[espResponse.headerSize++] = c;
            // Look for end of headers: \r\n\r\n
            if(espResponse.headerSize >= 4 && strncmp(&espResponse.headerBuffer[espResponse.headerSize - 4], "\r\n\r\n", 4) == 0) {
                headerDone = true;
            }
        }
    }

    TRACE_LOG("\n[espFetchHeader] Header size: %d", espResponse.headerSize);

    // Parse Content-Length from headers
    int contentLength = 0;
    char* start;
    char* end;
    if((start = strstr(espResponse.headerBuffer, "Content-Length:")) != NULL) {
        start += strlen("Content-Length:");
        // Find the end of the line
        if((end = strstr(start, "\r\n")) != NULL) {
            char lengthStr[16];
            size_t len = end - start;
            if(len >= sizeof(lengthStr)) {
                len = sizeof(lengthStr) - 1;
            }
            strncpy(lengthStr, start, len);
            lengthStr[len] = 0; // Null-terminate
            contentLength = atoi(lengthStr);
        }
    }

    TRACE_LOG("\n[espFetchHeader] Content-Length: %d", contentLength);
    return contentLength;
}