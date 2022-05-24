#pragma once

#include "RunTestInterface.hpp"


class RunTest : public RunTestInterface
{
public:
	virtual int Run(const Parameters& params, const ConfigFile& config) override;
};


