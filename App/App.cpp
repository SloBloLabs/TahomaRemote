#include "App.h"
#include "cmsis_os2.h"
#include "main.h"
#include "ESP8266_AT.h"
#include <cstdint>
#include <cstdio>
#include <stddef.h>
#include <string>
#include <string_view>
#include "lwjson/lwjson.h"
#include "TahomaManager.h"
#include "TahomaSetupParser.h"
#include "TahomaEventParser.h"
#include "System.h"
#include "ClosureSlider.h"
#include "LEDDriver.h"
#include "RGBLed.h"
#include "config.h"

#define BUTTON_BRIGHTNESS 0.8f

LEDDriver ledDriver;
__attribute__((section(".w"))) TahomaManager tahomaManager;
ClosureSlider closureSlider;

void resetESP();
void showIP();
void showTahomaVersion();
void runTahomaSetup();
void registerEventListener();
void fetchEvents();
void setClosure();
int  updateShutters();
void comFailure();
void enableButtons();
void disableButtons();

typedef enum {
    BOOTING = 0,
    AT_SUCCESSFUL,
    AT_INITIALIZED,
    WIFI_CONNECTED,
    LISTENER_REGISTERED,
    SETUP_COMPLETED,
    COM_FAILURE
} AppStatus;

AppStatus appStatus;

void uiMain() {

    //System::init();
    closureSlider.init();
    ledDriver.init();

    int curLed = 0;
    size_t numLeds = 5;
    int direction = 1;

    uint32_t curMillis
           , mainLogMillis = 0;
    
    uint16_t closureValue
           , oldClosureValue  = 0;

    while(1) {

        curMillis = System::ticks();

        //static float hue = 0.f;

        closureValue = tahomaManager.closureValue(); // Read closure pot value

        // Log closure value every 2 seconds if changed
        if((curMillis - mainLogMillis > 2000) && (closureValue != oldClosureValue)) {
            mainLogMillis = curMillis;

            oldClosureValue = closureValue;
            DEBUG_LOG("\n[appMain] Closure Value: %d.", closureValue);
        }

        // Update LEDs
        ledDriver.clear();

        // Set slider LED based on closure value
        ledDriver.setColourHSV(RGBLed::SLIDER, 1.0f, 1.f, (100 - closureValue) / 100.0f);

        // Update LED pattern based on app status
        switch(appStatus) {
            case BOOTING: {
                std::array<RGBLed::Code, 9> ledSnake = {
                    RGBLed::BTN3, RGBLed::BTN2, RGBLed::BTN1, RGBLed::BTN0,
                    RGBLed::BTN4, RGBLed::BTN5, RGBLed::BTN6, RGBLed::BTN7,
                    RGBLed::BTN8};
                
                for(size_t i = 0; i < numLeds; ++i) {
                    int ledIndex = static_cast<int>(curLed) - static_cast<int>(i) * direction;
                    
                    // Handle wrapping with proper signed/unsigned handling
                    while(ledIndex < 0) ledIndex += static_cast<int>(ledSnake.size());
                    while(ledIndex >= static_cast<int>(ledSnake.size())) ledIndex -= static_cast<int>(ledSnake.size());
                    
                    float ledHue = i * 80.f;
                    ledDriver.setColourHSV(ledSnake[ledIndex], ledHue, 1.f, BUTTON_BRIGHTNESS);
                    
                }
                curLed = static_cast<int>(curLed) + direction;
                if(curLed >= static_cast<int>(ledSnake.size())) {
                    curLed = static_cast<int>(ledSnake.size()) - 2;
                    direction = -1;
                } else if(curLed < 0) {
                    curLed = 1;
                    direction = 1;
                }
            }
                break;
            case AT_SUCCESSFUL: {
                ledDriver.setColourHSV(RGBLed::BTN4,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN0,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN5,   0.f, 1.f, BUTTON_BRIGHTNESS);
            }
                break;
            case AT_INITIALIZED: {
                ledDriver.setColourHSV(RGBLed::BTN4,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN0,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN5,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN1,  60.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN6,  60.f, 1.f, BUTTON_BRIGHTNESS);
            }
                break;
            case WIFI_CONNECTED: {
                ledDriver.setColourHSV(RGBLed::BTN4,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN0,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN5,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN1,  60.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN6,  60.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN2, 120.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN7, 120.f, 1.f, BUTTON_BRIGHTNESS);
            }
                break;
            case LISTENER_REGISTERED: {
                ledDriver.setColourHSV(RGBLed::BTN4,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN0,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN5,   0.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN1,  60.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN6,  60.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN2, 120.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN7, 120.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN3, 180.f, 1.f, BUTTON_BRIGHTNESS);
                ledDriver.setColourHSV(RGBLed::BTN8, 180.f, 1.f, BUTTON_BRIGHTNESS);
            }
                break;
            case SETUP_COMPLETED: {
                // Render acutal status of shutters
                std::array<RGBLed::Code, 9> leds = {
                    RGBLed::BTN0, RGBLed::BTN1, RGBLed::BTN2, RGBLed::BTN3,
                    RGBLed::BTN4, RGBLed::BTN5, RGBLed::BTN6, RGBLed::BTN7,
                    RGBLed::BTN8};
                
                float brightness = (curMillis % 1000 > 500) ? BUTTON_BRIGHTNESS : 0.f;

                int i = 0;
                for(Shutter s : tahomaManager) {
                    short closureState = s.closureState();
                    float ledHue = 2.4f *closureState + 120;
                    if(ledHue >= 360.f) ledHue -= 360.f;
                    ledDriver.setColourHSV(leds[i++], ledHue, 1.f, s.isMoving() ? brightness : BUTTON_BRIGHTNESS);
                }
            }
                break;
            case COM_FAILURE: {
                float brightness = (curMillis % 1000 > 500) ? BUTTON_BRIGHTNESS : 0.f;
                ledDriver.setColourHSV(RGBLed::BTN0, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN1, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN2, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN3, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN4, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN5, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN6, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN7, 0.f, 1.f, brightness);
                ledDriver.setColourHSV(RGBLed::BTN8, 0.f, 1.f, brightness);
            }
                break;
            default:
                break;
        }

        ledDriver.process();
        osDelay(40);
    }

}

void comMain() {

    disableButtons();

    appStatus = BOOTING;

    osDelay(2000);

    USER_LOG("\n##################################\n## Tahoma Control by SloBlo Labs ##\n##################################");

    for (const auto& cfg : shutterConfigTable) {
        tahomaManager.appendShutter(cfg.url, cfg.label);
    }
    
    USER_LOG("\n[comMain] Initializing ESP8266...");
    resetESP();

    uint32_t curMillis
           , fetchEventMillis = 0;
    
    bool activateFetchEvents = true;
    bool activateUpdateShutters = true;

    enableButtons();
    
    while(1) {

        curMillis = System::ticks();

        appStatus = SETUP_COMPLETED;

        int n = updateShutters();
        if(activateUpdateShutters && n > 0) {
            DEBUG_LOG("\n[comMain] Updated %d shutters at %ld", n, curMillis);
        }
        else if(activateFetchEvents && curMillis - fetchEventMillis > 1500) {
            fetchEventMillis = curMillis;

            fetchEvents();

            uint32_t curTime = System::ticks();

            DEBUG_LOG("\n[comMain] fetchEvents at %ld, took %ld ms.", curTime, curTime - fetchEventMillis);
        }
    }
}

void resetESP() {
    USER_LOG("\n[resetESP] Resetting ESP8266...");
    
    espInit();

    // Test AT startup
    AT_Status status = espAT();
    if (status != ESP8266_OK) {
        comFailure();
    }
    appStatus = AT_SUCCESSFUL;

    // Disable Echo
    if(!ENABLE_DEBUG_LOG && !ENABLE_TRACE_LOG) {
        status = espDisableEcho();
        if (status != ESP8266_OK) {
            comFailure();
        }
    }

    // Set single connection mode
    status = espSetSingleConnectionMode();
    if (status != ESP8266_OK) {
        comFailure();
    }

    // Set no sleep mode
    status = espNoSleep();
    if (status != ESP8266_OK) {
        comFailure();
    }
    appStatus = AT_INITIALIZED;

    // Connect ESP
    USER_LOG("\n[resetESP] Reset device, got IP...");
    status = espConnectWifi();
    if (status != ESP8266_OK) {
        comFailure();
    }
    appStatus = WIFI_CONNECTED;
    USER_LOG("\n[resetESP] ESP Init succeeded.");

    //showIP();
    //showTahomaVersion();
    registerEventListener();
    appStatus = LISTENER_REGISTERED;

    runTahomaSetup();
}

void showIP() {
    char ipBuf[16];
    memset(ipBuf, 0, sizeof(ipBuf));
    AT_Status status = espGetIP();
    if(status != ESP8266_OK) {
      DEBUG_LOG("\n[showIP] Failed to get IP, error %d. Check Debug Logs!", status);
      comFailure();
    }

    // Parse IP from response buffer into ip buffer
    char* start;
    char* end;
    char startToken[64];
    char endToken[32];
    strcpy(startToken, "+CIFSR:STAIP,");
    strcpy(endToken, "\"\r\n");
    if((start = strstr(espResponse.protocolBuffer, startToken)) != NULL) {
        start += strlen("+CIFSR:STAIP,\"");
        // Find the end of the line
        if((end = strstr(start, endToken)) != NULL) {
            strncpy(ipBuf, start, (end - start));
        }
    }
    USER_LOG("\n[showIP] Got IP: %s", ipBuf);
}

void showTahomaVersion() {
    char tahomaVersion[64];
    memset(tahomaVersion, 0, sizeof(tahomaVersion));
    AT_Status status = espGetTahomaVersion();
    if(status != ESP8266_OK) {
      DEBUG_LOG("\n[showTahomaVersion] Failed to get Tahoma Version, error %d. Check Debug Logs!", status);
      comFailure();
    }

    // Parse the JSON response to extract the protocolVersion field
    // Example response: {"protocolVersion":"2025.4.4-5"}
    char* start;
    char* end;
    char startToken[64];
    char endToken[32];
    strcpy(startToken, "{\"protocolVersion\":\"");
    strcpy(endToken, "\"}");
    if((start = strstr(espResponse.bodyBuffer, startToken)) != NULL) {
        start += strlen(startToken);
        // Find the end of the line
        if((end = strstr(start, endToken)) != NULL) {
            size_t versionLen = end - start;
            memmove(tahomaVersion, start, versionLen);
            tahomaVersion[versionLen] = 0; // Null-terminate the string
        }
    }
    USER_LOG("\n[showTahomaVersion] Tahoma Version: %s", tahomaVersion);
}

void runTahomaSetup() {
    TahomaSetupParser setupParser(tahomaManager);

    uint32_t startTime = System::ticks();
    DEBUG_LOG("\n[runTahomaSetup] Beginning Tahoma Setup... at %ld", startTime);
    AT_Status status = espGetTahomaSetup(setupParser);
    uint32_t curTime = System::ticks();
    DEBUG_LOG("\n[runTahomaSetup] Tahoma Setup completed at %ld, took %ld ms.", curTime, curTime - startTime);

    if(status != ESP8266_OK) {
        DEBUG_LOG("\n[runTahomaSetup] Failed to get Tahoma Setup, error %d. Check Debug Logs!", status);
        comFailure();
    }

    TRACE_LOG("\n[runTahomaSetup] Tahoma Setup JSON (%d) bytes received:", espResponse.bodySize);
    if(ENABLE_TRACE_LOG) {
        traceEspResponse();
    }

    DEBUG_LOG("\n[runTahomaSetup] %d roller shutters found.", tahomaManager.size());

    for(auto shutter : tahomaManager) {
        USER_LOG("\n[runTahomaSetup] Shutter %s with URL %s and closure state %d",
            shutter.label().data(), shutter.url().data(), shutter.closureState());
    }

}

void registerEventListener() {
    AT_Status status = espRegisterTahomaListener();
    if(status != ESP8266_OK) {
        DEBUG_LOG("\n[registerEventListener] Failed to get Tahoma Setup, error %d. Check Debug Logs!", status);
        comFailure();
    }

    if(ENABLE_TRACE_LOG) {
        traceEspResponse();
    }

    // Parse the JSON response to extract the eventListenerID field
    // Example response: {"id": "3fa85f64-5717-4562-b3fc-2c963f66afa6"}
    char* start;
    char* end;
    char startToken[64];
    char endToken[32];
    strcpy(startToken, "{\"id\":\"");
    strcpy(endToken, "\"}");
    if((start = strstr(espResponse.bodyBuffer, startToken)) != NULL) {
        start += strlen(startToken);
        // Find the end of the line
        if((end = strstr(start, endToken)) != NULL) {
            tahomaManager.setEventListenerID(std::string_view(start, end - start));
        }
    }
    USER_LOG("\n[registerEventListener] Event listener %s registered.", tahomaManager.eventListenerID().data());

}

void fetchEvents() {
    DEBUG_LOG("\n[fetchEvents] Begin Fetching events from Tahoma...");
    TahomaEventParser parser(tahomaManager);
    AT_Status status = espFetchTahomaEvents(tahomaManager.eventListenerID(), parser);
    if(status != ESP8266_OK) {
        DEBUG_LOG("\n[fetchEvents] Failed to fetch events from listener id %s, error %d. Check Debug Logs!",
            tahomaManager.eventListenerID().data(), status);
        //comFailure();
    }

    if(ENABLE_TRACE_LOG) {
        traceEspResponse();
    }
    DEBUG_LOG("\n[fetchEvents] End Fetching events from Tahoma...");
}

void setClosure() {
//    std::string_view json_payload_pp = R"({
//    "label": "Move Shutter",
//    "actions": [
//        {
//            "commands": [
//                {
//                    "name": "setClosure",
//                    "parameters": [
//                        "40"
//                    ]
//                }
//            ],
//            "deviceURL": "io://2087-8265-6119/9447220"
//        },
//        {
//            "commands": [
//                {
//                    "name": "setClosure",
//                    "parameters": [
//                        "40"
//                    ]
//                }
//            ],
//            "deviceURL": "io://2087-8265-6119/6296541"
//        }
//    ]
//})";
    std::string_view json_payload = "{\"label\":\"Move Shutter\",\"actions\":[{\"commands\":[{\"name\":\"setClosure\",\"parameters\":[\"0\"]}],\"deviceURL\":\"io://2087-8265-6119/9447220\"},{\"commands\":[{\"name\":\"setClosure\",\"parameters\":[\"0\"]}],\"deviceURL\":\"io://2087-8265-6119/6296541\"}]}";
    AT_Status status = espSendClosureRequest(json_payload);
    if(status != ESP8266_OK) {
        DEBUG_LOG("\n[setClosure] Failed to send closure request, error %d. Check Debug Logs!", status);
        comFailure();
    }

    // set closure
    //response=$(curl -k -X 'POST'
    //  --cacert 'overkiz-root-ca-2048.crt'
    //  'https://gateway-2087-8265-6119.local:8443/enduser-mobile-web/1/enduserAPI/exec/apply'
    //  -H 'accept: application/json'
    //  -H 'Authorization: Bearer 685a9f41a38645a7dc54'
    //  -H 'Content-Type: application/json'
    //  -d "$json_payload")
    //echo $response
}

int updateShutters() {
    std::span<Shutter*> changedShutters = tahomaManager.getChangedShutters();
    int numChanged = static_cast<int>(changedShutters.size());
    if(numChanged > 0) {
        char jsonBuffer[1024];
        size_t numShutter = 0;
        strcpy(jsonBuffer, "{\"label\":\"Move Shutter\",\"actions\":[");
        for(auto* shutter : changedShutters) {
            char cmdBuffer[128];
            if(numShutter++ > 0) {
                strcat(jsonBuffer, ",");
            }
            sprintf(cmdBuffer, "{\"commands\":[{\"name\":\"setClosure\",\"parameters\":[\"%d\"]}],\"deviceURL\":\"%s\"}", shutter->targetClosureState(), shutter->url().data());
            strcat(jsonBuffer, cmdBuffer);
            DEBUG_LOG("\n[updateShutters] Changed shutter found: url: %s, label: %s, closureState: %d",
                shutter->url().data(), shutter->label().data(), shutter->targetClosureState());
            shutter->setChanged(false);
        }
        strcat(jsonBuffer, "]}");
        USER_LOG("\n[updateShutters] Sending closure request:\n%s\n", jsonBuffer);
        int attempt = 0;
        AT_Status status;
        do {
            status = espSendClosureRequest(jsonBuffer);
            attempt++;
        }
        while(status != ESP8266_OK && attempt < 3);
        if(status != ESP8266_OK) {
            DEBUG_LOG("\n[updateShutters] Failed to send closure request after 3 attempts, error %d. Check Debug Logs!", status);
            comFailure();
        }
    }
    return numChanged;
}

void comFailure() {
    appStatus = COM_FAILURE;
    espReset();
    osDelay(3000);
}

void appTick() {
  System::tick();
}

void appLEDTxComplete() {
    ledDriver.notifyTxComplete();
}

void appLEDTxError() {
    ledDriver.notifyTxError();
}

void appADCCompleted() {
    uint16_t closureValue = closureSlider.value();
    tahomaManager.setClosureValue(closureValue);
}

void setButton(size_t numButton, bool isSet) {
    //DEBUG_LOG("\n[setButton] Button #%d state changed: %s", numButton, isSet? "true":"false");
    tahomaManager.updateButtonState(numButton, isSet);
}

void enableButtons() {
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_EnableIRQ(EXTI2_IRQn);
  NVIC_EnableIRQ(EXTI3_IRQn);
  NVIC_EnableIRQ(EXTI4_IRQn);
  NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void disableButtons() {
  NVIC_DisableIRQ(EXTI0_IRQn);
  NVIC_DisableIRQ(EXTI1_IRQn);
  NVIC_DisableIRQ(EXTI2_IRQn);
  NVIC_DisableIRQ(EXTI3_IRQn);
  NVIC_DisableIRQ(EXTI4_IRQn);
  NVIC_DisableIRQ(EXTI9_5_IRQn);
}