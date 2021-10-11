/****************************************************************************
 *   DaliClock by (c) 2021 Marcio Teixeira                                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "time.h"

#define SUPPORT_NTSC 1
#define SUPPORT_PAL  0

#define CALENDAR_TOUCH_GPIO 4

#include "esp_pm.h"

#include "src/gfx/CompositeGraphics.h"
#include "src/gfx/CompositeColorOutput.h"
#include "src/gfx/Font.h"
#include "src/gfx/font6x8.h"

#include "dali_config.h"
#include "src/dali_color_theme.h"
#include "src/dali_digit.h"
#include "src/dali_clock.h"
#include "src/dali_grid.h"
#include "src/dali_city.h"
#include "src/dali_horizon.h"
#include "src/dali_sun.h"
#include "src/dali_sparkle.h"
#include "src/dali_stars.h"
#include "src/dali_status.h"

//Graphics using the fixed resolution for the color graphics
CompositeGraphics graphics(CompositeColorOutput::XRES, CompositeColorOutput::YRES);
//Composite output using the desired mode (PAL/NTSC) and a fixed resolution
CompositeColorOutput composite(CompositeColorOutput::NTSC);

Font<CompositeGraphics> font(6, 8, font6x8::pixels);

DaliStatus info;
DaliClock dali;
DaliStars stars;
DaliHorizon horizon;
DaliSun sun;
DaliGrid grid;
DaliCity city;
DaliSparkle sparkle[num_sparkles];

WebServer server(80);
DNSServer dnsServer;

#include <soc/rtc.h>

int timezone;
bool time_dst;

void get_network_time(String ntpServer) {
    configTime(timezone * 3600, time_dst ? 3600 : 0, ntpServer.c_str());
}

void setup() {
    //highest clockspeed needed
    esp_pm_lock_handle_t powerManagementLock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
    esp_pm_lock_acquire(powerManagementLock);

    Serial.begin(115200);

    wifi_start();
    dali.set_time(12, 59, 0);

    //initializing graphics double buffer
    composite.init();
    graphics.init();
    graphics.setFont(font);
}

void draw() {
    const uint32_t ms = millis();
    #define PERIOD(LENGTH,PHASE) float(uint32_t(ms - PHASE * 1000) % uint32_t(LENGTH * 1000)) / 1000
    const float period_1s   = PERIOD(1,0.0);
    const float period_3s   = PERIOD(3,0.0);
    const float period_7s   = PERIOD(7,0.0);
    const float period_7s1p = PERIOD(7,0.5);

    //clearing background and starting to draw
    graphics.begin(background_color);

    info.draw(graphics);
    grid.draw(graphics, period_1s);
    stars.draw(graphics,period_7s);
    horizon.draw(graphics);
    sun.draw(graphics);
    city.draw(graphics);
    dali.draw(graphics);
    for(int i = 0; i < num_sparkles; i++) {
        //sparkle[i].locate(graphics, period_7s1p - i * sparkle_phase, CLOCK_RECT, 0x0F);
    }
    dali.draw_gradient_and_shine(graphics, period_7s);
    for(int i = 0; i < num_sparkles; i++) {
        //sparkle[i].draw(graphics, period_7s1p - i * sparkle_phase);
    }

    //finished drawing, swap back and front buffer to display it
    graphics.end();
    
    DaliColorTheme::setTheme(float(millis() % 40000)/10000);
}

void loop() {
    draw();
    composite.sendFrameHalfResolution(&graphics.frame);
    delay(10);
    monitorTouch();
}

void monitorTouch() {
    static uint16_t smoothed;
    const uint16_t reading = touchRead(CALENDAR_TOUCH_GPIO);
    smoothed = (9 * smoothed + reading)/10;
    dali.set_calender_mode(reading < smoothed/2);
}

/********************************* WEB SERVER *********************************/
constexpr char *ap_ssid = "ESP32 Dali Clock";
constexpr char *configPath = "/config.txt";
constexpr uint32_t wifiTimeout = 10000;
constexpr char * webpage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <title>ESP32 Dali Clock</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            form {width: max-content;}
            form div {display: flex; gap: 1em;}
            label {flex-grow: 1};
        </style>
    </head>
    <body>
        <h1>ESP32 Dali Clock</h1>
        <form action="/config_wifi" method="get">
            <div><label for="ntp_addr">Time Server:</label>
            <input type="text" id="ntp_addr" name="ntp_addr" value="pool.ntp.org"></div>
            <div><label for="timezone">Time Zone:</label>
            <select id="timezone" name="timezone">
                <option value="0.00">GMT</option>
                <option value="0.00">UTC</option>
                <option value="1.00">ECT</option>
                <option value="2.00">EET</option>
                <option value="2.00">ART</option>
                <option value="3.00">EAT</option>
                <option value="3.30">MET</option>
                <option value="4.00">NET</option>
                <option value="5.00">PLT</option>
                <option value="5.30">IST</option>
                <option value="6.00">BST</option>
                <option value="7.00">VST</option>
                <option value="8.00">CTT</option>
                <option value="9.00">JST</option>
                <option value="9.30">ACT</option>
                <option value="10.00">AET</option>
                <option value="11.00">SST</option>
                <option value="12.00">NST</option>
                <option value="-11.00">MIT</option>
                <option value="-10.00">HST</option>
                <option value="-9.00">AST</option>
                <option value="-8.00">PST</option>
                <option value="-7.00">PNT</option>
                <option value="-7.00">MST</option>
                <option value="-6.00">CST</option>
                <option value="-5.00">EST</option>
                <option value="-5.00">IET</option>
                <option value="-4.00">PRT</option>
                <option value="-3.30">CNT</option>
                <option value="-3.00">AGT</option>
                <option value="-3.00">BET</option>
                <option value="-1.00">CAT</option>
            </select></div>
            <div><label for="time_dst">Daylight Savings Time:</label>
            <input type="checkbox" id="time_dst" name="time_dst"></div>
            <br>
            <div><label for="net_ssid">Network Name:</label>
            <input type="text" id="net_ssid" name="net_ssid"></div>
            <div><label for="net_pass">Network Password:</label>
            <input type="text" id="net_pass" name="net_pass"></div>
            <br>
            <input type="submit" value="Submit">
        </form>
    </body>
</html>
)rawliteral";

void wifi_start() {
    xTaskCreatePinnedToCore(wifi_task, "wifi_task", 3*1024, NULL, 0, NULL, 1);
}

bool connectToWirelessAccessPoint() {
    // If a configuration file exists, connect to the indicated wireless network
    if(!SPIFFS.begin(true)) return false;
    File file = SPIFFS.open(configPath, FILE_READ);
    if (!file) return false;
    String ssid     = file.readStringUntil('\n');
    String pass     = file.readStringUntil('\n');
    String ntp_addr = file.readStringUntil('\n');
    time_dst = file.readStringUntil('\n').startsWith("on");
    timezone = file.readStringUntil('\n').toFloat();
    ssid.trim();
    pass.trim();
    ntp_addr.trim();
    if(ssid.length() == 0) return false;
    info.set("Connecting to " + ssid + "...");
    WiFi.begin(ssid.c_str(), pass.c_str());
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if(millis() - start > wifiTimeout) return false;
    }
    info.set("Getting time from " + ntp_addr + "...");
    get_network_time(ntp_addr);
    dali.sync_from_rtc();
    delay(1000);
    return true;
}

bool becomeWirelessAccessPoint() {
    info.set(String("Failed to connect.\nPlease connect to \"") + ap_ssid + "\" to configure");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid);
    delay(100);
    dnsServer.start(53, "*", WiFi.softAPIP());
}

void wifi_task(void* arg) {
    // Either join an AP or become an AP
    if(connectToWirelessAccessPoint() || becomeWirelessAccessPoint()) {
        if (WiFi.getMode() == WIFI_AP)
            info.set(String("Failed to connect. Please join \"") + ap_ssid + "\" to configure");
        else
            info.set(String("Go to http://") + WiFi.localIP().toString() + " to reconfigure");

        // Start web server
        server.on("/config_wifi", HTTP_GET, [](){
            server.send(200, "text/plain", "Configuration accepted!");
            // Write a configuration file
            File file = SPIFFS.open(configPath, FILE_WRITE);
            file.println(server.arg("net_ssid"));
            file.println(server.arg("net_pass"));
            file.println(server.arg("ntp_addr"));
            file.println(server.arg("time_dst"));
            file.println(server.arg("timezone"));
            file.close();
            info.set("Rebooting...");
            delay(2000);
            ESP.restart();
        });
        server.onNotFound([](){
            server.send(200, "text/html", webpage);
        });
        server.begin();
        while(1) {
            dnsServer.processNextRequest();
            server.handleClient();
            delay(2);//allow the cpu to switch to other tasks
        }
    }
    vTaskDelete( NULL );
}
