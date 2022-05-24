#pragma once

#include "Main.hpp"
#include <vector>
#include <string>


class SystemExecuteInterface : public Interface
{
public:
	virtual bool Run(const std::string& application, const std::vector<std::string>& params) = 0;
	virtual int Read() = 0;
	virtual void Kill() = 0;
	virtual int Wait() = 0;
};
