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

#pragma once

#include <SPIFFS.h>

class DaliConfig {
    public:
        static constexpr char *configPath = "/config.txt";

        int    theme_id = 0;
        String net_ssid;
        String net_pass;
        String ntp_addr;
        String timezone = "UTC0";
        bool   mil_time = true;

    /**
     * If the key exists in the config structure, assign it,
     * otherwise return false.
     */
    bool set(String key, String val) {
        if(     key == "theme_id") theme_id = val.toInt();
        else if(key == "mil_time") mil_time = val == "on";
        else if(key == "net_ssid") net_ssid = val;
        else if(key == "net_pass") net_pass = val;
        else if(key == "ntp_addr") ntp_addr = val;
        else if(key == "timezone") timezone = val;
        else return false;
        return true;
    }

    /**
     * Parses a single key value pair by splitting the line
     * at the ":" character.
     */
    bool set(String line) {
        const int delim = line.indexOf(":");
        if(delim == -1) return false;
        String key = line.substring(0, delim);
        String val = line.substring(delim + 1);
        key.trim();
        val.trim();
        return set(key, val);
    }

    void dump(Stream &config) {
        config.print("theme_id:"); config.println(theme_id);
        config.print("mil_time:"); config.println(mil_time ? "on" : "off");
        config.print("net_ssid:"); config.println(net_ssid);
        config.print("net_pass:"); config.println(net_pass);
        config.print("ntp_addr:"); config.println(ntp_addr);
        config.print("timezone:"); config.println(timezone);
    }

    /**
     * Loads the config file from the SPIFFS. Returns true
     * on success.
     */
    bool load() {
        if(!SPIFFS.exists(configPath)) {
            Serial.println("no config file");
            return false;
        }
        File config = SPIFFS.open(configPath, FILE_READ);
        if(!config) {
            Serial.println("config load failed. using defaults");
            return false;
        }
        // Pass a line at a time to the "set" routine.
        for(String line; (line = config.readStringUntil('\n')).length(); set(line));
        config.close();
        return true;
    }

    /**
     * Saves the config file to the SPIFFS. Returns true
     * on success.
     */
    bool save() {
        File config = SPIFFS.open(configPath, FILE_WRITE);
        if(!config) {
            Serial.println("config save failed");
            return false;
        }
        dump(config);
        config.close();
        return true;
    }
};
