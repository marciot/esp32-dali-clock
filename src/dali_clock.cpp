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
#include "gfx/CompositeGraphics.h"
#include "../dali_config.h"
#include "dali_color_theme.h"
#include "dali_gradient.h"
#include "dali_digit.h"
#include "dali_clock.h"

void DaliClock::sync_from_rtc() {
    time_t rawtime;
    time( &rawtime );

    struct tm *t = localtime( &rawtime );
    set_time(t->tm_hour, t->tm_min, t->tm_sec);
    set_date(t->tm_mon+1, t->tm_mday, t->tm_year % 100);
}

void DaliClock::sync_to_rtc() {
    uint8_t h, m, s, month, day, year;
    get_time(h, m, s);
    get_date(month, day, year);
    //DS1302::set(h, m, s, month, day, year, 0);
}

void DaliClock::set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    offset = hms_to_ms(hours, minutes, seconds) - millis();
}

void DaliClock::get_time(uint8_t &hours, uint8_t &minutes, float &seconds) const {
    ms_to_hms(millis() + offset, hours, minutes, seconds);
}

void DaliClock::get_time(uint8_t &hours, uint8_t &minutes, uint8_t &seconds) const {
    float f;
    ms_to_hms(millis() + offset, hours, minutes, f);
    seconds = f;
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

void DaliClock::draw_morphed_digit(CompositeGraphics &g, const float digits[6], const uint8_t digit_limit[6]) {
    draw_digit(g, DaliDigit(digits[0], digit_limit[0]), DIGIT_1_POS);
    draw_digit(g, DaliDigit(digits[1], digit_limit[1]), DIGIT_2_POS);
    draw_digit(g, DaliDigit(digits[2], digit_limit[2]), DIGIT_3_POS);
    draw_digit(g, DaliDigit(digits[3], digit_limit[3]), DIGIT_4_POS);
    draw_digit(g, DaliDigit(digits[4], digit_limit[4]), DIGIT_5_POS);
    draw_digit(g, DaliDigit(digits[5], digit_limit[5]), DIGIT_6_POS);
}

void DaliClock::draw_morphed_digit(CompositeGraphics &g, const uint8_t d1[6], const uint8_t d2[6], uint8_t blend) {
    draw_digit(g, DaliDigit(d1[0], d2[0], blend), DIGIT_1_POS);
    draw_digit(g, DaliDigit(d1[1], d2[1], blend), DIGIT_2_POS);
    draw_digit(g, DaliDigit(d1[2], d2[2], blend), DIGIT_3_POS);
    draw_digit(g, DaliDigit(d1[3], d2[3], blend), DIGIT_4_POS);
    draw_digit(g, DaliDigit(d1[4], d2[4], blend), DIGIT_5_POS);
    draw_digit(g, DaliDigit(d1[5], d2[5], blend), DIGIT_6_POS);
}

void DaliClock::draw_morphed_separator(CompositeGraphics &g, uint8_t blend) {
    draw_digit(g, DaliDigit(DaliDigit::COLON, DaliDigit::DASH, blend), COLON_1_POS);
    draw_digit(g, DaliDigit(DaliDigit::COLON, DaliDigit::DASH, blend), COLON_2_POS);
}

void DaliClock::draw_gradient_and_shine(CompositeGraphics &g, float i) {
    const int shine = (display_width + digit_height) * i;
    DaliGradient::draw(g, CLOCK_TOP_RECT, digit_gradient_top, digit_gradient_midtop, masking_color, shine);
    DaliGradient::draw(g, CLOCK_BOT_RECT, digit_gradient_midbot, digit_gradient_bottom, masking_color, shine);
}

void DaliClock::draw(CompositeGraphics &g) {
    uint8_t month, day, year, hours, minutes;
    float   seconds;
    get_time(hours, minutes, seconds);

    animate_calendar_mode(seconds);

    if(calendar_blend == 0) {
        // Time display
        float   time[6];
        uint8_t time_limits[6];
        hms_to_morphed_digits(hours, minutes, seconds, time);
        get_clock_digit_limits(hours, time_limits);
        draw_morphed_digit(g, time, time_limits);
        draw_morphed_separator(g, 0);
    } else {
        uint8_t month, day, year;
        get_date(month, day, year);

        if(calendar_blend == 255) {
            // Calendar display
            uint8_t date_limits[6];
            float date[6];
            mdy_to_digits(month, day, year, date);
            get_calendar_digit_limits(date_limits);
            draw_morphed_digit(g, date, date_limits);
            draw_morphed_separator(g, 255);
        } else {
            // Transition between time and calendar display
            uint8_t time[6], date[6];
            mdy_to_digits(month, day, year, date);
            hms_to_digits(hours, minutes, seconds, time);
            draw_morphed_digit(g, time, date, calendar_blend);
            draw_morphed_separator(g, calendar_blend);
        }
    }
}

/*************************** Time Conversion Functions ****************************/

void DaliClock::get_clock_digit_limits(uint8_t hours, uint8_t digit_limit[6]) {
    digit_limit[0] = 2;
    digit_limit[1] = hours > 19 ? 4 : 9;
    digit_limit[2] = 5;
    digit_limit[3] = 9;
    digit_limit[4] = 5;
    digit_limit[5] = 9;
}

void DaliClock::ms_to_hms(uint32_t ms, uint8_t &hours, uint8_t &minutes, float &seconds) {
    seconds = fmod(ms, 60000)/1000;
    minutes = (ms /   60000) % 60;
    hours   = (ms / 3600000) % 24;
}

uint32_t DaliClock::hms_to_ms(uint8_t hours, uint8_t minutes, float seconds) {
    return seconds * 1000 + minutes * 60000 + hours * 3600000;
}

void DaliClock::ms_to_digits(uint32_t ms, uint8_t digits[6]) {
    uint8_t hours, minutes;
    float seconds;
    ms_to_hms(ms, hours, minutes, seconds);
    hms_to_digits(hours, minutes, seconds, digits);
}

void DaliClock::hms_to_digits(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t digits[6]) {
    digits[0] = hours / 10;
    digits[1] = hours % 10;
    digits[2] = minutes / 10;
    digits[3] = minutes % 10;
    digits[4] = seconds / 10;
    digits[5] = seconds % 10;
}

void DaliClock::digits_to_hms(const uint8_t digits[6], uint8_t &hours, uint8_t &minutes, uint8_t &seconds) {
    hours   = 10 * digits[0] + digits[1];
    minutes = 10 * digits[2] + digits[3];
    seconds = 10 * digits[4] + digits[5];
}

// This function returns floats corresponding to morphed digits, including the
// special case of when multiple digits are being morphed during a carry.
void DaliClock::hms_to_morphed_digits(uint8_t hours, uint8_t minutes, float seconds, float digits[6]) {
    const uint8_t hours_tens   = hours   / 10;
    const uint8_t hours_ones   = hours   % 10;
    const uint8_t minutes_tens = minutes / 10;
    const uint8_t minutes_ones = minutes % 10;
    const uint8_t seconds_tens = seconds / 10;
    const uint8_t seconds_ones = uint8_t(seconds) % 10;
    const float f              = seconds - floor(seconds);

    bool carry[6];                                             // 01 23 45
    carry[5] = seconds_ones == 9                            ;  // 00:00:0X
    carry[4] = seconds_tens == 5                 && carry[5];  // 00:00:X0
    carry[3] = minutes_ones == 9                 && carry[4];  // 00:0X:00
    carry[2] = minutes_tens == 5                 && carry[3];  // 00:X0:00
    carry[1] = (hours_ones  == 9 || hours == 24) && carry[2];  // 0X:00:00

    // Hours
    digits[0] = hours_tens   + (carry[1] ? f : 0);
    digits[1] = hours_ones   + (carry[2] ? f : 0);
    // Minutes
    digits[2] = minutes_tens + (carry[3] ? f : 0);
    digits[3] = minutes_ones + (carry[4] ? f : 0);
    // Seconds
    digits[4] = seconds_tens + (carry[5] ? f : 0);
    digits[5] = seconds_ones +             f     ;
}

uint32_t DaliClock::digits_to_ms(const uint8_t digits[6]) {
    uint8_t hours, minutes, seconds;
    hours   = digits[0] * 10 + digits[1];
    minutes = digits[2] * 10 + digits[3];
    seconds = digits[4] * 10 + digits[5];
    return hms_to_ms(hours, minutes, seconds);
}

/*************************** Calendar Display Mode ****************************/

void DaliClock::get_calendar_digit_limits(uint8_t digit_limit[6]) {
    digit_limit[0] = 1;
    digit_limit[1] = 9;
    digit_limit[2] = 3;
    digit_limit[3] = 9;
    digit_limit[4] = 9;
    digit_limit[5] = 9;
}

void DaliClock::set_date(uint8_t month, uint8_t day, uint8_t year) {
    calendar_m = month;
    calendar_d = day;
    calendar_y = year;
}

void DaliClock::get_date(uint8_t &month, uint8_t &day, uint8_t &year) const {
    month = calendar_m;
    day   = calendar_d;
    year  = calendar_y;
}

void DaliClock::mdy_to_digits(uint8_t month, uint8_t day, uint8_t year, uint8_t digits[6]) {
    digits[0] = month / 10;
    digits[1] = month % 10;
    digits[2] = day   / 10;
    digits[3] = day   % 10;
    digits[4] = year  / 10;
    digits[5] = year  % 10;
}

void DaliClock::mdy_to_digits(uint8_t month, uint8_t day, uint8_t year, float digits[6]) {
    digits[0] = month / 10;
    digits[1] = month % 10;
    digits[2] = day   / 10;
    digits[3] = day   % 10;
    digits[4] = year  / 10;
    digits[5] = year  % 10;
}

void DaliClock::digits_to_mdy(const uint8_t digits[6], uint8_t &month, uint8_t &day, uint8_t &year) {
    month   = 10 * digits[0] + digits[1];
    day     = 10 * digits[2] + digits[3];
    year    = 10 * digits[4] + digits[5];
}

void DaliClock::set_calender_mode(bool enable) {
    calendar_requested = enable;
}

void DaliClock::animate_calendar_mode(float seconds) {
    constexpr uint8_t blend_speed         = 64;
    constexpr uint32_t calendar_hold_time = 5000;

    if(calendar_requested) {
        if(calendar_blend == 0) {
            const float f = seconds - floor(seconds);
            if(f < 0.1) {
                calendar_blend += blend_speed;
                calendar_entry_ms = millis();
            }
        } else {
            if(calendar_blend < 255)
                calendar_blend = min(255, int16_t(calendar_blend) + blend_speed);
        }
    } else {
        if(calendar_blend == 255) {
            const float f = seconds - floor(seconds);
            if(f < 0.1 && (millis() - calendar_entry_ms) > calendar_hold_time)
                calendar_blend -= blend_speed;
        } else {
            if(calendar_blend > 0)
               calendar_blend = max(0, int16_t(calendar_blend) - blend_speed);
        }
    }
}
