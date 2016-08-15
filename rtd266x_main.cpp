#include "Arduino.h"
#include "rtd266x_main.h"


static const FlashDesc FlashDevices[] = {
    // name,        Jedec ID,    sizeK, page size, block sizeK
    {"AT25DF041A" , 0x1F4401,      512,       256, 64},
    {"AT25DF161"  , 0x1F4602, 2 * 1024,       256, 64},
    {"AT26DF081A" , 0x1F4501, 1 * 1024,       256, 64},
    {"AT26DF0161" , 0x1F4600, 2 * 1024,       256, 64},
    {"AT26DF161A" , 0x1F4601, 2 * 1024,       256, 64},
    {"AT25DF321" ,  0x1F4701, 4 * 1024,       256, 64},
    {"AT25DF512B" , 0x1F6501,       64,       256, 32},
    {"AT25DF512B" , 0x1F6500,       64,       256, 32},
    {"AT25DF021"  , 0x1F3200,      256,       256, 64},
    {"AT26DF641" ,  0x1F4800, 8 * 1024,       256, 64},
    // Manufacturer: ST 
    {"M25P05"     , 0x202010,       64,       256, 32},
    {"M25P10"     , 0x202011,      128,       256, 32},
    {"M25P20"     , 0x202012,      256,       256, 64},
    {"M25P40"     , 0x202013,      512,       256, 64},
    {"M25P80"     , 0x202014, 1 * 1024,       256, 64},
    {"M25P16"     , 0x202015, 2 * 1024,       256, 64},
    {"M25P32"     , 0x202016, 4 * 1024,       256, 64},
    {"M25P64"     , 0x202017, 8 * 1024,       256, 64},
    // Manufacturer: Windbond 
    {"W25X10"     , 0xEF3011,      128,       256, 64},
    {"W25X20"     , 0xEF3012,      256,       256, 64},
    {"W25X40"     , 0xEF3013,      512,       256, 64},
    {"W25X80"     , 0xEF3014, 1 * 1024,       256, 64},
    // Manufacturer: Macronix 
    {"MX25L512"   , 0xC22010,       64,       256, 64},
    {"MX25L3205"  , 0xC22016, 4 * 1024,       256, 64},
    {"MX25L6405"  , 0xC22017, 8 * 1024,       256, 64},
    {"MX25L8005"  , 0xC22014,     1024,       256, 64},
    // Microchip
    {"SST25VF512" , 0xBF4800,       64,       256, 32},
    {"SST25VF032" , 0xBF4A00, 4 * 1024,       256, 32},
    {NULL , 0, 0, 0, 0}
};



uint32_t SPICommonCommand(ECommondCommandType cmd_type,
    uint8_t cmd_code,
    uint8_t num_reads,
    uint8_t num_writes,
    uint32_t write_value) {
  num_reads &= 3;
  num_writes &= 3;
  write_value &= 0xFFFFFF;
  uint8_t reg_value = (cmd_type << 5) | 
    (num_writes << 3) |
    (num_reads << 1);

    Serial.print(F("cmd type: ")); Serial.print(cmd_type); Serial.print(F(" code: ")); Serial.print(cmd_code);
    Serial.print(F(" r len: ")); Serial.print(num_reads); Serial.print(F(" w len: ")); Serial.print(num_writes);
    Serial.print(F(" val: 0x")); Serial.println(write_value);
    

  WriteReg(0x60, reg_value);
  WriteReg(0x61, cmd_code);
  switch (num_writes) {
  case 3:
    WriteReg(0x64, write_value >> 16);
    WriteReg(0x65, write_value >> 8);
    WriteReg(0x66, write_value);
    break;
  case 2:
    WriteReg(0x64, write_value >> 8);
    WriteReg(0x65, write_value);
    break;
  case 1:
    WriteReg(0x64, write_value);
    break;
  }
  WriteReg(0x60, reg_value | 1); // Execute the command
  uint8_t b;
  do {
    b = ReadReg(0x60);
  } while (b & 1);  // TODO: add timeout and reset the controller
  switch (num_reads) {
  case 0: return 0;
  case 1: return ReadReg(0x67);
  case 2: return (ReadReg(0x67) << 8) | ReadReg(0x68);
  case 3: return (ReadReg(0x67) << 16) | (ReadReg(0x68) << 8) | ReadReg(0x69);
  }
  return 0;
}

void SPIRead(uint32_t address, uint8_t *data, int32_t len) {
  WriteReg(0x60, 0x46);
  WriteReg(0x61, 0x3);
  WriteReg(0x64, address>>16);
  WriteReg(0x65, address>>8);
  WriteReg(0x66, address);
  WriteReg(0x60, 0x47); // Execute the command
  uint8_t b;
  do {
    b = ReadReg(0x60);
  } while (b & 1);  // TODO: add timeout and reset the controller
  while (len > 0) {
    int32_t read_len = len;
    if (read_len > 32)  // max 32 bytes at a time
      read_len = 32;
    ReadBytesFromAddr(0x70, data, read_len);
    data += read_len;
    len -= read_len;
  }
}



const FlashDesc* FindChip(uint32_t jedec_id) {
  const FlashDesc* chip = FlashDevices;
  while (chip->jedec_id != 0) {
    if (chip->jedec_id == jedec_id)
      return chip;
    chip++;
  }
  return NULL;
}


uint8_t SPIComputeCRC(uint32_t start, uint32_t end) {
  WriteReg(0x64, start >> 16);
  WriteReg(0x65, start >> 8);
  WriteReg(0x66, start);

  WriteReg(0x72, end >> 16);
  WriteReg(0x73, end >> 8);
  WriteReg(0x74, end);

  WriteReg(0x6f, 0x84);
  uint8_t b;
  do
  {
    b = ReadReg(0x6f);
  } while (!(b & 0x2));  // TODO: add timeout and reset the controller
  return ReadReg(0x75);
}

uint8_t GetManufacturerId(uint32_t jedec_id) {
  return jedec_id >> 16;
}

void SetupChipCommands(uint32_t jedec_id) {
  uint8_t manufacturer_id = GetManufacturerId(jedec_id);
  switch (manufacturer_id) {
  case 0xEF:
    // These are the codes for Winbond
    WriteReg(0x62, 0x6);  // Flash Write enable op code
    WriteReg(0x63, 0x50); // Flash Write register op code
    WriteReg(0x6a, 0x3);  // Flash Read op code.
    WriteReg(0x6b, 0xb);  // Flash Fast read op code.
    WriteReg(0x6d, 0x2);  // Flash program op code.
    WriteReg(0x6e, 0x5);  // Flash read status op code.
    break;
  default:
    Serial.println(F("Can not handle this chip"));
    break;
  }
}


bool SaveFlash(File *f, uint32_t chip_size) {
  uint8_t buffer[128];
  uint32_t addr = 0;
  InitCRC();
  
  do {
    Serial.print(F("Reading addr $")); Serial.println(addr, HEX);
    
    SPIRead(addr, buffer, sizeof(buffer));
    f->write(buffer, sizeof(buffer));
    
    ProcessCRC(buffer, sizeof(buffer));
    addr += sizeof(buffer);
  } while (addr < chip_size);

  Serial.println(F("Done"));

  uint8_t data_crc = GetCRC();
  uint8_t chip_crc = SPIComputeCRC(0, chip_size - 1);
  Serial.print(F("Received data CRC ")); Serial.println(data_crc, HEX);
  Serial.print(F("Chip CRC ")); Serial.println(chip_crc, HEX);
  return data_crc == chip_crc;
}


bool VerifyFlash(File *f, uint32_t file_size) {
  uint32_t addr = 0;
  uint8_t buffer[64], buffer2[64];

  InitCRC();
  do {
    memset(buffer, 0xFF, sizeof(buffer));
    memset(buffer2, 0xFF, sizeof(buffer2));
      
    Serial.print(F("Reading addr $")); Serial.println(addr, HEX);
    SPIRead(addr, buffer, sizeof(buffer));
    f->read(buffer2, sizeof(buffer));
    if (memcmp(buffer, buffer2, sizeof(buffer)) != 0) {
        Serial.println(F("Verification failed!"));
        return false;
      }    
    ProcessCRC(buffer, sizeof(buffer));
    addr += sizeof(buffer);
  } while (addr < file_size);
  
  Serial.println(F("Done"));
  uint8_t data_crc = GetCRC();
  uint8_t chip_crc = SPIComputeCRC(0, file_size - 1);
  Serial.print(F("Received data CRC ")); Serial.println(data_crc, HEX);
  Serial.print(F("Chip CRC ")); Serial.println(chip_crc, HEX);
  return data_crc == chip_crc;
}

bool ShouldProgramPage(uint8_t* buffer, uint32_t size) {
  for (uint32_t idx = 0; idx < size; ++idx) {
    if (buffer[idx] != 0xff) return true;
  }
  return false;
}


bool EraseFlash(void) {
  Serial.println(F("Erasing..."));
  SPICommonCommand(E_CC_WRITE_AFTER_EWSR, 1, 0, 1, 0); // Unprotect the Status Register
  SPICommonCommand(E_CC_WRITE_AFTER_WREN, 1, 0, 1, 0); // Unprotect the flash
  SPICommonCommand(E_CC_ERASE, 0xc7, 0, 0, 0);         // Chip Erase
  Serial.println(F("done"));
}

bool ProgramFlash(File *f, uint32_t chip_size) {
  uint32_t prog_size = f->size();

  EraseFlash();

  // Arduino i2c can only handle 16 bytes at a time!
  uint8_t buffer[256];
  uint8_t b;
  uint32_t addr = 0;
  
  uint32_t remaining_len = prog_size;
  InitCRC();
  do
  {
    // Wait for programming cycle to finish
    do {
      b = ReadReg(0x6f);
    } while (b & 0x40);

    Serial.print(F("Writing addr $")); Serial.println(addr, HEX);
    // Fill with 0xff in case we read a partial buffer.
    memset(buffer, 0xff, sizeof(buffer));

    uint16_t len = sizeof(buffer);
    if (len > remaining_len) {
      len = remaining_len;
    }
    uint16_t r = f->read(buffer, len);
    
    if (r != len) {
      Serial.print(F("Failed to read file"));
      return false;
    }
    remaining_len -= len;

    if (ShouldProgramPage(buffer, sizeof(buffer))) {
      // Set program size-1
      WriteReg(0x71, 255);

      // Set the programming address
      WriteReg(0x64, addr >> 16);
      WriteReg(0x65, addr >> 8);
      WriteReg(0x66, addr);

      // Write the content to register 0x70
      // we can only write 16 bytes at a time tho
      for (uint16_t x=0; x < 256; x += 16) {
        WriteBytesToAddr(0x70, buffer+x, 16); // write 16 at a time
      }
      WriteReg(0x6f, 0xa0); // Start Programing
    }
    ProcessCRC(buffer, sizeof(buffer));
    addr += sizeof(buffer);
  } while ((addr < chip_size) && (remaining_len != 0));

  // Wait for programming cycle to finish
  do {
    b = ReadReg(0x6f);
  } while (b & 0x40);

  SPICommonCommand(E_CC_WRITE_AFTER_EWSR, 1, 0, 1, 0x1c); // Unprotect the Status Register
  SPICommonCommand(E_CC_WRITE_AFTER_WREN, 1, 0, 1, 0x1c); // Protect the flash

  uint8_t data_crc = GetCRC();
  uint8_t chip_crc = SPIComputeCRC(0, addr - 1);
  Serial.print(F("Received data CRC ")); Serial.println(data_crc, HEX);
  Serial.print(F("Chip CRC ")); Serial.println(chip_crc, HEX);
  return data_crc == chip_crc;
}

