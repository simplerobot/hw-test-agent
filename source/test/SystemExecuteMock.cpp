#include "SystemExecuteMock.hpp"

#include "Test.hpp"
#include <sstream>
#include <time.h>
#include <mutex>
#include <thread>


static std::mutex g_lock;


SystemExecuteMock::SystemExecuteMock()
{
}

SystemExecuteMock::~SystemExecuteMock()
{
	if (!m_expected.empty())
		printf("Missing Expected: '%s'\n", m_expected.front().c_str());
	ASSERT(m_expected.empty());
}

bool SystemExecuteMock::Run(const std::string& application, const std::vector<std::string>& params)
{
	std::ostringstream out;
	out << "RUN " << application;
	for (auto& s : params)
		out << " " << s;
	return (Validate(out.str()) != 0);
}

int SystemExecuteMock::Read()
{
	return Validate("READ");
}

void SystemExecuteMock::Kill()
{
	Validate("KILL");
}

int SystemExecuteMock::Wait()
{
	return Validate("WAIT");
}

void SystemExecuteMock::ExpectRun(const std::string& command_line, bool result, size_t wait_ms)
{
	Expect("RUN " + command_line, result, wait_ms);
}

void SystemExecuteMock::ExpectRead(int result, size_t wait_ms)
{
	Expect("READ", result, wait_ms);
}

void SystemExecuteMock::ExpectReads(const std::string& text)
{
	for (char c : text)
		ExpectRead((int)(unsigned int)c);
}

void SystemExecuteMock::ExpectKill(size_t wait_ms)
{
	Expect("KILL", 0, wait_ms);
}

void SystemExecuteMock::ExpectWait(int result, size_t wait_ms)
{
	Expect("WAIT", result, wait_ms);
}

void SystemExecuteMock::Expect(const std::string& expected, int result, size_t wait_ms)
{
	m_expected.push(expected);
	m_results.push(result);
	m_wait_time_ms.push(wait_ms);
}

int SystemExecuteMock::Validate(const std::string& actual)
{
	if (m_expected.empty())
		printf("Unexpected call: '%s'\n", actual.c_str());
	ASSERT(!m_expected.empty());

	g_lock.lock();

	std::string expected = m_expected.front();
	m_expected.pop();

	int result = m_results.front();
	m_results.pop();

	size_t wait_time_ms = m_wait_time_ms.front();
	m_wait_time_ms.pop();

	g_lock.unlock();

	if (expected != actual)
	{
		printf("Expected call '%s'\n", expected.c_str());
		printf("Actual call   '%s'\n", actual.c_str());
	}
	ASSERT(expected == actual);

	if (wait_time_ms != 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));

	return result;
}

