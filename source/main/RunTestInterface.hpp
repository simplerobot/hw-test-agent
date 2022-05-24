#pragma once


class ConfigFile;
struct Parameters;


class RunTestInterface
{
public:
	virtual ~RunTestInterface() = default;

	virtual int Run(const Parameters& params, const ConfigFile& config) = 0;
};


