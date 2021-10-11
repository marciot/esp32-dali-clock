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

bool draw_stars               = DaliDusk::draw_stars;
char background_color         = DaliDusk::background_color;
char masking_color            = DaliDusk::masking_color;
char digit_gradient_top       = DaliDusk::digit_gradient_top;
char digit_gradient_midtop    = DaliDusk::digit_gradient_midtop;
char digit_gradient_midbot    = DaliDusk::digit_gradient_midbot;
char digit_gradient_bottom    = DaliDusk::digit_gradient_bottom;
char digit_shadow_color       = DaliDusk::digit_shadow_color;
char sun_gradient_top         = DaliDusk::sun_gradient_top;
char sun_gradient_bottom      = DaliDusk::sun_gradient_bottom;
char horizon_gradient_top     = DaliDusk::horizon_gradient_top;
char horizon_gradient_bottom  = DaliDusk::horizon_gradient_bottom;
char floor_gradient_top       = DaliDusk::floor_gradient_top;
char floor_gradient_bottom    = DaliDusk::floor_gradient_bottom;
char city_hue                 = DaliDusk::city_hue;

char DaliColorTheme::swapColors(char color) {
    // Swap $1x and $Fx so sun does not appear green
    if((color & 0xF0) == 0xF0) return color ^ 0xE0;
    if((color & 0xF0) == 0x10) return color ^ 0xE0;
    return color;
}
void DaliColorTheme::setTheme(float theme) {
    const int theme_base = floor(theme);
    const int theme_ratio = (theme - theme_base) * 16;
    #define BLEND_COLOR(A,B,C) C = swapColors(DaliGradient::gradient_color(theme_ratio, 16, A::C, B::C));
    #define BLEND_THEME(A,B) BLEND_COLOR(A,B,background_color       ); \
                             BLEND_COLOR(A,B,masking_color          ); \
                             BLEND_COLOR(A,B,digit_gradient_top     ); \
                             BLEND_COLOR(A,B,digit_gradient_midtop  ); \
                             BLEND_COLOR(A,B,digit_gradient_midbot  ); \
                             BLEND_COLOR(A,B,digit_gradient_bottom  ); \
                             BLEND_COLOR(A,B,digit_shadow_color     ); \
                             BLEND_COLOR(A,B,sun_gradient_top       ); \
                             BLEND_COLOR(A,B,sun_gradient_bottom    ); \
                             BLEND_COLOR(A,B,horizon_gradient_top   ); \
                             BLEND_COLOR(A,B,horizon_gradient_bottom); \
                             BLEND_COLOR(A,B,floor_gradient_top     ); \
                             BLEND_COLOR(A,B,floor_gradient_bottom  ); \
                             BLEND_COLOR(A,B,city_hue               );
    switch(theme_base % 4) {
        case 0: BLEND_THEME(DaliNight, DaliDawn); break;
        case 1: BLEND_THEME(DaliDawn,  DaliDay); break;
        case 2: BLEND_THEME(DaliDay,   DaliDusk); break;
        case 3: BLEND_THEME(DaliDusk,  DaliNight); break;
    }
}
