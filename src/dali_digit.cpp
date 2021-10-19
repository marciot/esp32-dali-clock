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

#define USE_ATARI_COLORS

#include <Arduino.h>
#include "gfx/CompositeGraphics.h"
#include "dali_digit.h"

const dali_digit_t *DaliDigit::getDigitInfo(char c) {
    const dali_digit_t *digits[] = {
        &DIGIT_INFO(slash),
        &DIGIT_INFO(zero),
        &DIGIT_INFO(one),
        &DIGIT_INFO(two),
        &DIGIT_INFO(three),
        &DIGIT_INFO(four),
        &DIGIT_INFO(five),
        &DIGIT_INFO(six),
        &DIGIT_INFO(seven),
        &DIGIT_INFO(eight),
        &DIGIT_INFO(nine),
        &DIGIT_INFO(colon)
    };
    return digits[uint8_t(c - '/') % 12];
}

void DaliDigit::rle_to_segment_endpoints(uint8_t rle[]) {
    if(rle[3] != 0) {
        rle[1] = rle[0] + rle[1];
        rle[2] = rle[1] + rle[2];
        rle[3] = rle[2] + rle[3];
    } else {
        rle[1] = rle[0] + rle[1];
        rle[2] = rle[0];
        rle[3] = rle[1];
    }
}

void DaliDigit::segment_endpoints_to_rle(uint8_t rle[]) {
    if(rle[2] > rle[1]) {
        rle[3] -= rle[2];
        rle[2] -= rle[1];
        rle[1] -= rle[0];
    } else {
        rle[1]  = rle[3] - rle[0];
        rle[2]  = 0;
        rle[3]  = 0;
    }
}

void DaliDigit::blend_rle(uint8_t start[4], uint8_t final[4], uint8_t blended[4], uint8_t blend) {
    rle_to_segment_endpoints(start);
    rle_to_segment_endpoints(final);
    for(uint8_t i = 0; i < 4; i++) {
        blended[i] = ((256-blend) * start[i] + blend * final[i]) / 256;
    }
    segment_endpoints_to_rle(blended);
}

void DaliDigit::unpack_rle_to_graphics(CompositeGraphics &g, const uint8_t *src, int x, int y, uint8_t color) {
    uint8_t len = 4;
    uint8_t n;
    do {
        // Skip background pixels
        x += *src++;
        // Draw foreground pixels
        n = *src++;
        while(n--) g.dot(x++, y, color);
        len -= 2;
    } while(len != 0);
}

void DaliDigit::draw_row(CompositeGraphics &g, char c1, char c2, uint8_t blend, uint16_t row, int &x, int y, uint8_t color) {
    const dali_digit_t *digit_1 = getDigitInfo(c1);
    const dali_digit_t *digit_2 = getDigitInfo(c2);
    uint8_t start[4]   = {0,0,0,0};
    uint8_t final[4]   = {0,0,0,0};
    uint8_t blended[4] = {0,0,0,0};
    if(row < digit_1->height) memcpy(start, digit_1->packed_rle + row * 4, 4);
    if(row < digit_2->height) memcpy(final, digit_2->packed_rle + row * 4, 4);
    blend_rle(start, final, blended, blend);
    unpack_rle_to_graphics(g, blended, x, y, color);
    x += max(digit_1->width, digit_2->width);
}

void DaliDigit::draw_row(CompositeGraphics &g, char *str1, char *str2, uint8_t blend, uint16_t row, int x, int y, uint8_t color) {
    while(*str1 && *str2)
        draw_row(g, *str1++, *str2++, blend, row, x, y, color);
}

void DaliDigit::draw(CompositeGraphics &g, char *str1, char *str2, uint8_t blend, int x, int y, uint8_t color) {
    for(uint16_t _row = 0; _row <= getDigitInfo('0')->height; _row++)
        draw_row(g, str1, str2, blend, _row, x, y++, color);
}
