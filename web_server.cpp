#include <ArduinoOTA.h>

#include <ESP8266HTTPClient.h>
#if defined(STRANDS_WEB_SYNC)
#include <ESP8266WebServer.h>
#else
#include <ESPAsyncWebServer.h>
#endif
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

#if defined(STRANDS_WEB_SYNC)
static ESP8266WebServer server{ 80 };

void srv_handle_not_found();
void srv_handle_index_html();
void srv_handle_bundle_js();
void srv_handle_modes();
void srv_handle_set();
#else
static AsyncWebServer server{ 80 };
#endif

WebServer::WebServer() {
}

void WebServer::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(STRANDS_WIFI_NAME);
    WiFi.begin(STRANDS_WIFI_SSID, STRANDS_WIFI_PASSWORD);

    Serial.println("wifi:connecting");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    /*
      while (WiFi.waitForConnectResult() != WL_CONNECTED) {
          Serial.println("wifi:failed");
          delay(5000);
          ESP.restart();
      }
  */
    Serial.println("wifi:connected");

#if defined(STRANDS_WEB_SYNC)
    server.onNotFound(srv_handle_not_found);
    server.on("/", srv_handle_index_html);
    server.on("/bundle.js", srv_handle_bundle_js);
    server.on("/modes", srv_handle_modes);
    server.on("/set", srv_handle_set);
#else
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

    server.on("/bundle.js", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /bundle.js");

        AsyncWebServerResponse *response =
            request->beginResponse_P(200, "application/javascript", www_bundle_js, www_bundle_js_len);
        request->send(response);
    });

    server.on("/status", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /status");

        if (request->hasParam("running")) {
            AsyncWebParameter *p = request->getParam("running");
            const char *running = p->value().c_str();
            if (strcmp(running, "true") == 0) {
                // ws2812fx_.start();
            } else {
                // ws2812fx_.stop();
            }
        }

        /*
        if (request->hasParam("auxFunc")) {
            AsyncWebParameter *p = request->getParam("auxFunc");
            int auxFuncIndex = atoi(p->value().c_str());
            size_t customAuxFuncSize = sizeof(customAuxFunc) / sizeof(customAuxFunc[0]);
            if (auxFuncIndex >= 0 && (size_t)auxFuncIndex < customAuxFuncSize) {
                customAuxFunc[auxFuncIndex]();
            }
        }
        */

        char status[50] = "{\"version\":\"";
        strcat(status, "?VERSION?");
        strcat(status, "\",\"isRunning\":");
        strcat(status, leds.isRunning() ? "true" : "false");
        strcat(status, "}");

        request->send(200, "application/json", status);
    });

    // TODO JSON
    server.on("/modes", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /modes");

        String modes = "";
        uint8_t num_modes = leds.getModeCount();
        for (uint8_t i = 0; i < num_modes; i++) {
            modes += "<li><a href='#'>";
            modes += leds.getModeName(i);
            modes += "</a></li>";
        }

        request->send(200, "text/plain", modes);
    });

    server.on("/set", [](AsyncWebServerRequest *request) {
        Serial.println("httpd: serving /set");

        if (request->hasParam("c")) {
            AsyncWebParameter *p = request->getParam("c");
            uint32_t tmp = (uint32_t)strtol(p->value().c_str(), NULL, 10);
            if (tmp <= 0xFFFFFF) {
                leds.setColor(tmp);
            }
        }

        if (request->hasParam("m")) {
            AsyncWebParameter *p = request->getParam("m");
            uint8_t tmp = (uint8_t)strtol(p->value().c_str(), NULL, 10);
            uint8_t new_mode = tmp % leds.getModeCount();
            leds.setMode(new_mode);

            Serial.print("httpd: ");
            Serial.print("mode is ");
            Serial.println(leds.getModeName(leds.getMode()));
        }

        request->send(200, "text/plain", "{}");
    });

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
#endif

    server.begin();

    /*
    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    */

#if defined(STRANDS_ENABLE_OTA)
    ArduinoOTA.setHostname(STRANDS_WIFI_NAME);

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

#if defined(STRANDS_WEB_SYNC)
    server.handleClient();
#endif
}

#if defined(STRANDS_WEB_SYNC)

void srv_handle_not_found() {
    server.send(404, "text/plain", "not found");
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

#endif