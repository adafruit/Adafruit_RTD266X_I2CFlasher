/* Read, write, verify and erase the SPI flash chip connected to an RTD266X video chip 
   from an Arduino (32u4 tested, but ought to work with any)

   Essentially just a port of      https://github.com/ghent360/RTD-2660-Programmer 
   but handy if you want to have a 'standalone' programmer using something like
   a Feather Adalogger https://www.adafruit.com/products/2795

   Tested with RTD2662 but ought to work w/RTD2660 also (2668 uses a different protocol tho)

   Connect GND, SDA and SCL -> RTD programming pins (on some boards, they are on the VGA port)
      don't add any pullups, you'll use the 3.3V pu's on the RTD board
   Connect SCK, MOSI, MISO, CS -> MicroSD card
*/

#include <SD.h>
#include "Wire.h"
#include "rtd266x_main.h"
#if defined(__AVR__)
   extern "C" { 
     #include "utility/twi.h"  // from Wire library, so we can do bus scanning
   }
#endif

// name of file to read to
#define SAVENAME "FLSHSAVE.BIN"
// name of file for write/verify
#define READNAME "RTDFLASH.BIN"

#define SD_CS        10         // pin connected to SD chip select
#define TWI_FREQ     200000     // only changed on AVR (shrug)

File dataFile;
const FlashDesc* chip;

void error(char *str) {
  Serial.println(str);
  while (1);
}
  
void setup(void) 
{
  while (!Serial);
  Serial.begin(115200);

  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CS)) {
    error("Card failed, or not present");
  }
  Serial.println(F("Card initialized."));

  Wire.begin();

#if defined(__AVR__)
  uint8_t data;
  if (twi_writeTo(0x4A, &data, 0, 1, 1)) {
    error("Couldn't find i2c device 0x4A");
  }
  Serial.println(F("Found 0x4A"));

  TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
#endif

  uint8_t retries = 4;
  
  while (1) {
    if ( ! retries-- ) {
      error("Could not connect!");    
    }
    if (!WriteReg(0x6f, 0x80)) {  // Enter ISP mode
      Serial.println(F("Write to 0x6F failed"));
      continue;
    }
    uint8_t b = ReadReg(0x6f);
    if (!(b & 0x80)) {
      Serial.println(F("Can't enable ISP mode"));
      continue;
    }
    uint32_t jedec_id = SPICommonCommand(E_CC_READ, 0x9f, 3, 0, 0);
    Serial.print(F("JEDEC ID: 0x")); Serial.println(jedec_id, HEX);
    chip = FindChip(jedec_id);
    if (NULL == chip) {
      Serial.println(F("Unknown chip ID"));
    }
    break;
  }
  
  Serial.print(F("Manufacturer "));
  uint32_t id = GetManufacturerId(chip->jedec_id);
  switch (id) {
    case 0x20: Serial.println(F("ST"));         break;
    case 0xef: Serial.println(F("Winbond"));    break;
    case 0x1f: Serial.println(F("Atmel"));      break;
    case 0xc2: Serial.println(F("Macronix"));   break;
    case 0xbf: Serial.println(F("Microchip"));  break;
    default:   Serial.println(F("Unknown"));    break;
  }

  Serial.print(F("Chip: "));      Serial.println(chip->device_name);
  Serial.print(F("Size (KB): ")); Serial.println(chip->size_kb);

  SetupChipCommands(chip->jedec_id);
}

void loop(void) 
{
  while (Serial.available()) { Serial.read(); }
  Serial.println(F("RTD FLASH TOOL - Hello! Press E (rase), W (rite), V (erify), or R (ead) to begin"));

  while (!Serial.available());
  char cmd = toupper(Serial.read());

  uint32_t starttime = millis();

  if (cmd == 'E') {
    EraseFlash();
    Serial.print(millis() - starttime); Serial.println(F(" ms"));
  }
  
  if (cmd == 'R') {
     // open the file. note that only one file can be open at a time,
     // so you have to close this one before opening another.
     // Open up the file we're going to log to!
     dataFile = SD.open(SAVENAME, FILE_WRITE);
     if (! dataFile) {
       Serial.println(F("Error opening file"));
       return;
     }
    
    Serial.println(F("Dumping FLASH to disk"));
     if (! SaveFlash(&dataFile, chip->size_kb * 1024)) {
      Serial.println(F("**** FAILED ****"));
    }
    Serial.println(F("OK!"));
    dataFile.flush();
    dataFile.close();
    Serial.print(millis() - starttime); Serial.println(F(" ms"));
  }
  
  if (cmd == 'V') {
     dataFile = SD.open(READNAME, FILE_READ);
     if (! dataFile) {
       Serial.println(F("Error opening file"));
       return;
     }
    Serial.println(F("Verifying FLASH from disk"));
     if (! VerifyFlash(&dataFile, dataFile.size())) {
      Serial.println(F("**** FAILED ****"));
    }
    Serial.println(F("OK!"));
    dataFile.close();
    Serial.print(millis() - starttime); Serial.println(" ms");
  }  

  if (cmd == 'W') {
     dataFile = SD.open(READNAME, FILE_READ);
     if (! dataFile) {
       Serial.println(F("Error opening file"));
       return;
     }

    Serial.println(F("Writing FLASH from disk"));
    if (! ProgramFlash(&dataFile, chip->size_kb * 1024)) {
      Serial.println(F("**** FAILED ****"));
    }
    Serial.println(F("OK!"));
    dataFile.close();

    Serial.print(millis() - starttime); Serial.println(" ms");
  } 
}


