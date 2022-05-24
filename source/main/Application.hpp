#pragma once


class Parameters;
class ConfigFile;


constexpr int EXITCODE_SUCCESS = 0;
constexpr int EXITCODE_TESTS_FAILED = 1;
constexpr int EXITCODE_TESTS_TIMEOUT = 2;
constexpr int EXITCODE_INVALID_PARAMETERS = 3;
constexpr int EXITCODE_INVALID_CONFIG = 4;
constexpr int EXITCODE_LOCK_FAILED = 5;
constexpr int EXITCODE_NOT_IMPLEMENTED = 6;
constexpr int EXITCODE_TOOL_FAILED = 7;
constexpr int EXITCODE_NETWORK_ERROR = 8;


class Application
{
public:
	Application();
	virtual ~Application();

	virtual int Run(int argc, char* const argv[]);

	virtual Parameters ParseArguments(int argc, char* const argv[]) const;
	virtual ConfigFile LoadConfig() const;

	virtual void PrintHelp() const;
	virtual void ShowConfig(const ConfigFile& config) const;

	virtual int RunServer(const Parameters& params, const ConfigFile& config) const;
	virtual int RunTest(const Parameters& params, const ConfigFile& config) const;

private:
	const char* m_app_name;
};
