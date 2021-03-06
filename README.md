![Dali Clock Animation][animation]

esp32-dali-clock
================

This is an ESP32 recreation of the "Dali" digital clock where the
digits "melt" into one another. Watch a full demonstration on
[YouTube]

## Stretch Goal for Sponsors!

:orange_heart: :yellow_heart: :purple_heart: If I get at least 25
GitHub Sponsors for this project, I will work on a vaporwave-style
of this clock! Watch this helpful video
[Vaporwave vs Outrun, What's the Difference?] for a breakdown of
the differences between outrun and vaporwave aesthetic! :heart_eyes:

## About the project

The Dali clock was first written in 1979 by Steve Capps for the
Xerox Alto and later ported to the Macintosh. Later, Jamie Zawinski
re-implemented an X Windows version called [xdaliclock]. Jamie's page
has a full write up of the history, including a link to a super
trippy film from the 1974s that inspired the clock.

My re-interpretation of the Dali clock includes colorful retro
graphics and outputs composite video output for any NTSC or PAL
television or monitor. Putting it together is ridiculously simple,
requiring only two wires. All configuration is done via a web
interface [YouTube] and clock's time is synchronized via the Internet.

<details>
<summary>More details</summary></br>

This project is a follow up to my earlier project, the
[arduino-dali-clock], which used a TFT panel and an
external clock module.

For my clock, I borrowed the ".xbm" font files from Jamie's
work, but my implementation does not share any code with that
version.

For video output, this project includes code from rossumur's
[esp_8_bit] and bitluni's [ESP32CompositeVideo]. These changes
are also available in my standalone project [ESP32CompositeColorVideo],
which also includes a write up on what bits I borrowed from where.

</details>

## Troubleshooting

<details>
<summary>Click to Expand</summary></br>

**I get an error about the files in "src" not being found:** Make sure you are using
the latest version of the Arduino IDE. Earlier versions do not support files in the
"src" directory.

</details>

## Wiring for an [Adafruit HUZZAH32]:

<details>
<summary>Click to Expand</summary></br>

![Dali Clock Wiring][wiring]

1. Use an alligator clip to connect the pin labeled "GND" on the [Adafruit HUZZAH32] to the outside barrel of the RCA plug
2. Use an alligator clip to connect the pin labeled "A1/DAC1" on the [Adafruit HUZZAH32] to the central pin of the RCA plug
3. Connect the other end of the RCA cable to the yellow jack on your TV or monitor

</details>

## Advanced Configuration Options

<details>
<summary>Click to Expand</summary></br>

**For PAL output**, change the line in "esp32-dali-clock.ino" from:

```
CompositeColorOutput composite(CompositeColorOutput::NTSC);
```

To:

```
CompositeColorOutput composite(CompositeColorOutput::PAL);
```

**To change the orientation of the screen**, uncomment (remove the leading two
slashes) from one of the following lines in the file "src/gfx/CompositeGraphics.h"

```
//#define GFX_UPSIDE_DOWN
//#define GFX_FLIP_HORIZONTAL
//#define GFX_FLIP_VERTICAL
```

</details>

## Licenses

<details>
<summary>Click to expand</summary>

### ESP32 Dali Clock (marciot)

```
DaliClock by (c) 2021 Marcio Teixeira

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

To view a copy of the GNU General Public License, go to the following
location: <http://www.gnu.org/licenses/>.
```

## [xdaliclock] (Jamie Zawinski)

```
xdaliclock - a melting digital clock
Copyright (c) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1999, 2001, 2006
 Jamie Zawinski <jwz@jwz.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.  No representations are made about the suitability of this
software for any purpose.  It is provided "as is" without express or
implied warranty.
```
 
### [esp_8_bit] (rossumur, Peter Barrett)

```
Copyright (c) 2020, Peter Barrett

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
```

### [ESP32CompositeVideo] (Bitluni)

```
CC0. Do whatever you like with the code but I will be thankfull 
if you attribute me. Keep the spirit alive :-)

- bitluni
```

</details>

![Dali Clock Mini TV][mini-tv]

[xdaliclock]: https://www.jwz.org/xdaliclock
[arduino-dali-clock]: https://github.com/marciot/arduino-dali-clock
[esp_8_bit]: https://github.com/rossumur/esp_8_bit
[ESP32CompositeVideo]: https://github.com/bitluni/ESP32CompositeVideo
[ESP32CompositeColorVideo]: https://github.com/marciot/ESP32CompositeColorVideo
[animation]: https://github.com/marciot/esp32-dali-clock/raw/master/artwork/animation.gif "Dali Clock Animation"
[mini-tv]: https://github.com/marciot/esp32-dali-clock/raw/master/artwork/mini_tv.jpg "Dali Clock on an Analog TV"
[wiring]: https://github.com/marciot/esp32-dali-clock/raw/master/artwork/wiring.jpg "Dali Clock Wiring"
[Adafruit HUZZAH32]: https://www.adafruit.com/product/3405
[YouTube]: https://www.youtube.com/watch?v=xGliOsGXlng&t=14s
[Vaporwave vs Outrun, What's the Difference?]: https://youtu.be/qGodWY9vZN8