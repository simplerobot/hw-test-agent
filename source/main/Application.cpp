#include "Application.hpp"
#include "Main.hpp"
#include "Config.hpp"
#include "Params.hpp"
#include "RunTest.hpp"
#include "RunServer.hpp"
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/file.h>


Application::Application()
	: m_app_name("sr-hw-test-agent")
{
}

Application::~Application()
{
}

int Application::Run(int argc, char* const argv[])
{
	if (argc > 0)
		m_app_name = argv[0];

	Parameters params = ParseArguments(argc, argv);

	if (params.invalid_parameters)
	{
		PrintHelp();
		return EXITCODE_INVALID_PARAMETERS;
	}

	if (params.show_help)
	{
		PrintHelp();
	}

	ConfigFile config = LoadConfig();

	if (params.show_config)
	{
		ShowConfig(config);
	}

	if (params.run_server)
	{
		return RunServer(params, config);
	}

	if (params.run_test)
	{
		return RunTest(params, config);
	}

	return EXITCODE_SUCCESS;

}

Parameters Application::ParseArguments(int argc, char* const argv[]) const
{
	return ParseCommandLineArguments(argc, argv);
}

ConfigFile Application::LoadConfig() const
{
	std::ifstream input(CONFIG_FILE);
	return ConfigFile(input);
}

void Application::PrintHelp() const
{
	::PrintHelp(m_app_name);
}

void Application::ShowConfig(const ConfigFile& config) const
{
	std::printf("Config from file '%s'\n", CONFIG_FILE);
	if (config.Count() == 0)
	{
		std::printf("No configuration found. Example:\n");
		std::printf("  [RLM3]\n");
		std::printf("  type=stlink\n");
		std::printf("  id=51ff78066778505447321267\n");
		std::printf("  flash-base=0x08000000\n");
		std::printf("  [RLM2]\n");
		std::printf("  type=network\n");
		std::printf("  host=127.0.0.1\n");
		std::printf("  port=5151\n");
	}
	else
	{
		for (size_t i = 0; i < config.Count(); i++)
		{
			ConfigSection c = config.Get(i);
			if (c.Has("type"))
			{
				std::string type = c.Get("type");
				if (type == "stlink")
					std::printf("Board: '%s' type: %s id: '%s'\n", c.Name().c_str(), type.c_str(), c.Get("id", "").c_str());
				else if (type == "network")
					std::printf("Board: '%s' type: %s host: '%s' port: '%s'\n", c.Name().c_str(), type.c_str(), c.Get("host", "").c_str(), c.Get("port", "").c_str());
				else
					std::printf("Board: '%s' type: unknown\n", c.Name().c_str());
			}
			else
				std::printf("Skipping unknown section '%s'\n", c.Name().c_str());
		}
	}
}

int Application::RunServer(const Parameters& params, const ConfigFile& config) const
{
	return ::RunServer(params, config);
}

int Application::RunTest(const Parameters& params, const ConfigFile& config) const
{
	::RunTest test_runner;
	return test_runner.Run(params, config);
}
