#pragma once

#include <string>
#include <inttypes.h>

std::string format(const char *fmt, ...);

uint8_t *strMacToArray(const char *mac_str);

int calcSHA256(std::string input, unsigned char *hash);