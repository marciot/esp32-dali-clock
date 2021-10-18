![Dali Clock Animation][animation]

esp32-dali-clock
================

This is an ESP32 recreation of the "Dali" digital clock where the
digits "melt" into one another.

The Dali clock was first written in 1979 by Steve Capps for the
Xerox Alto and later ported to the Macintosh. Later, Jamie
Zawinski re-implemented an X Windows version called [xdaliclock].
I began with the ".xbm" font files from Jamie's work, but my
implementation does not share any code with that version.

This project is a follow up to my earlier project, the
[arduino-dali-clock], but instead of driving a TFT panel, it
generates composite video output for any NTSC or PAL television or
monitor. Putting it toghether is ridiculously simple, requiring
only two wires. All configuration is done via a web interface and
clock's time is synchronized via the Internet.

This project includes modified code from rossumur's [esp_8_bit]
and bitluni's [ESP32CompositeVideo]. These changes are also
available in my standalone project [ESP32CompositeColorVideo].

## License (esp32-dali-clock)

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

## License ([xdaliclock])

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
 
## License ([esp_8_bit])

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

## License ([ESP32CompositeVideo])

```
CC0. Do whatever you like with the code but I will be thankfull 
if you attribute me. Keep the spirit alive :-)

- bitluni
```

![Dali Clock Mini TV][mini-tv]

[xdaliclock]: https://www.jwz.org/xdaliclock
[arduino-dali-clock]: https://github.com/marciot/arduino-dali-clock
[esp_8_bit]: https://github.com/rossumur/esp_8_bit
[ESP32CompositeVideo]: https://github.com/bitluni/ESP32CompositeVideo
[ESP32CompositeColorVideo]: https://github.com/marciot/ESP32CompositeColorVideo
[animation]: https://github.com/marciot/esp32-dali-clock/raw/master/artwork/animation.gif "Dali Clock Animation"
[mini-tv]: https://github.com/marciot/esp32-dali-clock/raw/master/artwork/mini_tv.jpg "Dali Clock on an Analog TV"
