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
	ASSERT(TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(TestCommandLine({ "app", "--server", "--port", "12345" }));
	ASSERT(TestCommandLine({ "app", "--show-config" }));

	ASSERT(TestCommandLine({ "app", "--help" }));
	ASSERT(TestCommandLine({ "app", "--run", "--board=boardname", "--file=filename", "--lock-timeout=120", "--test-timeout=120" }));
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
}

TEST_CASE(ParseCommandLineArguments_Duplicate)
{
	ASSERT(!TestCommandLine({ "app", "--help", "--help" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--lock-timeout", "120", "--test-timeout", "120" }));
	ASSERT(!TestCommandLine({ "app", "--run", "--board", "boardname", "--file", "filename", "--lock-timeout", "120", "--test-timeout", "120", "--test-timeout", "120" }));
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
	test.service = "service-name";
	test.board = "board-name";
	test.file = "file-name";
	std::string expected = "Parameters { invalid_parameters = true, run_test = true, run_server = true, show_config = true, show_help = true, lock_timeout_ms = 1234, test_timeout_ms = 5678, service = 'service-name', board = 'board-name', file = 'file-name', }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}