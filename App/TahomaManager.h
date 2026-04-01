#pragma once

#include <array>
#include <string_view>
#include <span>
#include "Dbg.h"
#include "utils.h"
#include "lwjson/lwjson.h"

#define MAX_SHUTTER_SIZE 16
#define MAX_LABEL_SIZE   16
#define MAX_URL_SIZE     32
#define MAX_UICLASS_SIZE 32

struct ShutterConfig {
    const char* label;
    const char* url;
};

class Shutter {
public:
    Shutter() {
        reset();
    }

    inline void reset() {
        memset(_label  , 0, MAX_LABEL_SIZE);
        memset(_url    , 0, MAX_URL_SIZE);
        memset(_uiClass, 0, MAX_UICLASS_SIZE);
        setClosureState(0);
        setTargetClosureState(0);
        setMoving(false);
        setChanged(false);
    }

    inline std::string_view label() {
        return _label;
    }

    inline void setLabel(std::string_view label) {
        strncpy(_label, label.data(), label.length());
    }

    inline std::string_view url() {
        return _url;
    }

    inline void setUrl(std::string_view url) {
        strncpy(_url, url.data(), url.length());
    }

    inline std::string_view uiClass() {
        return _uiClass;
    }

    inline void setUiClass(std::string_view uiClass) {
        strncpy(_uiClass  , uiClass.data()  , uiClass.length());
    }

    inline short closureState() {
        return _closureState;
    }

    inline void setClosureState(const short closureState) {
        _closureState = closureState;
    }

    inline short targetClosureState() {
        return _targetClosureState;
    }

    inline void setTargetClosureState(const short targetClosureState) {
        _targetClosureState = targetClosureState;
    }

    inline bool isMoving() {
        return _isMoving;
    }

    inline void setMoving(const bool isMoving) {
        _isMoving = isMoving;
    }

    inline bool isChanged() {
        return _isChanged;
    }
    
    inline void setChanged(const bool changed) {
        _isChanged = changed;
    }

private:
    char _label[MAX_LABEL_SIZE];
    char _url[MAX_URL_SIZE];
    char _uiClass[MAX_UICLASS_SIZE];
    short _closureState;
    short _targetClosureState;
    bool _isMoving;
    bool _isChanged;
};

class TahomaManager {
public:
    TahomaManager() : _numShutters(0) {
        memset(_eventListenerID, 0, sizeof(_eventListenerID));
    }

    inline bool appendShutter(std::string_view url, std::string_view label, unsigned short closureState = 0) {
        if(_numShutters >= _shutters.max_size()) {
            return false; // full
        }
        Shutter* curShutter = &_shutters[_numShutters];
        curShutter->setLabel(label);
        curShutter->setUrl(url);
        curShutter->setClosureState(closureState);
        _numShutters++;
        return true;
    }

    inline Shutter* getShutter(std::string_view url) {
        for(size_t i = 0; i < _numShutters; ++i) {
            auto& shutter = _shutters[i];
            if(shutter.url() == url) {
                return &shutter;
            }
        }
        return nullptr;
    }

    std::span<Shutter*> getChangedShutters();

    inline std::string_view eventListenerID() {
        return _eventListenerID;
    }

    inline void setEventListenerID(std::string_view eventListenerID) {
        strncpy(_eventListenerID, eventListenerID.data(), eventListenerID.length());
    }

    // array iterators
    auto begin() {
        return _shutters.begin();
    }
    
    auto end() {
        return _shutters.begin() + _numShutters;
    }
    
    const auto begin() const {
        return _shutters.begin();
    }
    
    const auto end() const {
        return _shutters.begin() + _numShutters;
    }

    inline size_t size() {
        return _numShutters;
    }

    inline const uint16_t closureValue() const {
        return _closureValue;
    }

    inline void setClosureValue(const uint16_t value) {
        _closureValue = value;
    }

    inline void updateButtonState(size_t numButton, bool isSet) {
        if(numButton < _numShutters && isSet) {
            Shutter* shutter = &_shutters[numButton];
            shutter->setTargetClosureState(closureValue());
            shutter->setChanged(true);
        }
    }

private:
    std::array<Shutter, MAX_SHUTTER_SIZE> _shutters;
    size_t _numShutters;
    char _eventListenerID[64];
    uint16_t _closureValue;
};
