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

#include <Arduino.h>

#include <sys/time.h>

#include "gfx/CompositeGraphics.h"
#include "../dali_config.h"
#include "dali_color_theme.h"
#include "dali_gradient.h"
#include "dali_digit.h"
#include "dali_clock.h"

DaliClock::DaliClock() {
    /*struct tm timeinfo;
    timeinfo.tm_sec = 1;
    
    offset_time = mktime(&timeinfo);*/
}

void DaliClock::sync_from_rtc() {
}

void DaliClock::sync_to_rtc() {
}

void DaliClock::set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
}

void DaliClock::set_date(uint8_t month, uint8_t day, uint16_t year) {
}

void DaliClock::set_calender_mode(bool enable) {
    calendar_mode = enable;
}

/*************************** Drawing and Display Functions ****************************/

// Position of individual digits and colons
#define DIGIT_1_POS left_margin + digit_width * 0 + colon_width * 0, digit_top
#define DIGIT_2_POS left_margin + digit_width * 1 + colon_width * 0, digit_top
#define COLON_1_POS left_margin + digit_width * 2 + colon_width * 0, digit_top
#define DIGIT_3_POS left_margin + digit_width * 2 + colon_width * 1, digit_top
#define DIGIT_4_POS left_margin + digit_width * 3 + colon_width * 1, digit_top
#define COLON_2_POS left_margin + digit_width * 4 + colon_width * 1, digit_top
#define DIGIT_5_POS left_margin + digit_width * 4 + colon_width * 2, digit_top
#define DIGIT_6_POS left_margin + digit_width * 5 + colon_width * 2, digit_top

void DaliClock::draw_digit(CompositeGraphics &g, DaliDigit digit, int x, int y, char color) {
    while(digit.draw_row(g, x, y, color)) y++;
}

void DaliClock::draw_digit(CompositeGraphics &g, DaliDigit digit, int x, int y) {
    draw_digit(g, digit, x + 2, y + 2, digit_shadow_color); // Draw the shadow
    draw_digit(g, digit, x    , y    , masking_color); // Draw the digit
}

void DaliClock::draw_gradient_and_shine(CompositeGraphics &g, float i) {
    const int shine = (display_width + digit_height) * i;
    DaliGradient::draw(g, CLOCK_TOP_RECT, digit_gradient_top, digit_gradient_midtop, masking_color, shine);
    DaliGradient::draw(g, CLOCK_BOT_RECT, digit_gradient_midbot, digit_gradient_bottom, masking_color, shine);
}


void DaliClock::draw(CompositeGraphics &g) {
    /*uint16_t blend = (millis() - offset_ms) * 255 / 1000;
    if(blend > 255) {
        // We are done cycling the previous digits, so setup to change to the next
        time_to_digits();
    }
    
    // Draw the display
    draw_digit(g, DaliDigit(old_display[0], new_display[0], blend), DIGIT_1_POS);
    draw_digit(g, DaliDigit(old_display[1], new_display[1], blend), DIGIT_2_POS);
    draw_digit(g, DaliDigit(old_display[2], new_display[2], blend), COLON_1_POS);
    draw_digit(g, DaliDigit(old_display[3], new_display[3], blend), DIGIT_3_POS);
    draw_digit(g, DaliDigit(old_display[4], new_display[4], blend), DIGIT_4_POS);
    draw_digit(g, DaliDigit(old_display[5], new_display[5], blend), COLON_2_POS);
    draw_digit(g, DaliDigit(old_display[6], new_display[6], blend), DIGIT_5_POS);
    draw_digit(g, DaliDigit(old_display[7], new_display[7], blend), DIGIT_6_POS);*/
}

/*************************** Time Conversion Functions ****************************/

void DaliClock::hms_to_str(uint8_t hours, uint8_t minutes, uint8_t seconds, char str[8]) {
    str[0] = hours / 10;
    str[1] = hours % 10;
    str[2] = ':';
    str[3] = minutes / 10;
    str[4] = minutes % 10;
    str[5] = ':';
    str[6] = seconds / 10;
    str[7] = seconds % 10;
}

void DaliClock::mdy_to_str(uint8_t month, uint8_t day, uint8_t year, char str[8]) {
    str[0] = '0' + month / 10;
    str[1] = '0' + month % 10;
    str[2] = '/';
    str[3] = '0' + day   / 10;
    str[4] = '0' + day   % 10;
    str[5] = '/';
    str[6] = '0' + year  / 10;
    str[7] = '0' + year  % 10;
}

void DaliClock::time_to_digits() {
    timeval rawtime;
    
    if(!gettimeofday(&rawtime, nullptr)) {
        // Update the offset
        offset_ms = millis() - rawtime.tv_usec/1000;
        
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
        else
            hms_to_str(t->tm_hour, t->tm_min, t->tm_sec, new_display);
    }
}