# Adafruit_RTD266X_I2CFlasher
Read, write, verify and erase the SPI flash chip connected to an RTD266X video chip from an Arduino


   Essentially just a port of      https://github.com/ghent360/RTD-2660-Programmer 
   but handy if you want to have a 'standalone' programmer using something like
   a Feather Adalogger https://www.adafruit.com/products/2795


   * Tested with RTD2662 but ought to work w/RTD2660 also (2668 uses a different protocol tho)

   * Connect GND, SDA and SCL -> RTD programming pins (on some boards, they are on the VGA port). Don't add any pullups, you'll use the 3.3V pu's on the RTD board

   * Connect SCK, MOSI, MISO, CS -> MicroSD card