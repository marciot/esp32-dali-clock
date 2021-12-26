/****************************************************************************
 *   DaliClock by (c) 2018 Marcio Teixeira                                  *
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

#include "dali_config.h"

class DaliClock {
    private:
        DaliConfig &config;
        bool calendar_mode;
        uint32_t last_change_ms;
        time_t   offset_time;

        float day_elapsed;

        char old_display[9];
        char new_display[9];

        void hms_to_str(uint8_t hours, uint8_t minutes, uint8_t seconds, char str[6]);
        void mdy_to_str(uint8_t month, uint8_t day, uint8_t year, char str[8]);
        void time_to_digits();
    public:
        DaliClock(DaliConfig &);

        static void draw_gradient_and_shine(CompositeGraphics &g, float f);

        void draw(CompositeGraphics &g);
        void set_time (uint8_t hours, uint8_t minutes, uint8_t seconds);
        void set_date(uint8_t month, uint8_t day, uint16_t year);

        void set_calender_mode(bool enable);

        float get_day_elapsed() {return day_elapsed;};
};

#define CLOCK_RECT      left_margin, digit_top, clock_width, digit_height
#define CLOCK_TOP_RECT  left_margin, digit_top, clock_width, digit_height*0.45
#define CLOCK_BOT_RECT  left_margin, digit_top + digit_height*0.45, clock_width, digit_height*0.55