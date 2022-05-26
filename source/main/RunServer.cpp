#include "RunServer.hpp"
#include "Application.hpp"
#include "Network.hpp"
#include "Params.hpp"
#include "Config.hpp"
#include "Protocol.hpp"
#include "RunTestInterface.hpp"
#include "RunTest.hpp"
#include "HWTA.hpp"
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>


static bool RunServerTestFirmware(const Parameters& params, const ConfigFile& config, Protocol& connection, RunTestInterface& test_runner);


extern int RunServer(const Parameters& params, const ConfigFile& config)
{
	// Verify we have the configuration needed.
	if (params.service == nullptr)
	{
		std::printf("ERROR: Unable to run as a service without specifying a service (port) to bind to.\n");
		return EXITCODE_INVALID_PARAMETERS;
	}

	Network server;
	if (!server.Bind(params.service))
	{
		std::printf("ERROR: Unable to bind to service (port) '%s'.\n", params.service);
		return EXITCODE_NETWORK_ERROR;
	}

	while (server.IsValid())
	{
		NetworkInterface* connection = server.Accept();
		if (connection != nullptr)
		{
			// Each connection will be handled in a forked process.  We do this since we need to redirect all errors
			// back through the network.
			if (::fork() == 0)
			{
				// This is the child process that will handle the request.
				server.Close();
				RunTest test_runner;
				int result = RunServer(params, config, *connection, test_runner);
				delete connection;
				exit(result);
			}

			// This is the parent process.  Clean up the child's resources.
			delete connection;
		}

		// Release any zombie children we have accumulated.
		int status;
		int pid = ::waitpid(-1, &status, WNOHANG);
		while (pid != 0 && pid != -1)
			pid = ::waitpid(-1, &status, WNOHANG);
	}

	return EXITCODE_SUCCESS;
}

extern int RunServer(const Parameters& params, const ConfigFile& config, NetworkInterface& network, RunTestInterface& test_runner)
{
	Protocol connection(network);

	connection.Write(HWTA_SIGNATURE);
	connection.Write(HWTA_VERSION_CURRENT);

	uint32_t remote_signature = 0;
	connection.Read(remote_signature);
	if (remote_signature != HWTA_SIGNATURE)
	{
		std::printf("ERROR: Client does not appear to be running HWTA protocol.\n");
		return EXITCODE_NETWORK_ERROR;
	}

	uint32_t remote_version = 0;
	connection.Read(remote_version);
	if (remote_version < HWTA_VERSION_MIN_SUPPORTED)
	{
		std::printf("ERROR: Client is running version %x, but we only support down to %x.\n", remote_version, HWTA_VERSION_MIN_SUPPORTED);
		return EXITCODE_NETWORK_ERROR;
	}

	while (true)
	{
		uint8_t command = HWTA_COMMAND_DONE;
		connection.Read(command);

		switch (command)
		{
		case HWTA_COMMAND_DONE:
			return EXITCODE_SUCCESS;
		case HWTA_COMMAND_TEST_FIRMWARE:
			if (!RunServerTestFirmware(params, config, connection, test_runner))
				return EXITCODE_NETWORK_ERROR;
			break;
		default:
			std::printf("ERROR: Client sent unknown command %d.\n", command);
			return EXITCODE_NETWORK_ERROR;
		}
	}
}

static bool RunServerTestFirmware(const Parameters& parent_params, const ConfigFile& config, Protocol& connection, RunTestInterface& test_runner)
{
	Parameters child_params(parent_params);

	std::string board;
	connection.Read(board);
	child_params.board = board.c_str();

	connection.Read(child_params.lock_timeout_ms);
	connection.Read(child_params.test_timeout_ms);
	connection.Read(child_params.system_frequency_hz);
	connection.Read(child_params.trace_frequency_hz);

	std::string temp_filename("/tmp/hwta.fw.XXXXXX");
	int temp_file_fd = ::mkstemp(&temp_filename[0]);
	child_params.file = temp_filename.c_str();

	std::ofstream firmware_out(temp_filename);
	connection.Read(firmware_out);
	::close(temp_file_fd);

	if (!connection)
	{
		::unlink(temp_filename.c_str());
		std::printf("ERROR: Client dropped connection while reading test firmware command.\n");
		return false;
	}

	connection.Write(HWTA_RESPONSE_OK);

	// Redirect output to go across the network.
	std::fflush(stdout);
	std::fflush(stderr);
	int saved_output_fd = ::dup(1);
	int saved_error_fd = ::dup(2);
	int pipe_fds[2];
	if (::pipe(pipe_fds) != 0)
	{
		::unlink(temp_filename.c_str());
		std::printf("ERROR: Unable to create pipe to capture test output.\n");
		return false;
	}
	int pipe_read_fd = pipe_fds[0];
	int pipe_write_fd = pipe_fds[1];
	::dup2(pipe_write_fd, 1);
	::dup2(pipe_write_fd, 2);
	::close(pipe_write_fd);
	std::thread redirect_thread([&] {
		char buffer[1024];
		ssize_t size = ::read(pipe_read_fd, buffer, sizeof(buffer));
		while (size > 0)
		{
			for (ssize_t i = 0; i < size; i++)
				if (buffer[i] == 0)
					buffer[i] = ' ';
			connection.Write(buffer, size);
			size = ::read(pipe_read_fd, buffer, sizeof(buffer));
		}
		::close(pipe_read_fd);
	});

	uint8_t result_code = HWTA_RESPONSE_ERROR;
	try
	{
		int result = test_runner.Run(child_params, config);
		switch (result)
		{
		case EXITCODE_SUCCESS:
			result_code = HWTA_RESPONSE_TESTS_PASS;
			break;
		case EXITCODE_TESTS_FAILED:
			result_code = HWTA_RESPONSE_TESTS_FAIL;
			break;
		case EXITCODE_LOCK_FAILED:
		case EXITCODE_TESTS_TIMEOUT:
			result_code = HWTA_RESPONSE_TESTS_TIMEOUT;
			break;
		}
	}
	catch (...)
	{
		// This was added for tests.  When a test asserts false, it throws and exception that results in the above thread not being joined.
	}

	// Stop redirecting output to the network.
	std::fflush(stdout);
	std::fflush(stderr);
	::dup2(saved_output_fd, 1);
	::dup2(saved_error_fd, 2);
	redirect_thread.join();
	::close(saved_output_fd);
	::close(saved_error_fd);

	::unlink(temp_filename.c_str());

	connection.Write((uint8_t)0);
	connection.Write(result_code);

	return true;
}
