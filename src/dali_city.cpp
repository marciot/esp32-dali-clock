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
#include "gfx/CompositeGraphics.h"
#include "dali_constants.h"
#include "dali_color_theme.h"
#include "dali_gradient.h"
#include "dali_city.h"

void DaliCity::draw(CompositeGraphics &g) {
    srand(0);
    DaliCity::draw(g, city_height,    0x0F);
    DaliCity::draw(g, city_height>>1, 0x0F);
}

void DaliCity::draw(CompositeGraphics &g, int city_height, char lumi) {
    for(int x = 0; x < display_width;) {
        const int w = 6 + rand() % 6;
        const int gap = rand() % 6;
        const int h = rand() % city_height;
        g.fillRect(x,horizon_y-horizon_depth-h,w,h,masking_color);
        x += w + gap;
    }
    // Draw the gradient
    DaliGradient::draw(g, 0, horizon_y-horizon_depth-city_height, display_width, city_height, city_hue | lumi, city_hue, masking_color);
}
