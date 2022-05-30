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

#define SUPPORT_NTSC 1
#define SUPPORT_PAL  1

// Pin used for toggling touch mode
#define CALENDAR_TOUCH_GPIO 4

#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "time.h"
#include "src/time_zones.h"

#include "esp_pm.h"

#define USE_ATARI_COLORS

#include "src/gfx/CompositeGraphics.h"
#include "src/gfx/CompositeColorOutput.h"
#include "src/gfx/Font.h"
#include "src/gfx/font6x8.h"

#include "src/dali_constants.h"
#include "src/dali_config.h"
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

DaliConfig config;
DaliStatus info;
DaliClock dali(config);
DaliStars stars;
DaliHorizon horizon;
DaliSun sun;
DaliGrid grid;
DaliCity city;
DaliSparkle sparkle[num_sparkles];
DaliColorTheme theme;

WebServer server(80);
DNSServer dnsServer;

#include <soc/rtc.h>

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
        sparkle[i].locate(graphics, period_7s1p - i * sparkle_phase, CLOCK_RECT);
    }
    dali.draw_gradient_and_shine(graphics, period_7s);
    for(int i = 0; i < num_sparkles; i++) {
        sparkle[i].draw(graphics, period_7s1p - i * sparkle_phase);
    }

    //finished drawing, swap back and front buffer to display it
    graphics.end();

    theme.update(dali.get_day_elapsed());
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
    dali.set_calender_mode(reading < smoothed*0.8);
}


/********************************* WEB SERVER *********************************/
constexpr char *ap_ssid = "ESP32 Dali Clock";
constexpr uint32_t wifiTimeout = 10000;
constexpr char * webpage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <title>ESP32 Dali Clock</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {width: max-content;}
            form div {display: flex; gap: 1em; margin: 0.5em 0;}
            label {flex-grow: 1}
            input:not([type = "submit"]), select {
                min-width: 20em;
                box-sizing: border-box;
            }
        </style>
        <script>
            // When the user selects a general location from the list,
            // repopulate it with more specific locations from the server
            async function onFilterChanged(el) {
                if(!el.value) return;
                tz = await fetch("timezones?prefix=" + el.value);
                tz = await tz.json();
                el.innerText = "";
                tz.forEach(x => {
                    o = document.createElement("option");
                    o.innerText = x;
                    el.appendChild(o);
                })
                el.name = "timezone";
                el.removeAttribute("onchange");
            }
        </script>
    </head>
    <body>
        <h1>ESP32 Dali Clock</h1>
        <form action="/config_prefs" method="get">
            <h2>Clock Preferences</h2>
            <div><label for="theme_id">Color Theme:</label>
            <select id="theme_id" name="theme_id">
                <option value="99">Time-of-Day Blend</option>
                <option value="98">Minute Blend</option>
                <option value="0">Night Theme Only</option>
                <option value="1">Dawn Theme Only</option>
                <option value="2">Day Theme Only</option>
                <option value="3">Dusk Theme Only</option>
            </select></div>
            <br>
            <input type="submit" value="Submit">
        </form>
        <form action="/config_time" method="get">
            <h2>Manual Time Selection</h2>
            <div><label for="datetime-local">Time:</label>
            <input type="datetime-local" id="datetime-local" name="datetime-local" step="1"></div>
            </div>
            <div><label for="mil_time">Show 24-hour clock:</label>
            <input type="checkbox" id="mil_time" name="mil_time" checked></div>
            <br>
            <input type="submit" value="Submit">
        </form>
        <form action="/config_wifi" method="get">
            <h2>Network Configuration</h2>
            <div><label for="net_ssid">Network Name:</label>
            <input type="text" id="net_ssid" name="net_ssid"></div>
            <div><label for="net_pass">Network Password:</label>
            <input type="text" id="net_pass" name="net_pass"></div>
            <br>
            <div><label for="ntp_addr">Time Server:</label>
            <input type="text" id="ntp_addr" name="ntp_addr" value="pool.ntp.org"></div>
            <div><label for="location">Location:</label>
            <select id="location" onchange="onFilterChanged(this)">
                <option value="">-- none selected --</option>
                <option>Africa</option>
                <option>America</option>
                <option>Asia</option>
                <option>Antartica</option>
                <option>Arctic</option>
                <option>Asia</option>
                <option>Atlantic</option>
                <option>Australia</option>
                <option>Etc</option>
                <option>Europe</option>
                <option>Indian</option>
                <option>Pacific</option>
            </select></div>
            <br>
            <input type="submit" value="Submit">
        </form>
    </body>
</html>
)rawliteral";

constexpr char * webpage_ok = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <title>ESP32 Dali Clock</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
    </head>
    <body>
        <h1>ESP32 Dali Clock</h1>
        <p>Configuration accepted!</p><br>
        <form>
            <input type="button" value="Okay" onclick="history.back()">
        </form>
    </body>
</html>
)rawliteral";

void wifi_start() {
    xTaskCreatePinnedToCore(wifi_task, "wifi_task", 3*1024, NULL, 0, NULL, 1);
}

bool connectToWirelessAccessPoint() {
    // Read the configuration file if it exists

    if(!SPIFFS.begin(true)) return false;
    if(!config.load()) return false;
    theme.setTheme(config.theme_id);
    if(config.net_ssid.length() == 0) return false;
    info.set("Connecting to " + config.net_ssid + "...");
    WiFi.begin(config.net_ssid.c_str(), config.net_pass.c_str());
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if(millis() - start > wifiTimeout) return false;
    }
    info.set("Getting time from " + config.ntp_addr + "...");
    Serial.print("Timezone is ");
    Serial.print(config.timezone);
    String tz = getTzByLocation(config.timezone);
    Serial.print(". Setting TZ to ");
    Serial.print(tz);
    Serial.println();
    configTimeWithTz(tz, config.ntp_addr);
    delay(1000);
    return true;
}

bool becomeWirelessAccessPoint() {
    info.set(String("Starting access point \"") + ap_ssid + "\"");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid);
    delay(100);
    Serial.print(String("Running AP at" ) + ap_ssid + " with IP address " + WiFi.softAPIP().toString());
    dnsServer.start(53, "*", WiFi.softAPIP());
    return true;
}

void wifi_task(void* arg) {
    // Either join an AP or become an AP
    if(connectToWirelessAccessPoint() || becomeWirelessAccessPoint()) {
        if (WiFi.getMode() == WIFI_AP)
            info.set(String("Join \"") + ap_ssid + "\" and then go to\nhttp://" + WiFi.softAPIP().toString() + " to configure");
        else
            info.set(String("Go to http://") + WiFi.localIP().toString() + " to reconfigure");

        // Start web server
        server.on("/config_prefs", HTTP_GET, [](){
            server.send(200, "text/html", webpage_ok);
            config.set("theme_id", server.arg("theme_id"));
            config.save();
            theme.setTheme(config.theme_id);
        });
        server.on("/config_wifi", HTTP_GET, [](){
            server.send(200, "text/html", webpage_ok);
            if(server.arg("net_ssid") != "") {
                config.set("net_ssid", server.arg("net_ssid"));
                config.set("net_pass", server.arg("net_pass"));
            }
            config.set("ntp_addr", server.arg("ntp_addr"));
            config.set("time_dst", server.arg("time_dst"));
            config.set("timezone", server.arg("timezone"));
            config.save();
            info.set("Rebooting...");
            delay(2000);
            ESP.restart();
        });
        server.on("/config_time", HTTP_GET, [](){
            server.send(200, "text/html", webpage_ok);
            String str = server.arg("datetime-local");
            dali.set_date(str.substring( 5, 7).toInt(), str.substring( 8,10).toInt(), str.substring( 0, 4).toInt());
            dali.set_time(str.substring(11,13).toInt(), str.substring(14,16).toInt(), str.substring(17,19).toInt());
            config.set("mil_time", server.arg("mil_time"));
        });
        server.on("/timezones", HTTP_GET, [](){
            String separator = "";
            String prefix = server.arg("prefix");
            String buff = "[\"";
            // Use chunked send otherwise we run out of memory
            server.setContentLength(CONTENT_LENGTH_UNKNOWN);
            server.send(200, "text/html");
            for(int i = 0;;) {
                const char *location = getLocation(i, prefix.c_str());
                if(!location) break;
                buff += separator + location;
                separator = "\",\n\"";
                if(buff.length() > 100) {
                    server.sendContent(buff);
                    buff = "";
                }
            }
            buff += "\"]";
            server.sendContent(buff);
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
