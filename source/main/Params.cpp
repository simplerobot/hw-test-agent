#include "Params.hpp"
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include "Main.hpp"


extern void PrintHelp(const char* app_name)
{
	std::printf("%s --help\n", app_name);
	std::printf("%s --run --board BOARD --file FILE\n", app_name);
	std::printf("%s --server --port PORT\n", app_name);
	std::printf("%s --show-config\n", app_name);
	std::printf("  -h, --help               Print this help\n");
	std::printf("  -r, --run                Runs test firmware on board\n");
	std::printf("  -bXX, --board=XX         Specify the target board for tests\n");
	std::printf("  -fXX, --file=XX          Specify the firmware .bin file to test\n");
	std::printf("  -u, --unlocked           Don't ensure only one agent runs at a time\n");
	std::printf("  -lXX, --lock-timeout=XX  Seconds to wait for exclusive agent access\n");
	std::printf("  -tXX, --test-timeout=XX  Seconds to wait for test to complete\n");
	std::printf("  -s, --server             Don't ensure only one agent runs at a time\n");
	std::printf("  -pXX, --port=XX          Port for the server to listen on\n");
	std::printf("  -c, --show-config        Show board configuration\n");
}

static void ParameterError(Parameters& params, const char* error_message)
{
	std::puts(error_message);
	params.invalid_parameters = true;
}

static void CheckParameterError(Parameters& params, bool condition, const char* error_message)
{
	if (!condition)
		ParameterError(params, error_message);
}

extern Parameters ParseCommandLineArguments(int argc, char* const argv[])
{
	static struct option long_options[] = {
		{ "help",         no_argument,       nullptr, 'h' },
		{ "run",          no_argument,       nullptr, 'r' },
		{ "board",        required_argument, nullptr, 'b' },
		{ "file",         required_argument, nullptr, 'f' },
		{ "lock-timeout", required_argument, nullptr, 'l' },
		{ "test-timeout", required_argument, nullptr, 't' },
		{ "server",       no_argument,       nullptr, 's' },
		{ "port",         required_argument, nullptr, 'p' },
		{ "show-config",  no_argument,       nullptr, 'c' },
		{ nullptr, 0, nullptr, 0 }
	};

	Parameters params = {};

	int option_index = 0;
	int c;
	while ((c = ::getopt_long(argc, argv, "hrb:f:l:t:sp:c", long_options, &option_index)) != -1)
	{
		switch (c)
		{
		case 'h': // help
			CheckParameterError(params, !params.show_help, "Error: Duplicate parameter.");
			params.show_help = true;
			break;
		case 'r': // run
			CheckParameterError(params, !params.run_test, "Error: Duplicate parameter.");
			params.run_test = true;
			break;
		case 'b': // board
			CheckParameterError(params, params.board == nullptr, "Error: Duplicate parameter.");
			params.board = optarg;
			break;
		case 'f': // file
			CheckParameterError(params, params.file == nullptr, "Error: Duplicate parameter.");
			params.file = optarg;
			break;
		case 'l': // lock-timeout
			CheckParameterError(params, params.lock_timeout_ms == 0, "Error: Duplicate parameter.");
			params.lock_timeout_ms = std::atoi(optarg) * 1000;
			CheckParameterError(params, params.lock_timeout_ms != 0, "Error: Invalid lock timeout.");
			break;
		case 't': // test-timeout
			CheckParameterError(params, params.test_timeout_ms == 0, "Error: Duplicate parameter.");
			params.test_timeout_ms = std::atoi(optarg) * 1000;
			CheckParameterError(params, params.test_timeout_ms != 0, "Error: Invalid test timeout.");
			break;
		case 's': // server
			CheckParameterError(params, !params.run_server, "Error: Duplicate parameter.");
			params.run_server = true;
			break;
		case 'p': // port
			CheckParameterError(params, params.service == nullptr, "Error: Duplicate parameter.");
			params.service = optarg;
			break;
		case 'c': // show-config
			CheckParameterError(params, !params.show_config, "Error: Duplicate parameter.");
			params.show_config = true;
			break;
		case '?':
		default:
			ParameterError(params, "Error: Unknown parameter.");
			break;
		}
	}

	for (int c = optind; c < argc; c++)
		printf("Error: Unknown parameter '%s'\n", argv[c]);
	CheckParameterError(params, optind == argc, "Error: Unknown extra parameters.");

	if (params.run_test && params.board == nullptr)
		ParameterError(params, "Error: No board provided for tests.");
	if (params.run_test && params.file == nullptr)
		ParameterError(params, "Error: No file provided for tests.");
	if (!params.run_test && params.board != nullptr)
		ParameterError(params, "Error: Only provide a board when running tests.");
	if (!params.run_test && params.file != nullptr)
		ParameterError(params, "Error: Only provide a file when running tests.");
	if (!params.run_test && params.test_timeout_ms != 0)
		ParameterError(params, "Error: Only provide a test timeout when running tests.");
	if (!params.run_test && !params.run_server && params.lock_timeout_ms != 0)
		ParameterError(params, "Error: Only provide a lock timeout when running tests or server.");
	if (!params.run_server && params.service != nullptr)
		ParameterError(params, "Error: Only provide service name when running server.");
	if (params.run_test && params.run_server)
		ParameterError(params, "Error: Cannot run tests and server together.");

	if (!params.run_test && !params.run_server && !params.show_config && !params.show_help)
		params.show_help = true;

	if (params.lock_timeout_ms == 0)
		params.lock_timeout_ms = DEFAULT_LOCK_TIMEOUT_MS;
	if (params.test_timeout_ms == 0)
		params.test_timeout_ms = DEFAULT_TEST_TIMEOUT_MS;

	return params;
}

extern std::ostream& operator<<(std::ostream& out, const Parameters& params)
{
	Parameters init = {};
#define PRINT_PARAM_BOOL(X) if (params.X != init.X) out << " " #X " = " << (params.X ? "true" : "false") << ",";
#define PRINT_PARAM_STR(X) if (params.X != init.X) { if (params.X == nullptr) out << " " #X " = nullptr,"; else out << " " #X " = '" << params.X << "',"; }
#define PRINT_PARAM_INT(X) if (params.X != init.X) out << " " #X " = " << params.X << ",";

	out << "Parameters {";
	PRINT_PARAM_BOOL(invalid_parameters);
	PRINT_PARAM_BOOL(run_test);
	PRINT_PARAM_BOOL(run_server);
	PRINT_PARAM_BOOL(show_config);
	PRINT_PARAM_BOOL(show_help);
	PRINT_PARAM_INT(lock_timeout_ms);
	PRINT_PARAM_INT(test_timeout_ms);
	PRINT_PARAM_STR(service);
	PRINT_PARAM_STR(board);
	PRINT_PARAM_STR(file);
	out << " }";

	return out;
}

extern std::string ToString(const Parameters& params)
{
	std::ostringstream out;
	out << params;
	return out.str();
}

