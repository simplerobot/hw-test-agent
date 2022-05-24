#include "Test.hpp"
#include "RunTestMock.hpp"
#include "Params.hpp"
#include "Config.hpp"
#include <cstring>


RunTestMock::RunTestMock()
	: m_expecting_mock_call(false)
	, m_result(0)
{
}

RunTestMock::RunTestMock(const Parameters& expected_params, const ConfigFile& expected_config, const std::string& print, int result)
	: m_expecting_mock_call(true)
	, m_expected_params(ToString(expected_params))
	, m_expected_config(ToString(expected_config))
	, m_print(print)
	, m_result(result)
{
}

RunTestMock::~RunTestMock()
{
	if (m_expecting_mock_call)
		std::printf("ERROR: Mock call was never made.\n");
	ASSERT(!m_expecting_mock_call);
}

int RunTestMock::Run(const Parameters& params, const ConfigFile& config)
{
	if (!m_expecting_mock_call)
		std::fprintf(stderr, "ERROR: No mock call was expected.\n");
	ASSERT(m_expecting_mock_call);
	m_expecting_mock_call = false;

	// Remove random suffix from temp files so our tests will match as expected.
	Parameters temp_params = params;
	if (std::strncmp("/tmp/hwta.fw.", temp_params.file, 13) == 0)
		temp_params.file = "/tmp/hwta.fw.XXXXXX";

	std::string actual_params = ToString(temp_params);
	std::string actual_config = ToString(config);

	if (actual_params != m_expected_params)
	{
		std::fprintf(stderr, "Parameters do not match expected.\n");
		std::fprintf(stderr, "Actual: %s\n", actual_params.c_str());
		std::fprintf(stderr, "Expect: %s\n", m_expected_params.c_str());
	}
	if (actual_config != m_expected_config)
	{
		std::fprintf(stderr, "Config does not match expected.\n");
		std::fprintf(stderr, "Actual: %s\n", actual_config.c_str());
		std::fprintf(stderr, "Expect: %s\n", m_expected_config.c_str());
	}
	ASSERT(actual_params == m_expected_params);
	ASSERT(actual_config == m_expected_config);

	std::printf("%s", m_print.c_str());

	return m_result;
}
