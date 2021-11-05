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
#include "dali_constants.h"
#include "dali_color_theme.h"
#include "dali_stars.h"

void DaliStars::draw(CompositeGraphics &g, float t) {
    DaliStars::draw(g,t,0,0,display_width,horizon_y-horizon_depth);
}
    
void DaliStars::draw(CompositeGraphics &g, float t, int x, int y, int w, int h) {
    if(background_color != 0x00) return;
    srand(0);
    for(uint8_t i = 0; i < 100; i++) {
        const int _x = x + rand() % w;
        const int _y = y + rand() % h;
        const char hue  = rand() % 16;
        const uint32_t luma_freq  = rand() % 10000 + 1000;
        const uint32_t luma_phase = rand() % luma_freq;
        const char luma = 12 + 4 * sin(2*PI*(millis()+luma_phase)/luma_freq);
        const char luma_fade = max(0, luma - _y*10/h); // Fade towards horizon
        g.dot(_x,_y,(hue << 4) + luma_fade);
    }
}
