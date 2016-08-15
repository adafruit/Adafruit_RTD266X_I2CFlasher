#ifndef _RTD266X_MAIN_H_
#define _RTD266X_MAIN_H_

#include <Arduino.h>
#include <SD.h>
#include "crc.h"

enum ECommondCommandType {
  E_CC_NOOP = 0,
  E_CC_WRITE = 1,
  E_CC_READ = 2,
  E_CC_WRITE_AFTER_WREN = 3,
  E_CC_WRITE_AFTER_EWSR = 4,
  E_CC_ERASE = 5
};

struct FlashDesc {
  const char* device_name;
  uint32_t    jedec_id;
  uint32_t    size_kb;
  uint32_t    page_size;
  uint32_t    block_size_kb;
};

void PrintHex(const byte * data, const uint32_t numBytes);

uint32_t SPICommonCommand(ECommondCommandType cmd_type,
    uint8_t cmd_code,
    uint8_t num_reads,
    uint8_t num_writes,
    uint32_t write_value);

bool WriteReg(uint8_t a, uint8_t d);
uint32_t ReadReg(uint8_t a);
bool ReadBytesFromAddr(uint8_t reg, uint8_t* dest, uint8_t len);
const FlashDesc* FindChip(uint32_t jedec_id);
uint8_t GetManufacturerId(uint32_t jedec_id);
void SetupChipCommands(uint32_t jedec_id);
bool EraseFlash(void);
bool ProgramFlash(File *f, uint32_t chip_size);
bool WriteBytesToAddr(uint8_t reg, uint8_t* values, uint8_t len);
bool VerifyFlash(File *f, uint32_t chip_size);
bool ShouldProgramPage(uint8_t* buffer, uint32_t size);
bool SaveFlash(File *f, uint32_t chip_size);

#endif
