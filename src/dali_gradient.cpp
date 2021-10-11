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
#include "../dali_config.h"
#include "gfx/CompositeGraphics.h"
#include "dali_gradient.h"

uint8_t DaliGradient::gradient_color(int row, int h, uint8_t color1, uint8_t color2) {
    #define MIX(A,B) uint8_t((1 - ratio) * (A) + ratio * (B))
    const float   ratio = float(row)/(h-1);
    const uint8_t chroma = MIX(color1 >> 4, color2 >> 4) << 4;
    const uint8_t luma   = MIX(color1 & 0x0F, color2 & 0x0F) & 0x0F;
    return chroma | luma;
}

char add_luminance(char color, char luminance) {
    const char sum = (color & 0x0F) + luminance;
    return color & 0xF0 | (sum < 0x0F ? sum : 0x0F);
}

// Replaces mask_color with a gradient from color1 to color2
void DaliGradient::draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color) {
    for(int j = y; j < y + h; j++)
        for(int i = x; i < x + w; i++)
            if(g.get(i, j) == mask_color)
                g.dotFast(i, j, gradient_color(j - y, h, color1, color2));
}

// Replaces mask_color with a gradient from color1 to color2
// If shine > 0, draw a diagonal highlight across the gradient
void DaliGradient::draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color, int shine) {
    const bool do_shine = shine > 0;
    for(int j = y; j < y + h; j++) {
        for(int i = x; i < x + w; i++) {
            if(g.get(i, j) == mask_color) {
                char color = gradient_color(j - y, h, color1, color2);
                if (do_shine) {
                    const char glow = shine_intensity - min(shine_intensity, abs(shine - i));
                    if(glow)
                        color = add_luminance(color, glow);
                }
                g.dotFast(i, j, color);
            }
        }
        if(do_shine && j&1) shine--;
    }
}
