
# Firmware for Time Circuits Display - DMX controlled

This repository holds a firmware for CircuitSetup's Time Circuits Display kit which allows to control each element through DMX. It is designed to work using the [Sparkfun LED-to-DMX](https://www.sparkfun.com/products/15110) shield.

(DMX control is also available for [Flux Capacitor](https://github.com/realA10001986/Flux-Capacitor-DMX/blob/main/README.md) and [SID[(https://github.com/realA10001986/SID-DMX/tree/main))

### DMX channels

<table>
    <tr><td>DMX channel</td><td>Function</td></tr>
    <tr><td>1</td><td>Destination Time: Month</td></tr>
    <tr><td>2</td><td>Destination Time: Day</td></tr>
    <tr><td>3</td><td>Destination Time: Year 1000s</td></tr>
    <tr><td>4</td><td>Destination Time: Year 100s</td></tr>
    <tr><td>5</td><td>Destination Time: Year 10s</td></tr>
    <tr><td>6</td><td>Destination Time: Year 1s</td></tr>
    <tr><td>7</td><td>Destination Time: Hour</td></tr>
    <tr><td>8</td><td>Destination Time: Minute</td></tr>
    <tr><td>9</td><td>Destination Time: AM/PM</td></tr>
    <tr><td>10</td><td>Destination Time: Colon (0-85=off; 86-170=on; 171-255=blink)</td></tr>
    <tr><td>11</td><td>Destination Time: Brightness (0=off; 1-255=darkest-brightest)</td></tr>
    <tr><td>12</td><td>Present Time: Month</td></tr>
    <tr><td>13</td><td>Present Time: Day</td></tr>
    <tr><td>14</td><td>Present Time: Year 1000s</td></tr>
    <tr><td>15</td><td>Present Time: Year 100s</td></tr>
    <tr><td>16</td><td>Present Time: Year 10s</td></tr>
    <tr><td>17</td><td>Present Time: Year 1s</td></tr>
    <tr><td>18</td><td>Present Time: Hour</td></tr>
    <tr><td>19</td><td>Present Time: Minute</td></tr>
    <tr><td>20</td><td>Present Time: AM/PM</td></tr>
    <tr><td>21</td><td>Present Time: Colon (0-85=off; 86-170=on; 171-255=blink)</td></tr>
    <tr><td>22</td><td>Present Time: Brightness (0=off; 1-255=darkest-brightest)</td></tr>
    <tr><td>23</td><td>Last Time Departed: Month</td></tr>
    <tr><td>24</td><td>Last Time Departed: Day</td></tr>
    <tr><td>25</td><td>Last Time Departed: Year 1000s</td></tr>
    <tr><td>26</td><td>Last Time Departed: Year 100s</td></tr>
    <tr><td>27</td><td>Last Time Departed: Year 10s</td></tr>
    <tr><td>28</td><td>Last Time Departed: Year 1s</td></tr>
    <tr><td>29</td><td>Last Time Departed: Hour</td></tr>
    <tr><td>30</td><td>Last Time Departed: Minute</td></tr>
    <tr><td>31</td><td>Last Time Departed: AM/PM</td></tr>
    <tr><td>32</td><td>Last Time Departed: Colon (0-85=off; 86-170=on; 171-255=blink)</td></tr>
    <tr><td>33</td><td>Last Time Departed: Brightness (0=off; 1-255=darkest-brightest)</td></tr>
</table>

If speedo support is enabled (by defining TC_HAVESPEEDO in tcd_global.h), additional channels are supported:

<table>
    <tr><td>DMX channel</td><td>Function</td></tr>
    <tr><td>57</td><td>Speedo: Speed (0-255 = 0-88mph)</td></tr>
    <tr><td>58</td><td>Speedo: Brightness (0-255)</td></tr>
</table>

#### Packet verification

The DMX protocol uses no checksums. Therefore, transmission errors cannot be detected. Typically, such errors manifest themselves in flicker or a corrupted display for short moments. Since the TCD is no ordinary light fixture, this can be an issue.

In order to at least filter out grossly malformed/corrupt DMX data packets, the firmware supports a simple DMX packet verifier: For a DMX data packet to be considered valid, _channel 46 must be at value 100_. If a packet contains any other value for this channel, the packet is ignored. 

To enable this filter, DMX_USE_VERIFY must be #defined in tcd_global.h. This feature is disabled by default, because it hinders a global "black out". If your DMX controller can exclude channels from "black out" (or this function is not to be used), and you experience flicker, you can try to activate this packet verifier.

### Firmware update

To update the firmware without Arduino IDE/PlatformIO, copy a pre-compiled binary (filename must be "tcdfw.bin") to a FAT32 formatted SD card, insert this card into the TCD, and power up. The TCD will display "UPDATING" and update the firmware. Afterwards it will reboot.

### Build information

Requires [esp_dmx](https://github.com/someweisguy/esp_dmx) library v4.0.1 or later.

### Hardware: Pin mapping

<table>
    <tr>
     <td align="center">TCD</td><td align="center">LED-to-DMX shield</td>
    </tr>
    <tr>
     <td align="center">PWR Trigger (IO13)</a></td>
     <td align="center">J1 P14</td>
    </tr>
    <tr>
     <td align="center">TT OUT (IO14)</td>
     <td align="center">J1 P15</td>
    </tr>
    <tr>
     <td align="center">TT IN (IO27)</td>
     <td align="center">J1 P16</td>
    </tr>
    <tr>
     <td align="center">5V/GND from "Fake Power" or "Time Travel"</td>
     <td align="center">J12</td>
    </tr>
</table>

![DMXshield-TCD](https://github.com/realA10001986/Time-Circuits-Display-DMX/assets/76924199/66f4c3ab-215e-4fb2-8ca0-15d16c9f164f)

The required 3.3V are generated through a voltage converter (like [this](https://www.amazon.com/Anmbest-AMS1117-3-3-4-75V-12V-Voltage-Regulator/dp/B07CP4P5XJ/ref=sr_1_5) one) attached to the LCD-to-DMX shield:

![voltage converter](https://github.com/realA10001986/TCD-DMX/assets/76924199/d7e42eff-1782-41a6-b751-f22e25e6e564)

GND is taken from J1 P4, 5V from J2 P3. The 3.3V output is on J1 P2.


