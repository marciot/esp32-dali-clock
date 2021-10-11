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

// Display size and digit sizes
// The display size is fixed by the Composite video code and the digit sizes
// correspond to the font file.

constexpr int   display_width            = 336;
constexpr int   display_height           = 240;
constexpr int   digit_width              = 45;
constexpr int   digit_height             = 64;
constexpr int   colon_width              = 25;

// Positioning

constexpr int   digit_top                = 30;
constexpr int   sun_radius               = 38;
constexpr int   horizon_y                = 170;
constexpr int   horizon_depth            = 8;
constexpr char  city_height              = 20;

// Other attributes

constexpr int   shine_intensity          = 10; // Between 1 and 16

constexpr int   num_sparkles             = 3;
constexpr float sparkle_duration         = 0.5; // seconds
constexpr float sparkle_phase            = 0.4; // seconds
constexpr int   sparkle_size             = 10;

// Center the clock on the display

constexpr int clock_width = digit_width * 6 + colon_width * 2;
//constexpr int left_margin = (display_width - clock_width)/2;
constexpr int left_margin = 0;
