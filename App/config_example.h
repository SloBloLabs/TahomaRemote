#pragma once

constexpr std::string_view WIFI_SSID         = "<WIFI_SSID>";
constexpr std::string_view WIFI_PASSWD       = "<WIFI_PASSWD>";
constexpr std::string_view TAHOMA_HOST       = "<TAHOMA_HOST>";
constexpr std::string_view TAHOMA_AUTH       = "Bearer <TAHOMA_AUTH_TOKEN>";
constexpr ShutterConfig shutterConfigTable[] = {
    {"<ShutterName1>",  "<ShutterURI1>"},
    {"<ShutterName2>",  "<ShutterURI2>"},
    // Add more shutters as needed
};