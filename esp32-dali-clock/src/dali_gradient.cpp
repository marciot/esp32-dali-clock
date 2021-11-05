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
#include "dali_constants.h"
#include "gfx/CompositeGraphics.h"
#include "dali_gradient.h"

// Linear blend
uint8_t DaliGradient::mix(uint8_t a, uint8_t b, float ratio) {
    return uint8_t((1 - ratio) * a + ratio * b);
}

// More sophisticated blend that treats hue as a circular map.
uint8_t DaliGradient::mix_chroma(uint8_t a, uint8_t b, float ratio) {
    if(a > b) return mix_chroma(b, a, 1 - ratio);
    if(a == 0) return mix(a, b,ratio); // Blending from white to color
    // Blending from hue to hue:
    a--; b--;// Skip white ($0x) when mixing hues
    if(b - a > 7) return (mix(a+15,b,ratio)%15)+1; // Wrap around the bottom
    return mix(a,b,ratio)+1;
}

uint8_t DaliGradient::gradient_color(int row, int h, uint8_t color1, uint8_t color2) {
    const float   ratio = float(row)/(h-1);
    const uint8_t chroma = mix_chroma(color1 >> 4, color2 >> 4, ratio) << 4;
    const uint8_t luma   = mix(color1 & 0x0F, color2 & 0x0F, ratio) & 0x0F;
    return chroma | luma;
}

char add_luminance(char color, char luminance) {
    const char sum = (color & 0x0F) + luminance;
    return color & 0xF0 | (sum < 0x0F ? sum : 0x0F);
}

// Replaces mask_color with a gradient from color1 to color2
void DaliGradient::draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color) {
    for(int j = y; j < y + h; j++) {
        char new_color = gradient_color(j - y, h, color1, color2);
        for(int i = x; i < x + w; i++)
            if(g.get(i, j) == mask_color)
                g.dotFast(i, j, new_color);
    }
}

// Replaces mask_color with a gradient from color1 to color2
// If shine > 0, draw a diagonal highlight across the gradient
void DaliGradient::draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color, int shine) {
    const bool do_shine = shine > 0;
    for(int j = y; j < y + h; j++) {
        char new_color = gradient_color(j - y, h, color1, color2);
        for(int i = x; i < x + w; i++) {
            if(g.get(i, j) == mask_color) {
                if (do_shine) {
                    const char glow = shine_intensity - min(shine_intensity, abs(shine - i));
                    if(glow) {
                        g.dotFast(i, j,add_luminance(new_color, glow));
                        continue;
                    }
                }
                g.dotFast(i, j, new_color);
            }
        }
        if(do_shine && j&1) shine--;
    }
}
