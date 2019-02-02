# Nixie-Protoboard-Large Board
Protoboard for testing Nixie tube driving software development

I have a bunch of the new-old-stock K155ID1 chips that I found cheap on Amazon and eBay. I've been told NTE still makes the 74141 and found these links,

http://www.nteinc.com/specs/7400to7499/pdf/nte74141.pdf
http://dilp.netcomponents.com/cgi-bin/nteinc.asp?partnumber1=NTE74141


# Nixie Tube Tester Code
Learning to code for the Arduino for a Nixie tube project.
ATmega328 Driving SN74141 Nixie driver chip via pins A, B, C, & D

## Attributions and Thanks
- ATMega328 outside of Arduino Uno - https://dronebotworkshop.com/arduino-uno-atmega328/
- Multi-tasking the Arduino - https://learn.adafruit.com/multi-tasking-the-arduino-part-1?view=all


## Connections
```
ATMega328 Pin 28 (Analog 5 - I2C Clock) to MCP Pin 12
ATMega328 Pin 27 (Analog 4 - I2C Data)  to MCP Pin 13

MCP Pins #15, 16 and 17 are address selection (see below)
MCP Pin  9 to 5V
MCP Pin 10 to GND
MCP Pin 18 to 10K Resistor to 5V (reset pin, active low)
```

## MCP Addresses
```
addr 0 = A2 low,  A1 low,  A0 low  000
addr 1 = A2 low,  A1 low,  A0 high 001
addr 2 = A2 low,  A1 high, A0 low  010
addr 3 = A2 low,  A1 high, A0 high 011
addr 4 = A2 high, A1 low,  A0 low  100
addr 5 = A2 high, A1 low,  A0 high 101
addr 6 = A2 high, A1 high, A0 low  110
addr 7 = A2 high, A1 high, A0 high 111
```
