#include "RunTestStlink.hpp"
#include "Application.hpp"
#include "Config.hpp"
#include "Params.hpp"
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>
#include "SystemExecute.hpp"


enum TestState
{
	TESTSTATE_INITIAL,
	TESTSTATE_FAIL,
	TESTSTATE_PASS,

	TESTSTATE_MID_LINE,
	TESTSTATE_TEXT_E,
	TESTSTATE_TEXT_EO,
	TESTSTATE_TEXT_EOT,
	TESTSTATE_TEXT_EOT_,
};


extern int RunTestStlink(const Parameters& params, const ConfigSection& config)
{
	SystemExecute exec;
	return RunTestStlink(params, config, exec);
}

extern int RunTestStlink(const Parameters& params, const ConfigSection& config, SystemExecuteInterface& exec)
{
	// Verify we have the parameters needed for our Stlink method.

	std::string id = config.Get("id", "");
	if (id.empty())
	{
		std::printf("ERROR: board '%s' configuration missing id.\n", params.board);
		return EXITCODE_INVALID_CONFIG;
	}

	std::string flash_base = config.Get("flash-base", "");
	if (flash_base.empty())
	{
		std::printf("ERROR: board '%s' configuration missing flash-base.\n", params.board);
		return EXITCODE_INVALID_CONFIG;
	}

	// Download firmware to the board. (Only show the output if we fail.)

	if (!exec.Run("/usr/local/bin/st-flash", { "--reset", "--serial", id, "write", params.file, flash_base }))
	{
		std::printf("ERROR: Unable to create process st-flash (%d)\n", errno);
		return EXITCODE_TOOL_FAILED;
	}

	std::ostringstream output_buffer;
	int c;
	while ((c = exec.Read()) >= 0)
		output_buffer.put(c);

	int exec_result = exec.Wait();
	if (exec_result != 0)
	{
		std::printf("%s", output_buffer.str().c_str());
		std::printf("ERROR: Failed writing test firmware to board '%s' error %d.\n", params.board, exec_result);
		return EXITCODE_TOOL_FAILED;
	}

	// Read data from the SWO output.  Echo output to the stdout and watch for a line starting with "EOT P[ASS]" or "EOT F[AIL]".

	std::vector<std::string> trace_args;
	trace_args.push_back("--serial");
	trace_args.push_back(id);
	if (params.system_frequency_hz != 0)
		trace_args.push_back("--clock=" + std::to_string(params.system_frequency_hz / 1000000));
	if (params.trace_frequency_hz != 0)
		trace_args.push_back("--trace=" + std::to_string(params.trace_frequency_hz));

	if (!exec.Run("/usr/local/bin/st-trace", trace_args))
	{
		std::printf("ERROR: Unable to create process st-trace (%d)\n", errno);
		return EXITCODE_TOOL_FAILED;
	}

	// TODO: Consider moving this timeout behavior into SystemExec.
	std::timed_mutex cleanup_lock;
	cleanup_lock.lock();
	bool watchdog_timeout = false;
	std::thread cleanup_thread([&]() {
		// Kill the application after we finished reading from it, or after our timeout.
		watchdog_timeout = !cleanup_lock.try_lock_for(std::chrono::milliseconds(params.test_timeout_ms));
		exec.Kill();
	});

	TestState state = TESTSTATE_INITIAL;
	while (state != TESTSTATE_FAIL && state != TESTSTATE_PASS)
	{
		int c = exec.Read();
		if (c != EOF)
			std::putchar(c);
		if (c == '\n')
			std::fflush(stdout);

		if (c == EOF)
			state = TESTSTATE_FAIL;
		else if (c == '\n')
			state = TESTSTATE_INITIAL;
		else if (state == TESTSTATE_INITIAL && c == 'E')
			state = TESTSTATE_TEXT_E;
		else if (state == TESTSTATE_TEXT_E && c == 'O')
			state = TESTSTATE_TEXT_EO;
		else if (state == TESTSTATE_TEXT_EO && c == 'T')
			state = TESTSTATE_TEXT_EOT;
		else if (state == TESTSTATE_TEXT_EOT && c == ' ')
			state = TESTSTATE_TEXT_EOT_;
		else if (state == TESTSTATE_TEXT_EOT_ && c == 'P')
			state = TESTSTATE_PASS;
		else if (state == TESTSTATE_TEXT_EOT_)
			state = TESTSTATE_FAIL;
		else
			state = TESTSTATE_MID_LINE;
	}
	std::printf("\n");

	cleanup_lock.unlock();
	cleanup_thread.join();
	exec.Wait();

	if (watchdog_timeout)
	{
		std::printf("WARNING: Test timeout running test firmware on board '%s'\n", params.board);
		return EXITCODE_TESTS_TIMEOUT;
	}

	if (state != TESTSTATE_PASS)
	{
		std::printf("WARNING: Tests failed on board '%s'\n", params.board);
		return EXITCODE_TESTS_FAILED;
	}

	std::printf("Tests passed on board '%s'\n", params.board);
	return EXITCODE_SUCCESS;
}

