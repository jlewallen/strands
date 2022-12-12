#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <ESPNtpClient.h>
#include <WiFiUdp.h>

#include "palettes.h"
#include "web_server.h"
#include "ws2812fx.h"

#include "static.g.h"

#include "secrets.h"

extern LedDriver leds;

static AsyncWebServer server{ 80 };

WebServer::WebServer() {
}

void WebServer::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(STRANDS_WIFI_NAME);
    WiFi.begin(STRANDS_WIFI_SSID, STRANDS_WIFI_PASSWORD);

    Serial.println("wifi:connecting");

    auto started = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        if (millis() - started > 10000) {
            ESP.restart();
        }
    }

    Serial.println("wifi:connected");

    server.onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "not found"); });

    /*
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404);
        }
    });
    */

    server.on("/", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /");

        AsyncWebServerResponse *response =
            request->beginResponse_P(200, "text/html", www_index_html, www_index_html_len);
        request->send(response);
    });

    server.on("/jquery-3.6.1.min.js", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /jquery-3.6.1.min.js");

        AsyncWebServerResponse *response = request->beginResponse_P(
            200, "application/javascript", www_jquery_3_6_1_min_js, www_jquery_3_6_1_min_js_len);
        request->send(response);
    });

    server.on("/bundle.js", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /bundle.js");

        AsyncWebServerResponse *response =
            request->beginResponse_P(200, "application/javascript", www_bundle_js, www_bundle_js_len);
        request->send(response);
    });

    server.on("/status", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /status");

        /*
        uint8_t macAddr[6];
        WiFi.macAddress(macAddr);
        */

        char status[50] = "{\"version\":\"";
        strcat(status, "?VERSION?");
        strcat(status, "\",\"isRunning\":");
        strcat(status, leds.isRunning() ? "true" : "false");
        strcat(status, "}");

        request->send(200, "application/json", status);
    });

    server.on("/modes.json", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /modes.json");

        String body = "{ \"modes\": [";
        for (auto i = 0u; i < leds.getModeCount(); i++) {
            if (i > 0) {
                body += ", ";
            }
            body += "{ \"key\": " + String(i) + ", \"name\": \"";
            body += leds.getModeName(i);
            body += "\" }";
        }
        body += "] }";

        request->send(200, "application/json", body);
    });

    server.on("/palettes.json", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /palettes.json");

        String body = "{ \"palettes\": [";
        body += "{ \"key\": 0, \"name\": \"None\" }";
        for (auto i = 0u; i < gGradientPaletteCount; i++) {
            body += ", ";
            body += "{ \"key\": " + String(i + 1) + ", \"name\": \"";
            body += gGradientPalettes[i].name;
            body += "\" }";
        }
        body += "] }";

        request->send(200, "application/json", body);
    });

    server.on("/set", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /set");

        if (request->hasParam("c")) {
            auto p = request->getParam("c");
            auto index = (uint32_t)strtol(p->value().c_str(), NULL, 10);
            if (index <= 0xFFFFFF) {
                leds.setColor(index);
            }

            Serial.print("httpd: color is ");
            Serial.println(index);
        }

        if (request->hasParam("p")) {
            auto p = request->getParam("p");
            auto index = (uint8_t)strtol(p->value().c_str(), NULL, 10);
            auto new_palette = index % gGradientPaletteCount;
            Serial.print("httpd: palette is ");
            if (index == 0) {
                Serial.println("NONE");
                leds.setColor(STRANDS_WS2812FX_DEFAULT_COLOR);
            } else {
                auto pal = gGradientPalettes[new_palette - 1];
                Serial.println(pal.name);

                CRGBPalette16 palette{ pal.p };
                leds.setPalette(palette);
            }
        }

        if (request->hasParam("m")) {
            auto p = request->getParam("m");
            auto index = (uint8_t)strtol(p->value().c_str(), NULL, 10);
            leds.setMode(index % leds.getModeCount());

            Serial.print("httpd: mode is ");
            Serial.println(leds.getModeName(leds.getMode()));
        }

        if (request->hasParam("b")) {
            auto b = request->getParam("b");
            if (b->value()[0] == '-') {
                leds.setBrightness(leds.getBrightness() * 0.8);
            } else if (b->value()[0] == ' ') {
                leds.setBrightness(std::min(std::max(leds.getBrightness(), (uint8_t)5) * 1.2, 255.0));
            } else {
                leds.setBrightness((uint8_t)strtol(b->value().c_str(), NULL, 10));
            }

            Serial.print("httpd: brightness is ");
            Serial.println(leds.getBrightness());
        }

        if (request->hasParam("s")) {
            auto s = request->getParam("s");
            if (s->value()[0] == '-') {
                leds.setSpeed(max(leds.getSpeed(), (uint16_t)5) * 1.2);
            } else if (s->value()[0] == ' ') {
                leds.setSpeed(leds.getSpeed() * 0.8);
            } else {
                leds.setSpeed((uint16_t)strtol(s->value().c_str(), NULL, 10));
            }

            Serial.print("httpd: speed is ");
            Serial.println(leds.getSpeed());
        }

        if (request->hasParam("r")) {
            auto p = request->getParam("r");
            if (strcmp(p->value().c_str(), "true") == 0) {
                leds.start();
            } else {
                leds.stop();
            }
        }

        request->send(200, "text/plain", "{}");
    });

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    server.begin();

#if defined(STRANDS_ENABLE_OTA)
    ArduinoOTA.setHostname(STRANDS_WIFI_NAME);

    ArduinoOTA.setPassword(STRANDS_OTA_PASSWORD);

    ArduinoOTA.onStart([]() { Serial.println("ota: start"); });

    ArduinoOTA.onEnd([]() { Serial.println("\nota: end"); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("ota: progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("ota: error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Serial.println("ota: auth failed");
        else if (error == OTA_BEGIN_ERROR)
            Serial.println("ota: begin failed");
        else if (error == OTA_CONNECT_ERROR)
            Serial.println("ota: connect failed");
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println("ota: receive failed");
        else if (error == OTA_END_ERROR)
            Serial.println("ota: end failed");
        delay(5000);
        ESP.restart();
    });

    ArduinoOTA.begin();
#endif

#if defined(STRANDS_ENABLE_NTP)
    if (!MDNS.begin(STRANDS_WIFI_NAME)) {
        Serial.println("mdns: error");
        delay(5000);
        ESP.restart();
    }

    NTP.onNTPSyncEvent([&](NTPEvent_t event) {
        switch (event.event) {
        case timeSyncd:
            Serial.println("ntp: synced");
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
}
