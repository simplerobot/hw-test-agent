#include "RunTest.hpp"
#include "Application.hpp"
#include "Config.hpp"
#include "Params.hpp"
#include "RunTestStlink.hpp"
#include "RunTestNetwork.hpp"
#include <fcntl.h>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/file.h>


int RunTest::Run(const Parameters& params, const ConfigFile& config)
{
	std::string sanitized_board_name = params.board;
	for (char& c : sanitized_board_name)
		if (!::isalnum(c))
			c = '_';

	auto configs = config.Find(sanitized_board_name);
	if (configs.empty())
	{
		std::printf("ERROR: board '%s' is not configured.\n", sanitized_board_name.c_str());
		return EXITCODE_INVALID_CONFIG;
	}
	if (configs.size() > 1)
	{
		// We should allow this for network connections to load balance.
		std::printf("ERROR: board '%s' has multiple configurations.\n", sanitized_board_name.c_str());
		return EXITCODE_INVALID_CONFIG;
	}
	auto& board_config = configs.front();

	// Open/create a lock file for this board.
	std::string lock_filename = "/tmp/hwta." + sanitized_board_name + ".lock";
	int lock_fd = ::open(lock_filename.c_str(), O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (lock_fd < 0)
	{
		std::printf("ERROR: Unable to open lock file '%s'.\n", lock_filename.c_str());
		return EXITCODE_LOCK_FAILED;
	}

	// Lock this file so another instance cannot use this board at the same time.
	auto start_time = std::chrono::steady_clock::now();
	while (-1 == ::flock(lock_fd, LOCK_EX | LOCK_NB))
	{
		auto current_time = std::chrono::steady_clock::now();
		size_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
		if (elapsed_ms >= params.lock_timeout_ms)
		{
			std::printf("ERROR: Unable to get exclusive lock before timeout.\n");
			return EXITCODE_LOCK_FAILED;
		}
		// Wait up to one second to try again.
		size_t sleep_time_ms = std::min<ssize_t>(1000, params.lock_timeout_ms - elapsed_ms);
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
	}

	int result;

	std::string type = board_config.Get("type", "");
	if (type == "stlink")
	{
		result = RunTestStlink(params, board_config);
	}
	else if (type == "network")
	{
		result = RunTestNetwork(params, board_config);
	}
	else
	{
		std::printf("ERROR: board '%s' uses unknown type '%s'\n", params.board, type.c_str());
		result = EXITCODE_INVALID_CONFIG;
	}

	::close(lock_fd);
	return result;
}


