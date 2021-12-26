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

#define USE_ATARI_COLORS

#include <Arduino.h>

#include <sys/time.h>

#include "gfx/CompositeGraphics.h"
#include "dali_constants.h"
#include "dali_color_theme.h"
#include "dali_gradient.h"
#include "dali_digit.h"
#include "dali_clock.h"

DaliClock::DaliClock(DaliConfig &_conf) : config(_conf) {
    offset_time = 1;
    strcpy(old_display,"00:00:00");
    strcpy(new_display,"00:00:00");
}

void DaliClock::set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    timeval rawtime;
    if(!gettimeofday(&rawtime, nullptr)) {
        struct tm *t = localtime(&rawtime.tv_sec);
        t->tm_hour = hours;
        t->tm_min  = minutes;
        t->tm_sec  = seconds;
        rawtime.tv_sec = mktime(t);
        settimeofday(&rawtime, nullptr);
    }
}

void DaliClock::set_date(uint8_t month, uint8_t day, uint16_t year) {
    timeval rawtime;
    if(!gettimeofday(&rawtime, nullptr)) {
        struct tm *t = localtime(&rawtime.tv_sec);
        t->tm_mon  = month-1;
        t->tm_mday = day;
        t->tm_year = year - 1900;
        rawtime.tv_sec = mktime(t);
        settimeofday(&rawtime, nullptr);
    }
}

void DaliClock::set_calender_mode(bool enable) {
    calendar_mode = enable;
}

/*************************** Drawing and Display Functions ****************************/

void DaliClock::draw_gradient_and_shine(CompositeGraphics &g, float i) {
    const int shine = (display_width + digit_height) * i;
    DaliGradient::draw(g, CLOCK_TOP_RECT, digit_gradient_top, digit_gradient_midtop, masking_color, shine);
    DaliGradient::draw(g, CLOCK_BOT_RECT, digit_gradient_midbot, digit_gradient_bottom, masking_color, shine);
}


void DaliClock::draw(CompositeGraphics &g) {
    constexpr uint16_t blend_time_ms = 250;

    uint32_t elapsed = millis() - last_change_ms;
    if (elapsed > 1000) {
        last_change_ms = millis();
        // We are done cycling the previous digits, so setup to change to the next
        time_to_digits();
        elapsed = millis() - last_change_ms;
    }

    const uint8_t blend = min(255u, elapsed * 255 / blend_time_ms);

    // Draw the display
    DaliDigit::draw(g, old_display, new_display, blend, left_margin + 2, digit_top + 2, digit_shadow_color); // Draw the shadow
    DaliDigit::draw(g, old_display, new_display, blend, left_margin    , digit_top    , masking_color     ); // Draw the digit
}

/*************************** Time Conversion Functions ****************************/

void DaliClock::hms_to_str(uint8_t hours, uint8_t minutes, uint8_t seconds, char str[9]) {
    str[0] = '0' + hours / 10;
    str[1] = '0' + hours % 10;
    str[2] = ':';
    str[3] = '0' + minutes / 10;
    str[4] = '0' + minutes % 10;
    str[5] = ':';
    str[6] = '0' + seconds / 10;
    str[7] = '0' + seconds % 10;
    str[8] = '\0';
}

void DaliClock::mdy_to_str(uint8_t month, uint8_t day, uint8_t year, char str[9]) {
    str[0] = '0' + month / 10;
    str[1] = '0' + month % 10;
    str[2] = '/';
    str[3] = '0' + day   / 10;
    str[4] = '0' + day   % 10;
    str[5] = '/';
    str[6] = '0' + year  / 10;
    str[7] = '0' + year  % 10;
    str[8] = '\0';
}

void DaliClock::time_to_digits() {
    timeval rawtime;

    if(!gettimeofday(&rawtime, nullptr)) {
        // Sync our transitions to real time

        if (rawtime.tv_usec > 100000)
            last_change_ms -= 100;

        // Swap the displays
        memcpy(old_display, new_display, 8);

        /* In order to animate the time correctly, we need to add one second
         * to the current time. We do some non-standard's compliant math here,
         * but it works on the ESP32
         */
        time_t next_time = rawtime.tv_sec + offset_time; // Not standards compliant
        struct tm *t = localtime( &next_time );
        if(calendar_mode)
            mdy_to_str(t->tm_mon+1, t->tm_mday, t->tm_year % 100, new_display);
        else {
            int hour = t->tm_hour;
            if(!config.mil_time) {
                // Convert to 12 hour clock
                hour= hour % 12;
                if(hour == 0) hour = 12;
            }
            hms_to_str(hour, t->tm_min, t->tm_sec, new_display);
        }

        // Keep a fractionary number corresponding to how much of the day has elapsed
        day_elapsed = float(t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec) / 86400;
    }
}