# Arduino Library for LTC2941 and LTC2942

Supports Analog Devices [LTC2941](https://www.analog.com/en/products/ltc2941.html), 
[LTC2941-1](https://www.analog.com/en/products/ltc2941-1.html), 
[LTC2942](https://www.analog.com/en/products/ltc2942.html), 
and [LTC2942-1](https://www.analog.com/en/products/ltc2942-1.html) battery gas 
gauges.


## Usage

Search the library using the Library Manager of Arduino IDE, or download it 
directly via GitHub.

Connect a LTC2941 or LTC2942 series device to your Arduino board according to 
the datasheet, and run the example sketch `BatteryReadings.ino`.

You can find more API functions in `src/LTC2942.h`, which are pretty much 
self-explanatory if you have read the datasheet.


## Compatibility

This library has been tested on SAMD21 and ESP8266 only. However, it should 
also be compatible with any architecture that provides `Wire.h` I<sup>2</sup>C 
support.


## License

MIT
