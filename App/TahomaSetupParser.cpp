#include "TahomaSetupParser.h"
#include "utils.h"

void TahomaSetupParser::lwParseCallback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {

    //{                                     -> stack[0] == Object
    //    "devices":                        -> stack[1] == Key, == "Devices"
    //              [                       -> stack[2] == Array
    //               {                      -> stack[3] == Object (Device 0, Device 1, Device 2, ...)
    //                "deviceURL":          -> stack[4] == Key, = deviceURL
    //                             "io:..." -> stack[5] == String

    // A new device is detected
    if(jsp->stack_pos == 3
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && type == LWJSON_STREAM_TYPE_OBJECT) {
        //TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Enter device %d\n", ++_enterDevice);
        _tempShutter.reset();
        _tempClosureState = -1;
        _withinClosureStateBraces = false;
    }

    // Current device is parsed and can be closed
    if(jsp->stack_pos == 3
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && type == LWJSON_STREAM_TYPE_OBJECT_END) {
        //TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Leave device %d\n", ++_leaveDevice);
        // Pull into manager if it is a roller shutter here
        commit();
    }

    // Fetch deviceURL from current device
    if (jsp->stack_pos >= 4
            && lwjson_stack_seq_5(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "deviceURL") == 0)) {
        TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Stack %d: Got url '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[4].meta.name, jsp->data.str.buff);
        char tmpUrl[MAX_URL_SIZE];
        stripSlash(tmpUrl, jsp->data.str.buff);
        _tempShutter.setUrl(tmpUrl);
    }

    // Fetch label from current device
    if (jsp->stack_pos >= 4
            && lwjson_stack_seq_5(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "label") == 0)) {
        TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Stack %d: Got label '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[4].meta.name, jsp->data.str.buff);
        _tempShutter.setLabel(jsp->data.str.buff);
    }

    // Fetch uiClass from current device
    if (jsp->stack_pos >= 6
            && lwjson_stack_seq_7(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "definition") == 0)
            && (strcmp(jsp->stack[6].meta.name, "uiClass") == 0)) {
        TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Stack %d: Got uiClass '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[6].meta.name, jsp->data.str.buff);
        _tempShutter.setUiClass(jsp->data.str.buff);
    }

    // Detect new states entry
    if(jsp->stack_pos == 6
            && lwjson_stack_seq_6(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY, ARRAY)
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "states") == 0)
            && type == LWJSON_STREAM_TYPE_OBJECT) {
        //TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Enter new state entry...");
        _tempClosureState = -1;
        _withinClosureStateBraces = false;
    }

    // Fetch closure state from current device
    if (jsp->stack_pos >= 7
            && lwjson_stack_seq_8(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_NUMBER
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "states") == 0)
            && (strcmp(jsp->stack[7].meta.name, "value") == 0)) {
        _tempClosureState = atoi(jsp->data.str.buff);
        if(_withinClosureStateBraces) {
            // store closure state
            TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Stack %d: X Got closure state value '%d'\n", jsp->stack_pos, _tempShutter.closureState());
            _tempShutter.setClosureState(_tempClosureState);
        }
        //TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Stack %ld: Got states value '%d'", jsp->stack_pos, _tempClosureState);
    }

    // Check if this is the  closure state from current device
    if (jsp->stack_pos >= 7
            && lwjson_stack_seq_8(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "states") == 0)
            && (strcmp(jsp->stack[7].meta.name, "name") == 0)) {
        if(strcmp(jsp->data.str.buff, "core:ClosureState") == 0) {
            // This is the closure state of current device
            _withinClosureStateBraces = true;
            if(_tempClosureState >= 0) {
                // store closure state
                TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Stack %d: Y Got closure state value '%d'\n", jsp->stack_pos, _tempShutter.closureState());
                _tempShutter.setClosureState(_tempClosureState);
            }
        }
    }

    // Leave states entry
    if(jsp->stack_pos == 6
            && lwjson_stack_seq_6(jsp, 0, OBJECT, KEY, ARRAY, OBJECT, KEY, ARRAY)
            && (strcmp(jsp->stack[1].meta.name, "devices") == 0)
            && (strcmp(jsp->stack[4].meta.name, "states") == 0)
            && type == LWJSON_STREAM_TYPE_OBJECT_END) {
        TRACE_LOG("\n[TahomaSetupParser::lwParseCallback] Leave state entry...");
        _tempClosureState = -1;
        _withinClosureStateBraces = false;
    }
}

void TahomaSetupParser::commit() {
    TRACE_LOG("\n[TahomaSetupParser::commit] Committing device %s... ", _tempShutter.label().data());
    if(_tempShutter.uiClass() == "RollerShutter") {
        //_tahomaManagerappendShutter(_tempShutter.url(), _tempShutter.label(), _tempShutter.closureState());
        Shutter* shutter = _tahomaManager.getShutter(_tempShutter.url());
        if(shutter) {
            shutter->setLabel(_tempShutter.label());
            shutter->setClosureState(_tempShutter.closureState());
        }
        TRACE_LOG("\n[TahomaSetupParser::commit] OK");
    } else {
        TRACE_LOG("\n[TahomaSetupParser::commit] DENIED, not a RollerShutter (uiClass=%s)", _tempShutter.uiClass().data());
    }
}
