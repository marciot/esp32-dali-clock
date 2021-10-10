/****************************************************************************
 *   DaliClock by (c) 2021 Marcio Teixeira                               *
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
#include "dali_gradient.h"
#include "dali_grid.h"

constexpr int center_x = display_width/2;
constexpr int center_y = display_height/2;

void DaliGrid::draw(CompositeGraphics &g, float motion) {
    // Draw the horizon lines
    for(uint16_t p = 2; ; p++) {
        const int y = center_y + pow(2, motion + p);
        if(y > display_height) break;
        g.line(0, y, display_width, y, 0x0F);
    }

    // Draw the vanishing lines
    for(int16_t x = -int16_t(display_width); x < int16_t(display_width); x += display_width/4) {
        const int x1 = center_x + x/2;
        const int y1 = center_y;
        const int x2 = center_x + x;
        const int y2 = display_height;
        g.line(x1  , y1, x2  , y2, 0x0F);
        g.line(x1+1, y1, x2+1, y2, 0x0F);
    }

    // Draw the gradient
    DaliGradient::draw(g, 0, center_y, display_width, display_height - center_y, floor_gradient_top, floor_gradient_bottom);
}
