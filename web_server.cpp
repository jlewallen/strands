#include <ArduinoOTA.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <ESPNtpClient.h>
#include <WiFiUdp.h>

#include "web_server.h"

#include "fastled.h"
#include "ws2812fx.h"

#include "static.g.h"

#include "secrets.h"

extern LedDriver leds;

static ESP8266WebServer server{ 80 };

void srv_handle_not_found();
void srv_handle_index_html();
void srv_handle_bundle_js();
void srv_handle_modes();
void srv_handle_set();

WebServer::WebServer() {
}

void WebServer::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(STRANDS_WIFI_NAME);
    WiFi.begin(STRANDS_WIFI_SSID, STRANDS_WIFI_PASSWORD);

    Serial.println("wifi:connecting");

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("wifi:failed");
        delay(5000);
        ESP.restart();
    }

    Serial.println("wifi:connected");

    server.onNotFound(srv_handle_not_found);
    server.on("/", srv_handle_index_html);
    server.on("/bundle.js", srv_handle_bundle_js);
    server.on("/modes", srv_handle_modes);
    server.on("/set", srv_handle_set);

    // This applies to AsyncWebServer, I believe.
    // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    server.begin();

#if defined(STRANDS_ENABLE_OTA)
    ArduinoOTA.setHostname(STRANDS_WIFI_NAME);

    ArduinoOTA.onStart([]() { Serial.println("Start"); });

    ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
            Serial.println("End Failed");
        delay(5000);
        ESP.restart();
    });

    ArduinoOTA.begin();
#endif

#if defined(STRANDS_ENABLE_NTP)
    if (!MDNS.begin(STRANDS_WIFI_NAME)) {
        Serial.println("Error setting up MDNS responder!");
        delay(5000);
        ESP.restart();
    }

    NTP.onNTPSyncEvent([&](NTPEvent_t event) {
        switch (event.event) {
        case timeSyncd:
            have_time_ = true;
            break;
        default:
            break;
        }
    });
    NTP.setTimeZone(TZ_America_Los_Angeles);
    NTP.begin("north-america.pool.ntp.org");
#endif
}

void WebServer::service() {
    ArduinoOTA.handle();

    server.handleClient();

    /*
    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    */
}

void srv_handle_not_found() {
    server.send(404, "text/plain", "File Not Found");
}

void srv_handle_index_html() {
    server.send(200, "text/html", www_index_html, www_index_html_len);
}

void srv_handle_bundle_js() {
    server.send(200, "application/javascript", www_bundle_js, www_bundle_js_len);
}

String modes = "";

void modes_setup() {
    modes = "";
    uint8_t num_modes = leds.getModeCount();
    for (uint8_t i = 0; i < num_modes; i++) {
        modes += "<li><a href='#'>";
        modes += leds.getModeName(i);
        modes += "</a></li>";
    }
}

void srv_handle_modes() {
    if (modes.length() == 0) {
        modes_setup();
    }
    server.send(200, "text/plain", modes);
}

void srv_handle_set() {
    for (uint8_t i = 0; i < server.args(); i++) {
        if (server.argName(i) == "c") {
            uint32_t tmp = (uint32_t)strtol(server.arg(i).c_str(), NULL, 10);
            if (tmp <= 0xFFFFFF) {
                leds.setColor(tmp);
            }
        }

        if (server.argName(i) == "m") {
            uint8_t tmp = (uint8_t)strtol(server.arg(i).c_str(), NULL, 10);
            uint8_t new_mode = tmp % leds.getModeCount();
            leds.setMode(new_mode);
            // auto_cycle = false;
            Serial.print("mode is ");
            Serial.println(leds.getModeName(leds.getMode()));
        }

        if (server.argName(i) == "b") {
            if (server.arg(i)[0] == '-') {
                leds.setBrightness(leds.getBrightness() * 0.8);
            } else if (server.arg(i)[0] == ' ') {
                leds.setBrightness(std::min(std::max(leds.getBrightness(), (uint8_t)5) * 1.2, 255.0));
            } else { // set brightness directly
                uint8_t tmp = (uint8_t)strtol(server.arg(i).c_str(), NULL, 10);
                leds.setBrightness(tmp);
            }
            Serial.print("brightness is ");
            Serial.println(leds.getBrightness());
        }

        if (server.argName(i) == "s") {
            if (server.arg(i)[0] == '-') {
                leds.setSpeed(max(leds.getSpeed(), (uint16_t)5) * 1.2);
            } else if (server.arg(i)[0] == ' ') {
                leds.setSpeed(leds.getSpeed() * 0.8);
            } else {
                uint16_t tmp = (uint16_t)strtol(server.arg(i).c_str(), NULL, 10);
                leds.setSpeed(tmp);
            }
            Serial.print("speed is ");
            Serial.println(leds.getSpeed());
        }

        /*
        if (server.argName(i) == "a") {
            if (server.arg(i)[0] == '-') {
                auto_cycle = false;
            } else {
                auto_cycle = true;
                auto_last_change = 0;
            }
        }
        */
    }

    server.send(200, "text/plain", "OK");
}
