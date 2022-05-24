#pragma once

#include <cstdint>


constexpr char const* CONFIG_FILE = "/etc/hardware-test-agent.conf";

constexpr uint32_t DEFAULT_LOCK_TIMEOUT_MS = 5 * 60 * 1000;
constexpr uint32_t DEFAULT_TEST_TIMEOUT_MS = 5 * 60 * 1000;


class Interface
{
public:
	virtual ~Interface() = default;
};
