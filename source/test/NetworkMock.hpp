#pragma once

#include "NetworkInterface.hpp"
#include <queue>


class NetworkMock : public NetworkInterface
{
public:
	NetworkMock();
	virtual ~NetworkMock() override;

	virtual bool IsValid() const override;

	virtual bool Bind(const std::string& service) override;
	virtual NetworkInterface* Accept() override;

	virtual bool Connect(const std::string& hostname, const std::string& service) override;

	virtual bool Read(void* buffer, size_t length) override;
	virtual bool Write(const void* buffer, size_t length) override;

	virtual void Close() override;

	void ExpectConnect(const std::string& hostname, const std::string& service, bool result);

	void AddExpectRead(const void* buffer, size_t length);
	void AddExpectRead32(uint32_t data);
	void AddExpectRead16(uint16_t data);
	void AddExpectRead8(uint8_t data);

	void AddExpectWrite(const void* buffer, size_t length);
	void AddExpectWrite32(uint32_t data);
	void AddExpectWrite16(uint16_t data);
	void AddExpectWrite8(uint8_t data);

	void ExpectClose();

private:
	void Expect(const std::string& expected, bool result);
	bool Validate(const std::string&);

	std::queue<std::string> m_expected;
	std::queue<bool> m_results;

	std::queue<uint8_t> m_read_data;
	std::queue<uint8_t> m_write_data;
};
