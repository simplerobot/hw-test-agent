#include "NetworkMock.hpp"

#include "Test.hpp"
#include <stdexcept>
#include <arpa/inet.h>


NetworkMock::NetworkMock()
{
}

NetworkMock::~NetworkMock()
{
	if (!m_expected.empty())
		std::printf("Missing Expected: '%s'\n", m_expected.front().c_str());
	if (!m_read_data.empty())
		std::printf("Additional read data %zd bytes.\n", m_read_data.size());
	if (!m_write_data.empty())
		std::printf("Additional write data %zd bytes.\n", m_write_data.size());

	ASSERT(m_expected.empty());
	ASSERT(m_read_data.empty());
	ASSERT(m_write_data.empty());
}

bool NetworkMock::IsValid() const
{
	ASSERT(false);
	throw std::runtime_error("Mock Not Implemented");
}


bool NetworkMock::Bind(const std::string& service)
{
	ASSERT(false);
	throw std::runtime_error("Mock Not Implemented");
}

NetworkInterface* NetworkMock::Accept()
{
	ASSERT(false);
	throw std::runtime_error("Mock Not Implemented");
}

bool NetworkMock::Connect(const std::string& hostname, const std::string& service)
{
	return Validate("CONNECT " + hostname + " " + service);
}

bool NetworkMock::Read(void* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		if (m_read_data.empty())
			return false;
		((uint8_t*)buffer)[i] = m_read_data.front();
		m_read_data.pop();
	}
	return true;
}

bool NetworkMock::Write(const void* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		if (m_write_data.empty())
			return false;
		uint8_t actual = ((const uint8_t*)buffer)[i];
		uint8_t expected = m_write_data.front();
		m_write_data.pop();
		if (actual != expected)
			std::printf("Expected write '%c' 0x%02x got '%c' 0x%02x.\n", expected, expected, actual, actual);
		ASSERT(actual == expected);
	}
	return true;
}

void NetworkMock::Close()
{
	Validate("CLOSE");
}

void NetworkMock::ExpectConnect(const std::string& hostname, const std::string& service, bool result)
{
	Expect("CONNECT " + hostname + " " + service, result);
}

void NetworkMock::AddExpectRead(const void* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
		m_read_data.push(((const uint8_t*)buffer)[i]);
}

void NetworkMock::AddExpectRead32(uint32_t data)
{
	uint32_t x = htonl(data);
	AddExpectRead(&x, sizeof(x));
}

void NetworkMock::AddExpectRead16(uint16_t data)
{
	uint16_t x = htons(data);
	AddExpectRead(&x, sizeof(x));
}

void NetworkMock::AddExpectRead8(uint8_t data)
{
	AddExpectRead(&data, sizeof(data));
}

void NetworkMock::AddExpectWrite(const void* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
		m_write_data.push(((const uint8_t*)buffer)[i]);
}

void NetworkMock::AddExpectWrite32(uint32_t data)
{
	uint32_t x = htonl(data);
	AddExpectWrite(&x, sizeof(x));
}

void NetworkMock::AddExpectWrite16(uint16_t data)
{
	uint16_t x = htons(data);
	AddExpectWrite(&x, sizeof(x));
}

void NetworkMock::AddExpectWrite8(uint8_t data)
{
	AddExpectWrite(&data, sizeof(data));
}

void NetworkMock::ExpectClose()
{
	Expect("CLOSE", false);
}

void NetworkMock::Expect(const std::string& expected, bool result)
{
	m_expected.push(expected);
	m_results.push(result);
}

bool NetworkMock::Validate(const std::string& actual)
{
	if (m_expected.empty())
		printf("Unexpected call: '%s'\n", actual.c_str());
	ASSERT(!m_expected.empty());

	std::string expected = m_expected.front();
	m_expected.pop();

	int result = m_results.front();
	m_results.pop();

	if (expected != actual)
	{
		printf("Expected call '%s'\n", expected.c_str());
		printf("Actual call   '%s'\n", actual.c_str());
	}
	ASSERT(expected == actual);

	return result;
}

