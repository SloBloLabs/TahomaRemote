#pragma once

#include <stdio.h>
#include <functional>
#include <string_view>
#include "lwjson/lwjson.h"

typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT,
    ESP8266_BUSY,
    ESP8266_NO_RESPONSE,
    ESP8266_INVALID_RESPONSE,
    ESP8266_NOT_INITIALIZED,
    ESP8266_ALREADY_CONNECTED,
    ESP8266_NOT_CONNECTED,
    ESP8266_DNS_FAILURE,
    ESP8266_SEND_FAILURE,
    ESP8266_RECEIVE_FAILURE
} AT_Status;

typedef enum {
    ESP8266_DISCONNECTED = 0,
    ESP8266_CONNECTED_NO_IP,
    ESP8266_CONNECTED_IP_ACQUIRED
} AT_ConnectionStatus;

typedef struct {
    size_t protocolSize;
    size_t headerSize;
    size_t bodySize; // Added field for body size
    char protocolBuffer[1024];
    char headerBuffer[1024];
    char bodyBuffer[4096];
} AT_Response;

typedef std::function<lwjsonr_t(char& c)> StreamParser;

extern AT_ConnectionStatus espConnectionStatus;

inline constexpr std::string_view AT_CMD                 = "AT\r\n";
inline constexpr std::string_view AT_CMD_RES             = "OK\r\n";

inline constexpr std::string_view AT_DISABLE_ECHO        = "ATE0\r\n";
inline constexpr std::string_view AT_DISABLE_ECHO_RES    = "OK\r\n";

inline constexpr std::string_view AT_SINGLE_CONN         = "AT+CIPMUX=0\r\n";
inline constexpr std::string_view AT_SINGLE_CONN_RES     = "OK\r\n";

inline constexpr std::string_view AT_NO_SLEEP            = "AT+SLEEP=0\r\n";
inline constexpr std::string_view AT_NO_SLEEP_RES        = "OK\r\n";

inline constexpr std::string_view AT_STATION_MODE        = "AT+CWMODE=1\r\n";
inline constexpr std::string_view AT_STATION_MODE_RES    = "OK\r\n";

inline constexpr std::string_view AT_VERSION             = "AT+GMR\r\n";
inline constexpr std::string_view AT_VERSION_RES         = "OK\r\n";

inline constexpr std::string_view AT_RESET               = "AT+RST\r\n";
inline constexpr std::string_view AT_RESET_RES           = "WIFI GOT IP\r\n";

inline constexpr std::string_view AT_GETIP               = "AT+CIFSR\r\n";
inline constexpr std::string_view AT_GETIP_RES           = "OK\r\n";

inline constexpr std::string_view AT_STARTREQ            = "AT+CIPSTART";
inline constexpr std::string_view AT_STARTREQ_RES        = "CONNECT\r\n\r\nOK\r\n";
inline constexpr std::string_view AT_REQSEND             = "AT+CIPSEND";
inline constexpr std::string_view AT_REQSEND_RES         = "CLOSED\r\n";


inline constexpr std::string_view TAHOMA_VERSION_METHOD   = "GET";
inline constexpr std::string_view TAHOMA_VERSION_EP       = "/enduser-mobile-web/1/enduserAPI/apiVersion";

class IJsonParser {
public:
    virtual ~IJsonParser() {}
    virtual lwjsonr_t parse(char c) = 0;
};

void espInit();
AT_Status espReset();
AT_Status espAT();
AT_Status espDisableEcho();
AT_Status espSetSingleConnectionMode();
AT_Status espNoSleep();
AT_Status espConnectWifi();
AT_Status espGetIP();
AT_Status espGetATVersion();
AT_Status espGetTahomaVersion();
AT_Status espGetTahomaSetup(IJsonParser& setupParser);
AT_Status espRegisterTahomaListener();
AT_Status espFetchTahomaEvents(std::string_view eventListenerID, IJsonParser& eventParser);
AT_Status espSendClosureRequest(std::string_view jsonCommand);
AT_ConnectionStatus espGetConnectionStatus();
void traceEspResponse();
void traceBuffer(const char* buffer, size_t size);
