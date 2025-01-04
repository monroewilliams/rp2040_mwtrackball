# rp2040-based trackball main board

This work is based on version 2 of the "rp2040 Minimal KiCad" sample provided by the Raspberry Pi foundation [here](https://datasheets.raspberrypi.com/rp2040/Minimal-KiCAD.zip).

It is inspired by (although not directly based on) jfedor2's excellent [RP2040+PMW3360 board](https://github.com/jfedor2/rp2040-pmw3360). (Although I did borrow the USB header pinout directly from that project.)

I've modified it for the specific needs of [my trackball design](https://github.com/monroewilliams/trackball), with a few other additions that may make it useful for other experimentation as well. I intend it to pair with a [breakout board design](https://github.com/monroewilliams/pmw3360-breakout) with matching JST plugs so that off-the-shelf cables can be used for assembly (instead of the hand-built sensor cables the old design needed).

The overall dimensions of the board match the dimensions of the mini-breadboard I used in the original trackball design. This is not a coincidence. ;) 

The SPI connectors are 6 pin JST-SH (1.0mm pitch), with a pinout based on [Pmod type 2](https://en.wikipedia.org/wiki/Pmod_Interface). This should match the connectors on the sensor breakout board linked above, allowing assembly using off-the-shelf 6-pin JST-SH cables. The cables need to be straight-through, not "crossed" (i.e. connect pin 1 to pin 1, etc.). I've included two SPI connectors attached to one set of SPI pins but with distinct cable select lines (labelled SPI0.x), and a third connector attached to a second set of SPI pins (SPI1.x). The number after the decimal point is the GPIO number of the cable select pin routed to each connector. The trackball design works fine with both sensors on the same SPI bus, but I expect the second bus may be useful for other experiments (such as connecting a display), and I wanted to have some flexibility to play with that.

The button connectors are also JST-SH, but just two pins each (signal and ground), with the intention that off-the-shelf "pigtail" JST cables can be soldered to microswitches for assembly. They are labelled with the number of the GPIO pin they're connected to.

There's also an on-board surface mount piezo speaker, which my trackball design uses to generate scrolling clicks when using twist-to-scroll. It should be usable for other speaker-related duties as needed.

Additionally, I added a connector that I intend to be compatible with Adafruit's ["STEMMA QT" I2C connection](https://learn.adafruit.com/introducing-adafruit-stemma-qt/what-is-stemma), and an additional 6 pin JST-SH breakout connector for a few of the other unused pins on the rp2040 (GPIOs 26-29, which happen to be the ADC-capable pins, as well as power and ground).

Any issues with the changes I made to this layout are solely my fault. I've attempted to maintain the spirit of the rp2040 reference design, but this is my first attempt at designing a board with a microcontroller, so it would not surprise me if it has problems. Caveat emptor, etc, etc.
