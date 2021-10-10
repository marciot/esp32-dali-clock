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

//#define BLACK_AND_WHITE

// Display size and digit sizes
// The display size is fixed by the Composite video code and the digit sizes
// correspond to the font file.

constexpr int display_width  = 336;
constexpr int display_height = 240;
constexpr int digit_width    = 45;
constexpr int digit_height   = 64;
constexpr int colon_width    = 25;

// Colors

#ifdef BLACK_AND_WHITE
  constexpr char digit_gradient_top    = 0x09;
  constexpr char digit_gradient_bottom = 0x02;
  constexpr char digit_shadow_color    = 0x03;
  constexpr char floor_gradient_top    = 0x00;
  constexpr char floor_gradient_bottom = 0x06;
#else
  constexpr char digit_gradient_top    = 0x4b;
  constexpr char digit_gradient_bottom = 0x92;
  constexpr char digit_shadow_color    = 0x9b;
  constexpr char floor_gradient_top    = 0x00;
  constexpr char floor_gradient_bottom = 0x0f;
#endif

constexpr int shine_intensity = 10; // Between 1 and 16

constexpr int num_sparkles = 3;
constexpr float sparkle_duration = 0.5; // seconds
constexpr float sparkle_phase = 0.4; // seconds
constexpr int sparkle_size = 10;

// Center the clock on the display

constexpr int clock_width = digit_width * 6 + colon_width * 2;
constexpr int left_margin = (display_width - clock_width)/2;
constexpr int top_margin = (display_height - digit_height)/2;
