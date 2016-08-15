#include <Arduino.h>

#pragma once


void InitCRC();
void ProcessCRC(const uint8_t *data, int len);
uint8_t GetCRC();
