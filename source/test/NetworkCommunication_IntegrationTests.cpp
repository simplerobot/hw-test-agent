#include "Test.hpp"
#include "RunServer.hpp"
#include "RunTestNetwork.hpp"
#include "Config.hpp"
#include "Params.hpp"
#include "NetworkMock.hpp"
#include "RunTestMock.hpp"
#include <unistd.h>
#include <thread>
#include <sstream>


class NetworkPipeMock : public NetworkMock
{
public:
	NetworkPipeMock(int fd_in, int fd_out)
		: m_fd_in(fd_in)
		, m_fd_out(fd_out)
	{
	}

	virtual ~NetworkPipeMock()
	{
		Close();
	}

	virtual bool Read(void* buffer, size_t length) override
	{
		size_t total_read = 0;
		while (total_read < length)
		{
			ssize_t size = ::read(m_fd_in, (uint8_t*)buffer + total_read, length - total_read);
			if (size <= 0)
				return false;
			total_read += size;
		}
		ASSERT(total_read == length);
		return true;
	}

	virtual bool Write(const void* buffer, size_t length) override
	{
		size_t total_wrote = 0;
		while (total_wrote < length)
		{
			ssize_t size = ::write(m_fd_out, (const uint8_t*)buffer + total_wrote, length - total_wrote);
			if (size <= 0)
				return false;
			total_wrote += size;
		}
		ASSERT(total_wrote == length);
		return true;
	}

	virtual void Close() override
	{
		if (m_fd_in != -1)
		{
			::close(m_fd_in);
			m_fd_in = -1;
		}
		if (m_fd_out != -1)
		{
			::close(m_fd_out);
			m_fd_out = -1;
		}
	}

private:
	int m_fd_in;
	int m_fd_out;
};


// This test case runs the client networking code against the server networking code just to make sure everything matches.
TEST_CASE(NetworkCommunications_IntegrationTest_HappyCase)
{
	int pipes_a[2];
	int pipes_b[2];
	ASSERT(::pipe(pipes_a) == 0);
	ASSERT(::pipe(pipes_b) == 0);


	std::thread client_thread([&] {
		NetworkPipeMock client_mock(pipes_a[0], pipes_b[1]);
		ConfigSection client_config("board");
		client_config.Set("host", "host.domain.com");
		client_config.Set("port", "network.port");
		std::istringstream firmware("abcd");
		Parameters client_params = {};
		client_params.board = "board-name";
		client_params.lock_timeout_ms = 0x12345678;
		client_params.test_timeout_ms = 0x2468ABCD;
		client_params.system_frequency_hz = 0x13691215;
		client_params.trace_frequency_hz = 0x48121620;
		client_mock.ExpectConnect("host.domain.com", "network.port", true);

		int result = RunTestNetwork(client_params, client_config, firmware, client_mock);
		ASSERT(result == 0); // EXITCODE_SUCCESS
	});

	std::thread server_thread([&] {
		NetworkPipeMock server_mock(pipes_b[0], pipes_a[1]);
		Parameters server_params = {};
		ConfigFile server_config;
		server_config.Add("test-section");
		Parameters expected_params = {};
		expected_params.board = "board-name";
		expected_params.file = "/tmp/hwta.fw.XXXXXX";
		expected_params.lock_timeout_ms = 0x12345678;
		expected_params.test_timeout_ms = 0x2468ABCD;
		expected_params.system_frequency_hz = 0x13691215;
		expected_params.trace_frequency_hz = 0x48121620;
		RunTestMock run_test_mock(expected_params, server_config, "test-output\n", 0); // EXITCODE_SUCCESS

		int result = RunServer(server_params, server_config, server_mock, run_test_mock);
		ASSERT(result == 0); // EXITCODE_SUCCESS
	});

	client_thread.join();
	server_thread.join();
}
