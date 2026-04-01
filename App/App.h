#include "Dbg.h"
#include <stdbool.h>

#ifdef __cplusplus

#include "ESP8266_AT.h"
extern AT_Response espResponse;

extern "C" {
#endif

void uiMain(void);
void comMain(void);
void appTick(void);
void appLEDTxComplete();
void appLEDTxError();
void appADCCompleted(void);
void setButton(size_t numButton, bool isSet);

#ifdef __cplusplus
}
#endif