#include "TahomaManager.h"
#include "ESP8266_AT.h"

class TahomaSetupParser : public IJsonParser {
public:
    TahomaSetupParser(TahomaManager& tahomaManager) : _tahomaManager(tahomaManager) {
        lwjson_stream_init(&_setupStreamParser, &TahomaSetupParser::staticLwParseCallback);
        _setupStreamParser.user_data = this;
    }

    inline lwjsonr_t parse(char c) {
        lwjsonr_t res = lwjson_stream_parse(&_setupStreamParser, c);

        if(res == lwjsonSTREAMINPROG) {
            //printf("Stream Progress\r\n");
        } else if (res == lwjsonSTREAMWAITFIRSTCHAR) {
            DEBUG_LOG("\n[TahomaSetupParser.parse] Waiting first character");
        } else if (res == lwjsonSTREAMDONE) {
            DEBUG_LOG("\n[TahomaSetupParser.parse] Done");
        } else {
            DEBUG_LOG("\n[TahomaSetupParser.parse] Error");
        }
        return res;
    }

private:

    // --- STATIC CALLBACK (C-compatible) ---
    static void staticLwParseCallback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
        // Recover instance from the user_data pointer
        auto* self = static_cast<TahomaSetupParser*>(jsp->user_data);
        if(self != nullptr) {
            self->lwParseCallback(jsp, type);
        }
    }

    void lwParseCallback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type);

    void commit();

    lwjson_stream_parser_t _setupStreamParser;
    TahomaManager& _tahomaManager;

    // Temporary object to collect parsing results
    Shutter _tempShutter;
    short _tempClosureState;

    // Parsing helpers
    bool _withinClosureStateBraces;
};
