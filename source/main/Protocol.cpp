#include "Protocol.hpp"
#include <arpa/inet.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>


const size_t DEFAULT_MAX_STRING_LENGTH = 256;
const size_t DEFAULT_MAX_BINARY_LENGTH = 1 * 1024 * 1024;


Protocol::Protocol(NetworkInterface& network)
	: m_network(network)
	, m_error(false)
	, m_max_string_length(DEFAULT_MAX_STRING_LENGTH)
	, m_max_binary_length(DEFAULT_MAX_BINARY_LENGTH)
{
}

Protocol::operator bool() const
{
	return !m_error;
}

void Protocol::Error(const char* message, ...)
{
	if (m_error)
		return;
	m_error = true;
	va_list args;
	va_start(args, message);
	std::vprintf(message, args);
	va_end(args);
}

void Protocol::Write(std::istream& in)
{
	char buffer[255];
	size_t total_size = 0;
	size_t current_count;
	do
	{
		in.read(buffer, 255);
		current_count = in.gcount();
		total_size += current_count;
		if (total_size > m_max_binary_length)
			Error("Error: Binary object length exceeds maximum %zd.\n", m_max_binary_length);
		Write((uint8_t)current_count);
		Write(buffer, current_count);
	} while (current_count == 255);
}

void Protocol::Write(const std::string& s)
{
	if (s.size() > m_max_string_length)
		Error("Error: String length %zd exceeds maximum %zd.\n", s.size(), m_max_string_length);
	for (char c : s)
	{
		if (c == 0)
			Error("Error: String contains a nul character.\n");
		Write((uint8_t)c);
	}
	Write((uint8_t)0);
}

void Protocol::Write(uint32_t x)
{
	uint32_t v = ::htonl(x);
	Write(&v, sizeof(v));
}

void Protocol::Write(uint16_t x)
{
	uint16_t v = ::htons(x);
	Write(&v, sizeof(v));
}

void Protocol::Write(uint8_t x)
{
	Write(&x, sizeof(x));
}

void Protocol::Write(const void* data, size_t length)
{
	if (!m_error)
		if (!m_network.Write(data, length))
			Error("Error: Network send failed.\n");
}

void Protocol::Read(std::ostream& out)
{
	char buffer[255];
	size_t total_size = 0;
	uint8_t current_count;
	do
	{
		Read(current_count);
		total_size += current_count;
		if (total_size <= m_max_binary_length)
		{
			Read(buffer, current_count);
			out.write(buffer, current_count);
		}
		else
			Error("Error: Binary object length exceeds maximum %zd.\n", m_max_binary_length);
	} while (current_count == 255);
	out.flush();
}

void Protocol::Read(std::string& s)
{
	std::ostringstream buffer;
	uint8_t c = 0;
	Read(c);
	for (size_t length = 0; c != 0 && length < m_max_string_length; length++)
	{
		buffer.put((char)c);
		c = 0;
		Read(c);
	}
	if (c != 0)
		Error("Error: String length exceeds protocol maximum %zd.\n", m_max_string_length);
	s = buffer.str();
}

void Protocol::Read(uint32_t& x)
{
	uint32_t v = ::htonl(x);
	Read(&v, sizeof(v));
	x = ::ntohl(v);
}

void Protocol::Read(uint16_t& x)
{
	uint16_t v = ::htons(x);
	Read(&v, sizeof(v));
	x = ::htons(v);
}

void Protocol::Read(uint8_t& x)
{
	Read(&x, sizeof(x));
}

void Protocol::Read(void* data, size_t length)
{
	if (!m_error)
		if (!m_network.Read(data, length))
			Error("Network read error\n");
}

void Protocol::SetMaxStringLength(size_t max_string_length)
{
	m_max_string_length = max_string_length;
}

void Protocol::SetMaxBinaryLength(size_t max_binary_length)
{
	m_max_binary_length = max_binary_length;
}

