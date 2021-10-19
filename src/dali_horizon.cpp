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
#include "dali_horizon.h"

void DaliHorizon::draw(CompositeGraphics &g) {
    DaliGradient::draw(g, 0, horizon_y - horizon_depth, display_width, horizon_depth, horizon_gradient_top, horizon_gradient_bottom, background_color);
}
