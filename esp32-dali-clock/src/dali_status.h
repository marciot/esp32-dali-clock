/****************************************************************************
 *   DaliClock by (c) 2018 Marcio Teixeira                               *
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

class DaliStatus {
    private:
        String message;
        char color;
        uint32_t lastUpdate;

        bool darkBackground() {return (background_color & 0x0F) < 8;}

        void fade() {
            if(millis() - lastUpdate > 5000) {
                lastUpdate = millis();
                if(darkBackground()) {
                    if (color != 0x00) color--;
                } else {
                    if (color != 0x0F) color++;
                }
            }
        }

        void unfade() {
            color = darkBackground() ? 0x0F : 0x00;
        }
    public:
        void set(String str) {
            message = str;
            unfade();
            lastUpdate = millis();
        }

        void append(String str) {
            message += str;
            unfade();
            lastUpdate = millis();
        }

        void draw(CompositeGraphics &g) {
            if(color != background_color) {
                //setting text color, transparent background
                g.setTextColor(color);
                //text starting position
                g.setCursor(30, 15);
                //printing some lines of text
                g.print(message.c_str());
                fade();
            }
        }
};
