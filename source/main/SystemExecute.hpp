#pragma once

#include "SystemExecuteInterface.hpp"


class SystemExecute : public SystemExecuteInterface
{
public:
	SystemExecute();
	~SystemExecute();

	virtual bool Run(const std::string& application, const std::vector<std::string>& params) override;
	virtual int Read() override;
	virtual void Kill() override;
	virtual int Wait() override;

private:
	int m_input_pipe;
	int m_child_pid;
};
