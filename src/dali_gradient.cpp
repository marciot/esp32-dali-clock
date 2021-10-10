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
#include "dali_gradient.h"

uint8_t DaliGradient::gradient_color(int row, int h, char color1, char color2) {
    const float mix = float(row)/h;
    #define MIX(A,B) uint8_t((1 - mix) * (A) + mix * (B))
    return (MIX(color1 & 0xF0, color2 & 0xF0) & 0xF0) +
            MIX(color1 & 0x0F, color2 & 0x0F);
}

void DaliGradient::draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color) {
    for(int j = y; j < y + h; j++)
        for(int i = x; i < x + w; i++)
            if(g.get(i, j) == mask_color)
                g.dotFast(i, j, gradient_color(j - y, h, color1, color2));
}
