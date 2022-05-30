#include "Test.hpp"
#include "RunTestStlink.hpp"
#include "Params.hpp"
#include "Config.hpp"
#include "Application.hpp"
#include "SystemExecuteMock.hpp"


TEST_CASE(RunTestStlink_HappyCase)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;
	params.system_frequency_hz = 123000000;
	params.trace_frequency_hz = 2000000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(0);
	mock.ExpectRun("/usr/local/bin/st-trace --serial programmer-id --clock=123 --trace=2000000 --verbose=49", true);
	mock.ExpectReads("Testing...\nEOT P");
	mock.ExpectKill();
	mock.ExpectWait(1);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_SUCCESS);
}

TEST_CASE(RunTestStlink_HappyCase_NoSystemClock)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;
	params.trace_frequency_hz = 2000000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(0);
	mock.ExpectRun("/usr/local/bin/st-trace --serial programmer-id --trace=2000000 --verbose=49", true);
	mock.ExpectReads("Testing...\nEOT P");
	mock.ExpectKill();
	mock.ExpectWait(1);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_SUCCESS);
}

TEST_CASE(RunTestStlink_HappyCase_NoTraceClock)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;
	params.system_frequency_hz = 123000000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(0);
	mock.ExpectRun("/usr/local/bin/st-trace --serial programmer-id --clock=123 --verbose=49", true);
	mock.ExpectReads("Testing...\nEOT P");
	mock.ExpectKill();
	mock.ExpectWait(1);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_SUCCESS);
}

TEST_CASE(RunTestStlink_InvalidProgrammer)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;

	ConfigSection config("board");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_INVALID_CONFIG);
}

TEST_CASE(RunTestStlink_InvalidFlashBase)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");

	SystemExecuteMock mock;

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_INVALID_CONFIG);
}

TEST_CASE(RunTestStlink_FlashLaunchFailed)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", false);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_TOOL_FAILED);
}

TEST_CASE(RunTestStlink_FlashFailed)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(1);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_TOOL_FAILED);
}

TEST_CASE(RunTestStlink_TraceLaunchFailed)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(0);
	mock.ExpectRun("/usr/local/bin/st-trace --serial programmer-id --verbose=49", false);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_TOOL_FAILED);
}

TEST_CASE(RunTestStlink_TestsFailed)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 10000;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(0);
	mock.ExpectRun("/usr/local/bin/st-trace --serial programmer-id --verbose=49", true);
	mock.ExpectReads("Testing...\nEOT F");
	mock.ExpectKill();
	mock.ExpectWait(1);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_TESTS_FAILED);
}

TEST_CASE(RunTestStlink_Timeout)
{
	Parameters params = {};
	params.file = "filename.bin";
	params.test_timeout_ms = 5;

	ConfigSection config("board");
	config.Set("id", "programmer-id");
	config.Set("flash-base", "0x12345");

	SystemExecuteMock mock;
	mock.ExpectRun("/usr/local/bin/st-flash --reset --serial programmer-id write filename.bin 0x12345", true);
	mock.ExpectRead(-1);
	mock.ExpectWait(0);
	mock.ExpectRun("/usr/local/bin/st-trace --serial programmer-id --verbose=49", true);
	mock.ExpectReads("Testing...\n");
	mock.ExpectRead(-1, 10);
	mock.ExpectKill();
	mock.ExpectWait(1);

	ASSERT(RunTestStlink(params, config, mock) == EXITCODE_TESTS_TIMEOUT);
}


