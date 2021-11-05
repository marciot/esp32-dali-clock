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

#pragma once

struct dali_digit_t {
    uint16_t height;
    uint16_t width;
    uint8_t linestride;
    const uint8_t *packed_rle;
};

#include "dali_font_E.h"

class DaliDigit {
    private:
        static const dali_digit_t *getDigitInfo(char c);
        static void rle_to_segment_endpoints(uint8_t rle[]);
        static void segment_endpoints_to_rle(uint8_t rle[]);
        static void blend_rle(uint8_t start[4], uint8_t final[4], uint8_t blended[4], uint8_t blend);
        static void unpack_rle_to_graphics(CompositeGraphics &g, const uint8_t *src, int x, int y, uint8_t color);
    public:
        static void draw_row(CompositeGraphics &g, char c1, char c2, uint8_t blend, uint16_t row, int &x, int y, uint8_t color);
        static void draw_row(CompositeGraphics &g, char *str1, char *str2, uint8_t blend, uint16_t row, int x, int y, uint8_t color);
        static void draw(CompositeGraphics &g, char *str1, char *str2, uint8_t blend, int x, int y, uint8_t color);
};
