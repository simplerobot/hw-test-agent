#include "Test.hpp"
#include "RunTestNetwork.hpp"
#include "Config.hpp"
#include "NetworkMock.hpp"
#include <sstream>
#include "Params.hpp"


TEST_CASE(RunTestNetwork_HappyCase)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware("abcd");
	NetworkMock network_mock;
	Parameters params = {};
	params.board = "board-name";
	params.lock_timeout_ms = 0x12345678;
	params.test_timeout_ms = 0x2468ABCD;
	params.system_frequency_hz = 0x13691215;
	params.trace_frequency_hz = 0x48121620;
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectWrite8(1); // test firmware command
	network_mock.AddExpectWrite("board-name", 11); // board name
	network_mock.AddExpectWrite32(0x12345678); // lock timeout
	network_mock.AddExpectWrite32(0x2468ABCD); // test timeout
	network_mock.AddExpectWrite32(0x13691215); // system clock frequency
	network_mock.AddExpectWrite32(0x48121620); // trace clock frequency
	network_mock.AddExpectWrite8(4); // Firmware length
	network_mock.AddExpectWrite("abcd", 4); // Firmware contents
	network_mock.AddExpectRead8(0); // response OK
	network_mock.AddExpectRead("test firmware output\n", 22);
	network_mock.AddExpectRead8(10); // Tests pass
	network_mock.AddExpectWrite8(0); // done command

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 0); // Success
}

TEST_CASE(RunTestNetwork_MissingHost)
{
	ConfigSection config("board");
	config.Set("port", "network.port");
	std::istringstream firmware;
	NetworkMock network_mock;
	Parameters params = {};

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 4); // Invalid configuration
}

TEST_CASE(RunTestNetwork_MissingPort)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	std::istringstream firmware;
	NetworkMock network_mock;
	Parameters params = {};

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 4); // Invalid configuration
}

TEST_CASE(RunTestNetwork_ConnectionFailure)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware;
	NetworkMock network_mock;
	Parameters params = {};
	network_mock.ExpectConnect("host.domain.com", "network.port", false);

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 8); // Network error
}

TEST_CASE(RunTestNetwork_SignatureError)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware;
	NetworkMock network_mock;
	Parameters params = {};
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTB", 4);

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 8); // Network error
}

TEST_CASE(RunTestNetwork_UnsupportedVersion)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware;
	NetworkMock network_mock;
	Parameters params = {};
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x000000FF);

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 8); // Network error
}

TEST_CASE(RunTestNetwork_TestCommandFailure)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware("abcd");
	NetworkMock network_mock;
	Parameters params = {};
	params.board = "board-name";
	params.lock_timeout_ms = 0x12345678;
	params.test_timeout_ms = 0x2468ABCD;
	params.system_frequency_hz = 0x13691215;
	params.trace_frequency_hz = 0x48121620;
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectWrite8(1); // test firmware command
	network_mock.AddExpectWrite("board-name", 11); // board name
	network_mock.AddExpectWrite32(0x12345678); // lock timeout
	network_mock.AddExpectWrite32(0x2468ABCD); // test timeout
	network_mock.AddExpectWrite32(0x13691215); // system clock frequency
	network_mock.AddExpectWrite32(0x48121620); // trace clock frequency
	network_mock.AddExpectWrite8(4); // Firmware length
	network_mock.AddExpectWrite("abcd", 4); // Firmware contents
	network_mock.AddExpectRead8(1); // response Not OK

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 8); // Network error
}

TEST_CASE(RunTestNetwork_TestCommandDrop)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware("abcd");
	NetworkMock network_mock;
	Parameters params = {};
	params.board = "board-name";
	params.lock_timeout_ms = 0x12345678;
	params.test_timeout_ms = 0x2468ABCD;
	params.system_frequency_hz = 0x13691215;
	params.trace_frequency_hz = 0x48121620;
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectWrite8(1); // test firmware command
	network_mock.AddExpectWrite("board-name", 11); // board name
	network_mock.AddExpectWrite32(0x12345678); // lock timeout
	network_mock.AddExpectWrite32(0x2468ABCD); // test timeout
	network_mock.AddExpectWrite32(0x13691215); // system clock frequency
	network_mock.AddExpectWrite32(0x48121620); // trace clock frequency
	network_mock.AddExpectWrite8(4); // Firmware length
	network_mock.AddExpectWrite("abcd", 4); // Firmware contents

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 8); // Network error
}

TEST_CASE(RunTestNetwork_TestsFail)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware("abcd");
	NetworkMock network_mock;
	Parameters params = {};
	params.board = "board-name";
	params.lock_timeout_ms = 0x12345678;
	params.test_timeout_ms = 0x2468ABCD;
	params.system_frequency_hz = 0x13691215;
	params.trace_frequency_hz = 0x48121620;
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectWrite8(1); // test firmware command
	network_mock.AddExpectWrite("board-name", 11); // board name
	network_mock.AddExpectWrite32(0x12345678); // lock timeout
	network_mock.AddExpectWrite32(0x2468ABCD); // test timeout
	network_mock.AddExpectWrite32(0x13691215); // system clock frequency
	network_mock.AddExpectWrite32(0x48121620); // trace clock frequency
	network_mock.AddExpectWrite8(4); // Firmware length
	network_mock.AddExpectWrite("abcd", 4); // Firmware contents
	network_mock.AddExpectRead8(0); // response OK
	network_mock.AddExpectRead("test firmware output\n", 22);
	network_mock.AddExpectRead8(11); // Tests fail
	network_mock.AddExpectWrite8(0); // done command

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 1); // Tests failed
}

TEST_CASE(RunTestNetwork_TestsTimeout)
{
	ConfigSection config("board");
	config.Set("host", "host.domain.com");
	config.Set("port", "network.port");
	std::istringstream firmware("abcd");
	NetworkMock network_mock;
	Parameters params = {};
	params.board = "board-name";
	params.lock_timeout_ms = 0x12345678;
	params.test_timeout_ms = 0x2468ABCD;
	params.system_frequency_hz = 0x13691215;
	params.trace_frequency_hz = 0x48121620;
	network_mock.ExpectConnect("host.domain.com", "network.port", true);
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectWrite8(1); // test firmware command
	network_mock.AddExpectWrite("board-name", 11); // board name
	network_mock.AddExpectWrite32(0x12345678); // lock timeout
	network_mock.AddExpectWrite32(0x2468ABCD); // test timeout
	network_mock.AddExpectWrite32(0x13691215); // system clock frequency
	network_mock.AddExpectWrite32(0x48121620); // trace clock frequency
	network_mock.AddExpectWrite8(4); // Firmware length
	network_mock.AddExpectWrite("abcd", 4); // Firmware contents
	network_mock.AddExpectRead8(0); // response OK
	network_mock.AddExpectRead("test firmware output\n", 22);
	network_mock.AddExpectRead8(12); // Tests timeout
	network_mock.AddExpectWrite8(0); // done command

	int result = RunTestNetwork(params, config, firmware, network_mock);

	ASSERT(result == 2); // Tests timeout
}

