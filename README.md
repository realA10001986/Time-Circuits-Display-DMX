
**&#9888; Not for public release**

# Firmware for Time Circuits Display - DMX controlled

This repository holds a firmware for CircuitSetup's Time Circuits Display kit which allows to control each element through DMX. It is designed to work the the [Sparkfun LED-to-DMX](https://www.sparkfun.com/products/15110) shield.

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
    <tr><td>10</td><td>Destination Time: Colon (0-85=off; 86-170=on; 171-255=blink</td></tr>
    <tr><td>11</td><td>Destination Time: Brightness</td></tr>
    <tr><td>12</td><td>Present Time: Month</td></tr>
    <tr><td>13</td><td>Present Time: Day</td></tr>
    <tr><td>14</td><td>Present Time: Year 1000s</td></tr>
    <tr><td>15</td><td>Present Time: Year 100s</td></tr>
    <tr><td>16</td><td>Present Time: Year 10s</td></tr>
    <tr><td>17</td><td>Present Time: Year 1s</td></tr>
    <tr><td>18</td><td>Present Time: Hour</td></tr>
    <tr><td>19</td><td>Present Time: Minute</td></tr>
    <tr><td>20</td><td>Present Time: AM/PM</td></tr>
    <tr><td>21</td><td>Present Time: Colon (0-85=off; 86-170=on; 171-255=blink</td></tr>
    <tr><td>22</td><td>Present Time: Brightness</td></tr>
    <tr><td>23</td><td>Last Time Departed: Month</td></tr>
    <tr><td>24</td><td>Last Time Departed: Day</td></tr>
    <tr><td>25</td><td>Last Time Departed: Year 1000s</td></tr>
    <tr><td>26</td><td>Last Time Departed: Year 100s</td></tr>
    <tr><td>27</td><td>Last Time Departed: Year 10s</td></tr>
    <tr><td>28</td><td>Last Time Departed: Year 1s</td></tr>
    <tr><td>29</td><td>Last Time Departed: Hour</td></tr>
    <tr><td>30</td><td>Last Time Departed: Minute</td></tr>
    <tr><td>31</td><td>Last Time Departed: AM/PM</td></tr>
    <tr><td>32</td><td>Last Time Departed: Colon (0-85=off; 86-170=on; 171-255=blink</td></tr>
    <tr><td>33</td><td>Last Time Departed: Brightness</td></tr>
</table>

### Build information

Requires "esp_dmx" library (someweisguy) v4.0.1 or later.

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

The required 3.3V are generated through a voltage converter (like [this](https://www.amazon.com/dp/B09Q8Q3ZVM) one) soldered on to the LCD-to-DMX shield:

![voltage converter](https://github.com/realA10001986/TCD-DMX/assets/76924199/d7e42eff-1782-41a6-b751-f22e25e6e564)


