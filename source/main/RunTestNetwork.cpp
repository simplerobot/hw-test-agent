#include "RunTestNetwork.hpp"
#include "Application.hpp"
#include "Network.hpp"
#include "Config.hpp"
#include "Params.hpp"
#include "Protocol.hpp"
#include "HWTA.hpp"
#include <fstream>
#include <cstring>
#include <arpa/inet.h>


static const char* ErrorString(uint8_t status)
{
	switch (status)
	{
	case HWTA_RESPONSE_OK:
		return "OK";
	case HWTA_RESPONSE_ERROR:
		return "Error";
	default:
		return "Unknown";
	}
}
extern int RunTestNetwork(const Parameters& params, const ConfigSection& config)
{
	Network network;

	std::ifstream firmware(params.file, std::ios::binary);
	if (!firmware)
	{
		std::printf("ERROR: Error opening file '%s'\n", params.file);
		return EXITCODE_TOOL_FAILED;
	}

	return RunTestNetwork(params, config, firmware, network);
}

extern int RunTestNetwork(const Parameters& params, const ConfigSection& config, std::istream& firmware, NetworkInterface& network)
{
	// Verify we have the configuration needed.

	std::string host = config.Get("host", "");
	if (host.empty())
	{
		std::printf("ERROR: board '%s' configuration missing host.\n", params.board);
		return EXITCODE_INVALID_CONFIG;
	}

	std::string port = config.Get("port", "");
	if (port.empty())
	{
		std::printf("ERROR: board '%s' configuration missing port.\n", params.board);
		return EXITCODE_INVALID_CONFIG;
	}

	// Connect to to the server.
	if (!network.Connect(host, port))
	{
		std::printf("ERROR: Unable to connect to host '%s' port '%s'\n", host.c_str(), port.c_str());
		return EXITCODE_NETWORK_ERROR;
	}

	// Send and receive a header.

	Protocol connection(network);

	connection.Write(HWTA_SIGNATURE);
	connection.Write(HWTA_VERSION_CURRENT);

	uint32_t remote_signature = 0;
	connection.Read(remote_signature);
	if (remote_signature != HWTA_SIGNATURE)
	{
		std::printf("ERROR: Host '%s' port '%s' does not appear to be running HWTA protocol.\n", host.c_str(), port.c_str());
		return EXITCODE_NETWORK_ERROR;
	}

	uint32_t remote_version = 0;
	connection.Read(remote_version);
	if (remote_version < HWTA_VERSION_MIN_SUPPORTED)
	{
		std::printf("ERROR: Host '%s' port '%s' is running version %x, but we only support down to %x.\n", host.c_str(), port.c_str(), remote_version, HWTA_VERSION_MIN_SUPPORTED);
		return EXITCODE_NETWORK_ERROR;
	}

	// Send a test firmware command.
	connection.Write(HWTA_COMMAND_TEST_FIRMWARE);
	connection.Write(params.board);
	connection.Write(params.lock_timeout_ms);
	connection.Write(params.test_timeout_ms);
	connection.Write(params.system_frequency_hz);
	connection.Write(params.trace_frequency_hz);
	connection.Write(firmware);

	uint8_t status = HWTA_RESPONSE_ERROR;
	connection.Read(status);
	if (!connection)
	{
		std::printf("ERROR: Host '%s' port '%s' dropped connection.\n", host.c_str(), port.c_str());
		return EXITCODE_NETWORK_ERROR;
	}
	if (status != HWTA_RESPONSE_OK)
	{
		std::printf("ERROR: Host '%s' port '%s' refused to run tests on board '%s' with error %s (%d).\n", host.c_str(), port.c_str(), params.board, ErrorString(status), status);
		return EXITCODE_NETWORK_ERROR;
	}

	// Read output as it is generated by the host program and echo it to our output.
	uint8_t output = 0;
	connection.Read(output);
	while (output != 0)
	{
		std::putchar(output);
		output = 0;
		connection.Read(output);
	}

	if (!connection)
	{
		std::printf("ERROR: Host '%s' port '%s' dropped connection during tests.\n", host.c_str(), port.c_str());
		return EXITCODE_NETWORK_ERROR;
	}

	uint8_t test_status = HWTA_RESPONSE_ERROR;
	connection.Read(test_status);

	connection.Write(HWTA_COMMAND_DONE);

	if (test_status == HWTA_RESPONSE_TESTS_PASS)
	{
		std::printf("Tests passed on host '%s' port '%s'\n", host.c_str(), port.c_str());
		return EXITCODE_SUCCESS;
	}

	if (test_status == HWTA_RESPONSE_TESTS_FAIL)
	{
		std::printf("Tests failed on host '%s' port '%s'\n", host.c_str(), port.c_str());
		return EXITCODE_TESTS_FAILED;
	}

	if (test_status == HWTA_RESPONSE_TESTS_TIMEOUT)
	{
		std::printf("Tests timed out on host '%s' port '%s'\n", host.c_str(), port.c_str());
		return EXITCODE_TESTS_TIMEOUT;
	}

	std::printf("ERROR: Host '%s' port '%s' returned unexpected test status '%s' (%d).\n", host.c_str(), port.c_str(), ErrorString(test_status), test_status);
	return EXITCODE_NETWORK_ERROR;
}
