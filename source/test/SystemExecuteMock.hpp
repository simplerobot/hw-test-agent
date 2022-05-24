#pragma once

#include <queue>
#include "SystemExecuteInterface.hpp"


class SystemExecuteMock : public SystemExecuteInterface
{
public:
	SystemExecuteMock();
	~SystemExecuteMock();

	virtual bool Run(const std::string& application, const std::vector<std::string>& params) override;
	virtual int Read() override;
	virtual void Kill() override;
	virtual int Wait() override;

	void ExpectRun(const std::string& command_line, bool result, size_t wait_ms = 0);
	void ExpectRead(int result, size_t wait_ms = 0);
	void ExpectReads(const std::string& text);
	void ExpectKill(size_t wait_ms = 0);
	void ExpectWait(int result, size_t wait_ms = 0);

private:
	void Expect(const std::string& expected, int result, size_t wait_ms);
	int Validate(const std::string&);

	std::queue<std::string> m_expected;
	std::queue<int> m_results;
	std::queue<size_t> m_wait_time_ms;
};
