#pragma once

#include <cstdint>


const uint32_t HWTA_SIGNATURE = 0x48575441; // 'HWTA'

const uint32_t HWTA_VERSION_CURRENT = 0x00000100;
const uint32_t HWTA_VERSION_MIN_SUPPORTED = 0x00000100;

const uint8_t HWTA_COMMAND_DONE = 0;
const uint8_t HWTA_COMMAND_TEST_FIRMWARE = 1;

const uint8_t HWTA_RESPONSE_OK = 0;
const uint8_t HWTA_RESPONSE_ERROR = 1;
const uint8_t HWTA_RESPONSE_TESTS_PASS = 10;
const uint8_t HWTA_RESPONSE_TESTS_FAIL = 11;
const uint8_t HWTA_RESPONSE_TESTS_TIMEOUT = 12;


