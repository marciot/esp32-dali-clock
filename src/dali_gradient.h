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

class DaliGradient {
    private:
        static uint8_t mix(uint8_t a, uint8_t b, float ratio);
        static uint8_t mix_chroma(uint8_t a, uint8_t b, float ratio);
    public:
        static uint8_t gradient_color(int row, int h, uint8_t color1, uint8_t color2);
        static void draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color);
        static void draw(CompositeGraphics &g, int x, int y, int w, int h, char color1, char color2, char mask_color, int shine);
};
