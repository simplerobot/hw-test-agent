#include "Test.hpp"
#include "Network.hpp"
#include <thread>


TEST_CASE(Network_Constructor)
{
	Network test;

	ASSERT(!test.IsValid());
}

TEST_CASE(Network_Bind_HappyCase)
{
	Network test;

	ASSERT(test.Bind("61883"));

	ASSERT(test.IsValid());
}

TEST_CASE(Network_Bind_InvalidPort)
{
	Network test;

	ASSERT(!test.Bind("invalid_port_name"));

	ASSERT(!test.IsValid());
}

TEST_CASE(Network_Bind_ForbiddenPort)
{
	Network test;

	ASSERT(!test.Bind("1023"));

	ASSERT(!test.IsValid());
}

TEST_CASE(Network_Connect_HappyCase)
{
	Network test, server;
	server.Bind("61883");

	ASSERT(test.Connect("127.0.0.1", "61883"));

	ASSERT(test.IsValid());
}

TEST_CASE(Network_Connect_NotOpen)
{
	Network test;

	ASSERT(!test.Connect("127.0.0.1", "61883"));
}

TEST_CASE(Network_Accept_HappyCase)
{
	Network test;
	test.Bind("61883");
	std::thread client_thread([] {
		Network client;
		ASSERT(client.Connect("127.0.0.1", "61883"));
	});

	auto* connection = test.Accept();

	ASSERT(connection != nullptr);
	ASSERT(connection->IsValid());
	client_thread.join();
	delete connection;
}

TEST_CASE(Network_Accept_AlreadyClosed)
{
	Network test, client;
	test.Bind("61883");
	client.Connect("127.0.0.1", "61883");
	client.Close();

	auto* connection = test.Accept();

	delete connection;
}

TEST_CASE(Network_IO_HappyCase)
{
	Network test;
	test.Bind("61883");
	std::thread client_thread([] {
		Network client;
		ASSERT(client.Connect("127.0.0.1", "61883"));
		ASSERT(client.Write("ABCD", 4));
		char buffer[4];
		ASSERT(client.Read(buffer, 4));
		ASSERT(buffer[0] == 'D' && buffer[1] == 'E' && buffer[2] == 'F' && buffer[3] == 'G');
	});

	auto* connection = test.Accept();
	ASSERT(connection != nullptr);

	char buffer[4];
	ASSERT(connection->Read(buffer, 4));
	ASSERT(buffer[0] == 'A' && buffer[1] == 'B' && buffer[2] == 'C' && buffer[3] == 'D');
	connection->Write("DEFG", 4);
	ASSERT(!connection->Read(buffer, 4));

	client_thread.join();
	delete connection;
}

TEST_CASE(Network_Read_HappyCase)
{
	Network test;
	test.Bind("61883");
	std::thread client_thread([] {
		Network client;
		ASSERT(client.Connect("127.0.0.1", "61883"));
		ASSERT(client.Write("ABCD", 4));
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		ASSERT(client.Write("EFGH", 4));
	});

	auto* connection = test.Accept();
	ASSERT(connection != nullptr);

	char buffer[8];
	ASSERT(connection->Read(buffer, 8));
	ASSERT(buffer[0] == 'A' && buffer[1] == 'B' && buffer[2] == 'C' && buffer[3] == 'D');
	ASSERT(buffer[4] == 'E' && buffer[5] == 'F' && buffer[6] == 'G' && buffer[7] == 'H');

	client_thread.join();
	delete connection;
}

TEST_CASE(Network_Read_Interrupted)
{
	Network test;
	test.Bind("61883");
	std::thread client_thread([] {
		Network client;
		ASSERT(client.Connect("127.0.0.1", "61883"));
		ASSERT(client.Write("ABCD", 4));
	});

	auto* connection = test.Accept();
	ASSERT(connection != nullptr);

	char buffer[8];
	ASSERT(!connection->Read(buffer, 8));

	client_thread.join();
	delete connection;
}

