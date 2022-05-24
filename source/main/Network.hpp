#pragma once

#include "NetworkInterface.hpp"


class Network : public NetworkInterface
{
public:
	Network();
	~Network();

	virtual bool IsValid() const override;

	virtual bool Bind(const std::string& service) override;
	virtual NetworkInterface* Accept() override;

	virtual bool Connect(const std::string& hostname, const std::string& service) override;

	virtual bool Read(void* buffer, size_t length) override;
	virtual bool Write(const void* buffer, size_t length) override;

	virtual void Close() override;

private:
	int m_socket;
};
