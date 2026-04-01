#include "TahomaEventParser.h"

void TahomaEventParser::lwParseCallback(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {

    // A new event is detected
    if(jsp->stack_pos == 1
            && lwjson_stack_seq_1(jsp, 0, ARRAY)
            && type == LWJSON_STREAM_TYPE_OBJECT) {
        //printf("New event %ld\n", ++enterEvent);
        _value = -1;
        _isMoving = false;
        _isDeviceStateChangedEvent = false;
        _isClosureStateEvent = false;
        _isMovingStateEvent = false;
        memset(_url, 0, sizeof(_url));
    }

    // Current device is parsed and can be closed
    if(jsp->stack_pos == 1
            && lwjson_stack_seq_1(jsp, 0, ARRAY)
            && type == LWJSON_STREAM_TYPE_OBJECT_END) {
        //printf("Leave event %ld\n", ++leaveEvent);
        // Pull into manager if it is a roller shutter here
        if(_isDeviceStateChangedEvent && _isClosureStateEvent) {
            commitClosureState();
        }
        if(_isDeviceStateChangedEvent && _isMovingStateEvent) {
            commitMovingState();
        }
    }

    // Fetch deviceURL from current event
    if (jsp->stack_pos >= 3
            && lwjson_stack_seq_3(jsp, 0, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[2].meta.name, "deviceURL") == 0)) {
        TRACE_LOG("\n[TahomaEventParser::lwParseCallback] Stack %d: Got url '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[2].meta.name, jsp->data.str.buff);
        char tmpUrl[MAX_URL_SIZE];
        stripSlash(tmpUrl, jsp->data.str.buff);
        strcpy(_url, tmpUrl);
    }

    // Check event type
    if (jsp->stack_pos >= 3
            && lwjson_stack_seq_3(jsp, 0, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[2].meta.name, "name") == 0)) {
        TRACE_LOG("\n[TahomaEventParser::lwParseCallback] Stack %d: Got label '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[2].meta.name, jsp->data.str.buff);
        if((strcmp(jsp->data.str.buff, "DeviceStateChangedEvent")) == 0) {
            _isDeviceStateChangedEvent = true;
        }
    }

    // Fetch state name from current event
    if (jsp->stack_pos >= 6
            && lwjson_stack_seq_6(jsp, 0, ARRAY, OBJECT, KEY, ARRAY, OBJECT, KEY)
            && type == LWJSON_STREAM_TYPE_STRING
            && (strcmp(jsp->stack[2].meta.name, "deviceStates") == 0)
            && (strcmp(jsp->stack[5].meta.name, "name") == 0)) {
        TRACE_LOG("\n[TahomaEventParser::lwParseCallback] Stack %d: Got device state '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[5].meta.name, jsp->data.str.buff);
        if((strcmp(jsp->data.str.buff, "core:ClosureState")) == 0) {
            _isClosureStateEvent = true;
        } else if((strcmp(jsp->data.str.buff, "core:MovingState")) == 0) {
            _isMovingStateEvent = true;
        }
    }

    // Fetch state value from current event
    if(jsp->stack_pos >= 6
            && lwjson_stack_seq_6(jsp, 0, ARRAY, OBJECT, KEY, ARRAY, OBJECT, KEY)
            && (strcmp(jsp->stack[2].meta.name, "deviceStates") == 0)
            && (strcmp(jsp->stack[5].meta.name, "value") == 0)) {
        TRACE_LOG("\n[TahomaEventParser::lwParseCallback] Stack %d: Got device state '%s' with value '%s'\n", jsp->stack_pos, jsp->stack[5].meta.name, jsp->data.str.buff);
        if(type == LWJSON_STREAM_TYPE_NUMBER) {
            _value = atoi(jsp->data.str.buff);
        } else if(type == LWJSON_STREAM_TYPE_TRUE) {
            _isMoving = true;
        } else if(type == LWJSON_STREAM_TYPE_FALSE) {
            _isMoving = false;
        }
    }
}

void TahomaEventParser::commitClosureState() {
    TRACE_LOG("\n[TahomaEventParser::commitClosureState] Committing device %s with closure %d\n", _url, _value);
    Shutter* target = _tahomaManager.getShutter(_url);
    if(target != nullptr) {
        target->setClosureState(_value);
    }
}

void TahomaEventParser::commitMovingState() {
    TRACE_LOG("\n[TahomaEventParser::commitMovingState] Committing device %s with moving state %d\n", _url, _isMoving);
    Shutter* target = _tahomaManager.getShutter(_url);
    if(target != nullptr) {
        target->setMoving(_isMoving);
    }
}
