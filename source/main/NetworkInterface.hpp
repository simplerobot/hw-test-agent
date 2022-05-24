#pragma once

#include "Main.hpp"
#include <cstdint>
#include <string>


class NetworkInterface : public Interface
{
public:
	virtual bool IsValid() const = 0;

	virtual bool Bind(const std::string& service) = 0;
	virtual NetworkInterface* Accept() = 0;

	virtual bool Connect(const std::string& hostname, const std::string& service) = 0;

	virtual bool Read(void* buffer, size_t length) = 0;
	virtual bool Write(const void* buffer, size_t length) = 0;

	virtual void Close() = 0;
};
