#pragma once

#include "RunTestInterface.hpp"
#include <string>


class RunTestMock : public RunTestInterface
{
public:
	RunTestMock();
	RunTestMock(const Parameters& expected_params, const ConfigFile& expected_config, const std::string& print, int result);
	~RunTestMock();

	virtual int Run(const Parameters& params, const ConfigFile& config) override;

private:
	bool m_expecting_mock_call;

	std::string m_expected_params;
	std::string m_expected_config;
	std::string m_print;
	int m_result;
};


