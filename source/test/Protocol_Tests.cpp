#include "Test.hpp"
#include "Protocol.hpp"
#include <sstream>
#include "NetworkMock.hpp"


TEST_CASE(Protocol_Constructor)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	ASSERT(test);
}

TEST_CASE(Protocol_Error)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	test.Error("Mock Error: Everything isn't awesome!\n");

	ASSERT(!test);
}

TEST_CASE(Protocol_Write8_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectWrite("a", 1);

	uint8_t value = 'a';
	test.Write(value);

	ASSERT(test);
}

TEST_CASE(Protocol_Write8_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	uint8_t value = 'a';
	test.Write(value);

	ASSERT(!test);
}

TEST_CASE(Protocol_Write16_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectWrite("ab", 2);

	uint16_t value = 0x6162; // 'ab'
	test.Write(value);

	ASSERT(test);
}

TEST_CASE(Protocol_Write16_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	uint16_t value = 0x6162; // 'ab'
	test.Write(value);

	ASSERT(!test);
}

TEST_CASE(Protocol_Write32_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectWrite("abcd", 4);

	uint32_t value = 0x61626364; // 'abcd'
	test.Write(value);

	ASSERT(test);
}

TEST_CASE(Protocol_Write32_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	uint32_t value = 0x61626364; // 'abcd'
	test.Write(value);

	ASSERT(!test);
}

TEST_CASE(Protocol_WriteString_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectWrite("abcdefg", 8);
	test.SetMaxStringLength(7);

	test.Write("abcdefg");

	ASSERT(test);
}

TEST_CASE(Protocol_WriteString_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectWrite("abcdefg", 7);

	test.Write("abcdefg");

	ASSERT(!test);
}

TEST_CASE(Protocol_WriteString_IncludesNul)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectWrite("abcd", 4);

	test.Write(std::string("abcd\0", 5));

	ASSERT(!test);
}

TEST_CASE(Protocol_WriteString_TooLong)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	test.SetMaxStringLength(7);

	test.Write("abcdefgh");

	ASSERT(!test);
}

TEST_CASE(Protocol_WriteBinary_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	char expected_buffer[] = { 7, 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
	mock_network.AddExpectWrite(expected_buffer, sizeof(expected_buffer));
	test.SetMaxBinaryLength(7);
	std::istringstream in(std::string(expected_buffer + 1, sizeof(expected_buffer) - 1));

	test.Write(in);

	ASSERT(test);
}

TEST_CASE(Protocol_WriteBinary_IncludesNul)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	char expected_buffer[] = { 7, 'A', 'B', 0, 'D', 'E', 'F', 'G' };
	mock_network.AddExpectWrite(expected_buffer, sizeof(expected_buffer));
	test.SetMaxBinaryLength(7);
	std::istringstream in(std::string(expected_buffer + 1, sizeof(expected_buffer) - 1));

	test.Write(in);

	ASSERT(test);
}

TEST_CASE(Protocol_WriteBinary_Oversized)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	char expected_buffer[] = { 7, 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
	test.SetMaxBinaryLength(6);
	std::istringstream in(std::string(expected_buffer + 1, sizeof(expected_buffer) - 1));

	test.Write(in);

	ASSERT(!test);
}

TEST_CASE(Protocol_WriteBinary_Large)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	test.SetMaxBinaryLength(2000);
	std::ostringstream expected, input;
	for (size_t i = 0; i < 2000; i++)
	{
		if (i % 255 == 0)
			expected.put((uint8_t)std::min<size_t>(2000 - i, 255));
		expected.put('A' + (i % 26));
		input.put('A' + (i % 26));
	}
	std::string expected_str = expected.str();
	mock_network.AddExpectWrite(expected_str.data(), expected_str.size());
	std::istringstream in(input.str());

	test.Write(in);

	ASSERT(test);
}

TEST_CASE(Protocol_Read8_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectRead("a", 1);

	uint8_t data = 0;
	test.Read(data);

	ASSERT(data == 'a');
	ASSERT(test);
}

TEST_CASE(Protocol_Read8_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	uint8_t data = 0;
	test.Read(data);

	ASSERT(!test);
}

TEST_CASE(Protocol_Read16_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectRead("ab", 2);

	uint16_t data = 0;
	test.Read(data);

	ASSERT(data == 0x6162);
	ASSERT(test);
}

TEST_CASE(Protocol_Read16_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	uint16_t data = 0;
	test.Read(data);

	ASSERT(!test);
}

TEST_CASE(Protocol_Read32_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectRead("abcd", 4);

	uint32_t data = 0;
	test.Read(data);

	ASSERT(data == 0x61626364);
	ASSERT(test);
}

TEST_CASE(Protocol_Read32_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	uint32_t data = 0;
	test.Read(data);

	ASSERT(!test);
}

TEST_CASE(Protocol_ReadString_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectRead("hello world", 12);
	test.SetMaxStringLength(11);

	std::string data;
	test.Read(data);

	ASSERT(data == "hello world");
	ASSERT(test);
}

TEST_CASE(Protocol_ReadString_EOF)
{
	NetworkMock mock_network;
	Protocol test(mock_network);

	std::string data;
	test.Read(data);

	ASSERT(data == "");
	ASSERT(!test);
}

TEST_CASE(Protocol_ReadString_TooLong)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	mock_network.AddExpectRead("hello world", 10);
	test.SetMaxStringLength(10);

	std::string data;
	test.Read(data);

	ASSERT(data == "hello worl");
	ASSERT(!test);
}

TEST_CASE(Protocol_ReadBinary_HappyCase)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	std::ostringstream expected, input;
	for (size_t i = 0; i < 2000; i++)
	{
		if (i % 255 == 0)
			expected.put((uint8_t)std::min<size_t>(2000 - i, 255));
		expected.put('A' + (i % 26));
		input.put('A' + (i % 26));
	}
	std::string expected_str = expected.str();
	mock_network.AddExpectRead(expected_str.data(), expected_str.length());
	test.SetMaxBinaryLength(2000);

	std::ostringstream actual;
	test.Read(actual);

	ASSERT(actual.str() == input.str());
	ASSERT(test);
}

TEST_CASE(Protocol_ReadBinary_TooLong)
{
	NetworkMock mock_network;
	Protocol test(mock_network);
	std::ostringstream expected, input;
	for (size_t i = 0; i < 2000; i++)
	{
		if (i % 255 == 0)
			expected.put((uint8_t)std::min<size_t>(2000 - i, 255));
		expected.put('A' + (i % 26));
		input.put('A' + (i % 26));
	}
	std::string expected_str = expected.str();
	mock_network.AddExpectRead(expected_str.data(), expected_str.length() - 215);
	test.SetMaxBinaryLength(1999);

	std::ostringstream actual;
	test.Read(actual);

	ASSERT(!test);
}
