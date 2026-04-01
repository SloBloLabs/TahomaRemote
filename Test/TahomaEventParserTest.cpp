#include <cstddef>
#include <fstream>
#include <iostream>
#include "lwjson/lwjson.h"
#include "TahomaEventParser.h"

int main(int, char**) {

    char jsonBuffer[4096];
    memset(jsonBuffer, 0, sizeof(jsonBuffer));

    std::ifstream in("../../../TahomaSampleEvent.json");
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    strcpy(jsonBuffer, contents.c_str());

    TahomaManager tahomaManager;
    TahomaEventParser eventParser = TahomaEventParser(tahomaManager);
    printf("size of jsonBuffer: %ld\n", strlen(jsonBuffer));

    for (const char* c = jsonBuffer; *c != '\0'; ++c) {
        eventParser.parse(*c);
    }
    printf("\nFinished parsing Tahoma events.\n");
    
    return 0;
}