#pragma once

#include <cstdint>
#include <iostream>


struct Parameters
{
	bool invalid_parameters = false;
	bool run_test = false;
	bool run_server = false;
	bool show_config = false;
	bool show_help = false;
	uint32_t lock_timeout_ms = 0;
	uint32_t test_timeout_ms = 0;
	const char* service = nullptr;
	const char* board = nullptr;
	const char* file = nullptr;
};

extern std::ostream& operator<<(std::ostream& out, const Parameters& params);
extern std::string ToString(const Parameters& params);

extern Parameters ParseCommandLineArguments(int argc, char* const argv[]);
extern void PrintHelp(const char* app_name);

