#include "TahomaManager.h"
#include "ESP8266_AT.h"

class TahomaEventParser : public IJsonParser {
public:
    TahomaEventParser(TahomaManager& tahomaManager) : _tahomaManager(tahomaManager) {
        lwjson_stream_init(&_setupStreamParser, &TahomaEventParser::staticLwParseCallback);
        _setupStreamParser.user_data = this;
    }

    inline lwjsonr_t parse(char c) {

        lwjsonr_t res = lwjson_stream_parse(&_setupStreamParser, c);

        if(res == lwjsonSTREAMINPROG) {
            //printf("Stream Progress\r\n");
        } else if (res == lwjsonSTREAMWAITFIRSTCHAR) {
            DEBUG_LOG("\n[TahomaEventParser.parse] Waiting first character");
        } else if (res == lwjsonSTREAMDONE) {
            DEBUG_LOG("\n[TahomaEventParser.parse] Done");
        } else {
            DEBUG_LOG("\n[TahomaEventParser.parse] Error");
        }
        return res;
    }

private:
    
    // --- STATIC CALLBACK (C-compatible) ---
    static void staticLwParseCallback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
        // Recover instance from the user_data pointer
        auto* self = static_cast<TahomaEventParser*>(jsp->user_data);
        if(self != nullptr) {
            self->lwParseCallback(jsp, type);
        }
    }

    void lwParseCallback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type);

    void commitClosureState();
    void commitMovingState();

    lwjson_stream_parser_t _setupStreamParser;
    TahomaManager& _tahomaManager;

    // Temporary objects to collect parsing results
    short _value;
    bool _isMoving;
    char _url[32];

    // Parsing helpers
    bool _isDeviceStateChangedEvent;
    bool _isClosureStateEvent;
    bool _isMovingStateEvent;
};