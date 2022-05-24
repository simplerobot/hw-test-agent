#pragma once

#include "NetworkInterface.hpp"
#include <cstdint>


class Protocol
{
public:
	Protocol(NetworkInterface& network);

	operator bool() const;
	void Error(const char* message, ...) __attribute__ ((format (printf, 2, 3)));;

	void Write(std::istream&);
	void Write(const std::string& s);
	void Write(uint32_t x);
	void Write(uint16_t x);
	void Write(uint8_t x);
	void Write(const void* data, size_t length);

	void Read(std::ostream&);
	void Read(std::string& s);
	void Read(uint32_t& x);
	void Read(uint16_t& x);
	void Read(uint8_t& x);
	void Read(void* data, size_t length);

	void SetMaxStringLength(size_t max_string_length);
	void SetMaxBinaryLength(size_t max_binary_length);

private:
	NetworkInterface& m_network;
	bool m_error;
	size_t m_max_string_length;
	size_t m_max_binary_length;
};
