#include "Test.hpp"
#include <vector>
#include "Params.hpp"
#include <getopt.h>


static bool TestCommandLine(const std::vector<const char*>& args)
{
	int argc = args.size();
	char* const* argv = (char* const*)args.data();
	optind = 1;
	auto params = ParseCommandLineArguments(argc, argv);
	return !params.invalid_parameters;
}

TEST_CASE(ParseCommandLineArguments_HappyCase)
{
	ASSERT(TestCommandLine({ "app", "--help" }));
	ASSERT(TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency", "180000000", "--trace-frequency", "2000000" }));
	ASSERT(TestCommandLine({ "app", "--server", "--port", "12345" }));
	ASSERT(TestCommandLine({ "app", "--show-config" }));

	ASSERT(TestCommandLine({ "app", "--help" }));
	ASSERT(TestCommandLine({ "app", "--run", "--board=boardname", "--file=filename", "--lock-timeout=120", "--test-timeout=120", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(TestCommandLine({ "app", "--server", "--port=12345" }));
	ASSERT(TestCommandLine({ "app", "--show-config" }));

	ASSERT(TestCommandLine({ "app", "-h" }));
	ASSERT(TestCommandLine({ "app", "-r", "-b", "boardname", "-f", "filename", "-l", "120", "-t", "120" }));
	ASSERT(TestCommandLine({ "app", "-s", "-p", "12345" }));
	ASSERT(TestCommandLine({ "app", "-c" }));

	ASSERT(TestCommandLine({ "app", "-h" }));
	ASSERT(TestCommandLine({ "app", "-r", "-bboardname", "-ffilename", "-l120", "-t120" }));
	ASSERT(TestCommandLine({ "app", "-s", "-p12345" }));
	ASSERT(TestCommandLine({ "app", "-c" }));

	ASSERT(TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency", "180m", "--trace-frequency", "2m" }));
}

TEST_CASE(ParseCommandLineArguments_Duplicate)
{
	ASSERT(!TestCommandLine({ "app", "--help", "--help" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--system-frequency=180000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=2000000", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--server", "--server", "--port", "12345" }));
	ASSERT(!TestCommandLine({ "app", "--server", "--port", "12345", "--port", "12345" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--show-config" }));
}

TEST_CASE(ParseCommandLineArguments_InvalidValues)
{
	ASSERT(!TestCommandLine({ "app", "extra" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout" }));

	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "abc", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "def" }));

	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=0", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=abc", "--trace-frequency=2000000" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=0" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--system-frequency=180000000", "--trace-frequency=abc" }));
}

TEST_CASE(ParseCommandLineArguments_InvalidArguments)
{
	ASSERT(!TestCommandLine({ "app", "--run", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--board", "boardname" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--file", "filename" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--lock-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--port", "12345" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--server", "--port", "12345" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--system-frequency=180000000" }));
	ASSERT(!TestCommandLine({ "app", "--show-config", "--trace-frequency=2000000" }));
}

TEST_CASE(Parameters_ToString_Empty)
{
	Parameters test;
	std::string expected = "Parameters { }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}

TEST_CASE(Parameters_ToString_HappyCase)
{
	Parameters test;
	test.invalid_parameters = true;
	test.run_test = true;
	test.run_server = true;
	test.show_config = true;
	test.show_help = true;
	test.lock_timeout_ms = 1234;
	test.test_timeout_ms = 5678;
	test.system_frequency_hz = 246;
	test.trace_frequency_hz = 369;
	test.service = "service-name";
	test.board = "board-name";
	test.file = "file-name";
	std::string expected = "Parameters { invalid_parameters = true, run_test = true, run_server = true, show_config = true, show_help = true, lock_timeout_ms = 1234, test_timeout_ms = 5678, system_frequency_hz = 246, trace_frequency_hz = 369, service = 'service-name', board = 'board-name', file = 'file-name', }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}

TEST_CASE(ParseFrequencyParameter_HappyCase)
{
	Parameters params;

	ASSERT(ParseFrequencyParameter(params, "123456") == 123456);
	ASSERT(ParseFrequencyParameter(params, "123.456k") == 123456);
	ASSERT(ParseFrequencyParameter(params, "123.456789m") == 123456789);
	ASSERT(ParseFrequencyParameter(params, "1.234567890g") == 1234567890);
	ASSERT(ParseFrequencyParameter(params, "4294967295") == 4294967295);

	ASSERT(!params.invalid_parameters);
}

TEST_CASE(ParseFrequencyParameter_InvalidValue)
{
	const char* INVALID_VALUE_TESTS[] = {
		nullptr,
		"",
		"0",
		"abc",
		"k",
		"10p",
		"10e",
		"-1",
		"5g",
		"4294967296",
	};

	for (const char* test : INVALID_VALUE_TESTS)
	{
		Parameters params;

		ParseFrequencyParameter(params, test);

		ASSERT(params.invalid_parameters);
	}

}

