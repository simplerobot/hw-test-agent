#include "Test.hpp"
#include "RunServer.hpp"
#include "Params.hpp"
#include "Config.hpp"
#include "RunTestMock.hpp"
#include "NetworkMock.hpp"


TEST_CASE(RunServer_HappyCase)
{
	Parameters params = {};
	ConfigFile config;
	config.Add("test-section");
	NetworkMock network_mock;
	Parameters expected_params = {};
	expected_params.board = "board-name";
	expected_params.file = "/tmp/hwta.fw.XXXXXX";
	expected_params.lock_timeout_ms = 0x12345678;
	expected_params.test_timeout_ms = 0x2468ABCD;
	expected_params.system_frequency_hz = 0x13691215;
	expected_params.trace_frequency_hz = 0x48121620;
	RunTestMock run_test_mock(expected_params, config, "test-output", 0); // EXITCODE_SUCCESS
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(1); // HWTA_COMMAND_TEST_FIRMWARE
	network_mock.AddExpectRead("board-name", 11);
	network_mock.AddExpectRead32(0x12345678);
	network_mock.AddExpectRead32(0x2468ABCD);
	network_mock.AddExpectRead32(0x13691215);
	network_mock.AddExpectRead32(0x48121620);
	network_mock.AddExpectRead8(5);
	network_mock.AddExpectRead("abcde", 5);
	network_mock.AddExpectWrite8(0); // HWTA_RESPONSE_OK
	network_mock.AddExpectWrite("test-output", 12);
	network_mock.AddExpectWrite8(10); // HWTA_RESPONSE_TESTS_PASS
	network_mock.AddExpectRead8(0); // HWTA_COMMAND_DONE

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 0); // EXITCODE_SUCCESS
}

TEST_CASE(RunServer_MissingSignature)
{
	Parameters params = {};
	ConfigFile config;
	NetworkMock network_mock;
	RunTestMock run_test_mock;
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTB", 4);

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 8); // EXITCODE_NETWORK_ERROR
}

TEST_CASE(RunServer_IncompatibleVersion)
{
	Parameters params = {};
	ConfigFile config;
	NetworkMock network_mock;
	RunTestMock run_test_mock;
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x000000FF);

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 8); // EXITCODE_NETWORK_ERROR
}

TEST_CASE(RunServer_UnknownCommand)
{
	Parameters params = {};
	ConfigFile config;
	NetworkMock network_mock;
	RunTestMock run_test_mock;
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(2); // Undefined command

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 8); // EXITCODE_NETWORK_ERROR
}

TEST_CASE(RunServer_IncompleteDownloadCommand)
{
	Parameters params = {};
	ConfigFile config;
	NetworkMock network_mock;
	RunTestMock run_test_mock;
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(1); // HWTA_COMMAND_TEST_FIRMWARE
	network_mock.AddExpectRead("board-name", 11);
	network_mock.AddExpectRead32(0x12345678);
	network_mock.AddExpectRead32(0x2468ABCD);
	network_mock.AddExpectRead32(0x13691215);
	network_mock.AddExpectRead32(0x48121620);
	network_mock.AddExpectRead8(5);
	network_mock.AddExpectRead("abcde", 4);

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 8); // EXITCODE_NETWORK_ERROR
}

TEST_CASE(RunServer_TestsFailed)
{
	Parameters params = {};
	ConfigFile config;
	config.Add("test-section");
	NetworkMock network_mock;
	Parameters expected_params = {};
	expected_params.board = "board-name";
	expected_params.file = "/tmp/hwta.fw.XXXXXX";
	expected_params.lock_timeout_ms = 0x12345678;
	expected_params.test_timeout_ms = 0x2468ABCD;
	expected_params.system_frequency_hz = 0x13691215;
	expected_params.trace_frequency_hz = 0x48121620;
	RunTestMock run_test_mock(expected_params, config, "test-output", 1); // EXITCODE_TESTS_FAILED
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(1); // HWTA_COMMAND_TEST_FIRMWARE
	network_mock.AddExpectRead("board-name", 11);
	network_mock.AddExpectRead32(0x12345678);
	network_mock.AddExpectRead32(0x2468ABCD);
	network_mock.AddExpectRead32(0x13691215);
	network_mock.AddExpectRead32(0x48121620);
	network_mock.AddExpectRead8(5);
	network_mock.AddExpectRead("abcde", 5);
	network_mock.AddExpectWrite8(0); // HWTA_RESPONSE_OK
	network_mock.AddExpectWrite("test-output", 12);
	network_mock.AddExpectWrite8(11); // HWTA_RESPONSE_TESTS_FAIL
	network_mock.AddExpectRead8(0); // HWTA_COMMAND_DONE

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 0); // EXITCODE_SUCCESS
}

TEST_CASE(RunServer_LockFailed)
{
	Parameters params = {};
	ConfigFile config;
	config.Add("test-section");
	NetworkMock network_mock;
	Parameters expected_params = {};
	expected_params.board = "board-name";
	expected_params.file = "/tmp/hwta.fw.XXXXXX";
	expected_params.lock_timeout_ms = 0x12345678;
	expected_params.test_timeout_ms = 0x2468ABCD;
	expected_params.system_frequency_hz = 0x13691215;
	expected_params.trace_frequency_hz = 0x48121620;
	RunTestMock run_test_mock(expected_params, config, "test-output", 5); // EXITCODE_LOCK_FAILED
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(1); // HWTA_COMMAND_TEST_FIRMWARE
	network_mock.AddExpectRead("board-name", 11);
	network_mock.AddExpectRead32(0x12345678);
	network_mock.AddExpectRead32(0x2468ABCD);
	network_mock.AddExpectRead32(0x13691215);
	network_mock.AddExpectRead32(0x48121620);
	network_mock.AddExpectRead8(5);
	network_mock.AddExpectRead("abcde", 5);
	network_mock.AddExpectWrite8(0); // HWTA_RESPONSE_OK
	network_mock.AddExpectWrite("test-output", 12);
	network_mock.AddExpectWrite8(12); // HWTA_RESPONSE_TESTS_TIMEOUT
	network_mock.AddExpectRead8(0); // HWTA_COMMAND_DONE

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 0); // EXITCODE_SUCCESS
}

TEST_CASE(RunServer_TestsTimeout)
{
	Parameters params = {};
	ConfigFile config;
	config.Add("test-section");
	NetworkMock network_mock;
	Parameters expected_params = {};
	expected_params.board = "board-name";
	expected_params.file = "/tmp/hwta.fw.XXXXXX";
	expected_params.lock_timeout_ms = 0x12345678;
	expected_params.test_timeout_ms = 0x2468ABCD;
	expected_params.system_frequency_hz = 0x13691215;
	expected_params.trace_frequency_hz = 0x48121620;
	RunTestMock run_test_mock(expected_params, config, "test-output", 2); // EXITCODE_TESTS_TIMEOUT
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(1); // HWTA_COMMAND_TEST_FIRMWARE
	network_mock.AddExpectRead("board-name", 11);
	network_mock.AddExpectRead32(0x12345678);
	network_mock.AddExpectRead32(0x2468ABCD);
	network_mock.AddExpectRead32(0x13691215);
	network_mock.AddExpectRead32(0x48121620);
	network_mock.AddExpectRead8(5);
	network_mock.AddExpectRead("abcde", 5);
	network_mock.AddExpectWrite8(0); // HWTA_RESPONSE_OK
	network_mock.AddExpectWrite("test-output", 12);
	network_mock.AddExpectWrite8(12); // HWTA_RESPONSE_TESTS_TIMEOUT
	network_mock.AddExpectRead8(0); // HWTA_COMMAND_DONE

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 0); // EXITCODE_SUCCESS
}

TEST_CASE(RunServer_RunThrows)
{
	Parameters params = {};
	ConfigFile config;
	config.Add("test-section");
	NetworkMock network_mock;
	Parameters expected_params = {};
	expected_params.board = "board-name";
	expected_params.file = "/tmp/hwta.fw.XXXXXX";
	expected_params.lock_timeout_ms = 0x12345678;
	expected_params.test_timeout_ms = 0x2468ABCD;
	expected_params.system_frequency_hz = 0x13691215;
	expected_params.trace_frequency_hz = 0x48121620;
	RunTestMock run_test_mock(expected_params, config, "test-output", 0); // EXITCODE_SUCCESS
	run_test_mock.ExpectRunThrow();
	network_mock.AddExpectWrite("HWTA", 4);
	network_mock.AddExpectWrite32(0x00000100);
	network_mock.AddExpectRead("HWTA", 4);
	network_mock.AddExpectRead32(0x00000100);
	network_mock.AddExpectRead8(1); // HWTA_COMMAND_TEST_FIRMWARE
	network_mock.AddExpectRead("board-name", 11);
	network_mock.AddExpectRead32(0x12345678);
	network_mock.AddExpectRead32(0x2468ABCD);
	network_mock.AddExpectRead32(0x13691215);
	network_mock.AddExpectRead32(0x48121620);
	network_mock.AddExpectRead8(5);
	network_mock.AddExpectRead("abcde", 5);
	network_mock.AddExpectWrite8(0); // HWTA_RESPONSE_OK
	network_mock.AddExpectWrite("test-output", 12);
	network_mock.AddExpectWrite8(1); // HWTA_RESPONSE_ERROR
	network_mock.AddExpectRead8(0); // HWTA_COMMAND_DONE

	int result = RunServer(params, config, network_mock, run_test_mock);

	ASSERT(result == 0); // EXITCODE_SUCCESS
}


