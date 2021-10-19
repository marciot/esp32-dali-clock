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
#include "dali_sun.h"

void DaliSun::draw(CompositeGraphics &g) {
    const int r2 = sun_radius*sun_radius;
    const int cy = horizon_y - horizon_depth - 10;
    const int cx = display_width/2;
    const int sun_top    = cy - sun_radius;
    const int sun_bottom = horizon_y - horizon_depth;
    for(int y = sun_top; y < sun_bottom; y++) {
        const int dy = y-cy;
        const int dx = sqrt(r2 - dy*dy);
        if((dy>>1)&3)
            g.line(cx - dx, y, cx + dx, y, masking_color);
    }
    // Draw the gradient
    DaliGradient::draw(g, cx - sun_radius, sun_top, sun_radius * 2, sun_bottom - sun_top, sun_gradient_top, sun_gradient_bottom, masking_color);
}
