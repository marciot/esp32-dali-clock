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

// Available color themes

namespace DaliDawn {
    constexpr char  masking_color            = 0xff;
    constexpr char  background_color         = 0x02; // 0x70
    constexpr char  digit_gradient_top       = 0x90;
    constexpr char  digit_gradient_midtop    = 0xac;
    constexpr char  digit_gradient_midbot    = 0x30;
    constexpr char  digit_gradient_bottom    = 0x3f;
    constexpr char  digit_shadow_color       = 0x9b;
    constexpr char  sun_gradient_top         = 0x3f;
    constexpr char  sun_gradient_bottom      = 0x30;
    constexpr char  horizon_gradient_top     = 0x30;
    constexpr char  horizon_gradient_bottom  = 0x85;
    constexpr char  floor_gradient_top       = 0x81;
    constexpr char  floor_gradient_bottom    = 0x0f;
    constexpr char  city_hue                 = 0x30;
}

namespace DaliNight {
    constexpr char  masking_color            = 0xff;
    constexpr char  background_color         = 0x00;
    constexpr char  digit_gradient_top       = 0x90;
    constexpr char  digit_gradient_midtop    = 0xac;
    constexpr char  digit_gradient_midbot    = 0x70;
    constexpr char  digit_gradient_bottom    = 0x8d;
    constexpr char  digit_shadow_color       = 0x9b;
    constexpr char  sun_gradient_top         = 0x0f;
    constexpr char  sun_gradient_bottom      = 0x00;
    constexpr char  horizon_gradient_top     = 0x80;
    constexpr char  horizon_gradient_bottom  = 0x97;
    constexpr char  floor_gradient_top       = 0x85;
    constexpr char  floor_gradient_bottom    = 0xca;
    constexpr char  city_hue                 = 0x80;
}

namespace DaliDay {
    constexpr char  masking_color            = 0xff;
    constexpr char  background_color         = 0x0f;
    constexpr char  digit_gradient_top       = 0x95;
    constexpr char  digit_gradient_midtop    = 0xac;
    constexpr char  digit_gradient_midbot    = 0x11;
    constexpr char  digit_gradient_bottom    = 0x1f;
    constexpr char  digit_shadow_color       = 0x00;
    constexpr char  sun_gradient_top         = 0x1f;
    constexpr char  sun_gradient_bottom      = 0x11;
    constexpr char  horizon_gradient_top     = 0xc0;
    constexpr char  horizon_gradient_bottom  = 0x17;
    constexpr char  floor_gradient_top       = 0x1f;
    constexpr char  floor_gradient_bottom    = 0x11;
    constexpr char  city_hue                 = 0xe0;
}

namespace DaliDusk {
    constexpr char  masking_color            = 0xff;
    constexpr char  background_color         = 0x02;
    constexpr char  digit_gradient_top       = 0x90;
    constexpr char  digit_gradient_midtop    = 0xac;
    constexpr char  digit_gradient_midbot    = 0xa0;
    constexpr char  digit_gradient_bottom    = 0xad;
    constexpr char  digit_shadow_color       = 0xab;
    constexpr char  sun_gradient_top         = 0x5f;
    constexpr char  sun_gradient_bottom      = 0x50;
    constexpr char  horizon_gradient_top     = 0xb0;
    constexpr char  horizon_gradient_bottom  = 0xc7;
    constexpr char  floor_gradient_top       = 0xc1;
    constexpr char  floor_gradient_bottom    = 0xaa;
    constexpr char  city_hue                 = 0xa0;
}

// Active color theme

extern char background_color;
extern char masking_color;
extern char digit_gradient_top;
extern char digit_gradient_midtop;
extern char digit_gradient_midbot;
extern char digit_gradient_bottom;
extern char digit_shadow_color;
extern char sun_gradient_top;
extern char sun_gradient_bottom;
extern char horizon_gradient_top;
extern char horizon_gradient_bottom;
extern char floor_gradient_top;
extern char floor_gradient_bottom;
extern char city_hue;

class DaliColorTheme {
    private:
        uint8_t theme;
        static char swapColors(char);
    public:
        enum {
            NightColors = 0,
            DawnColors  = 1,
            DayColors   = 2,
            DuskColors  = 3,
            MinuteCycle = 98,
            DayCycle    = 99
        };

        void applyBlendedTheme(float);
        void setTheme(uint8_t id) {theme = id;}
        void update(float dayElapsed);
};