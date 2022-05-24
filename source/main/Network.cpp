#include "Network.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>


Network::Network()
	: m_socket(-1)
{
}

Network::~Network()
{
	Close();
}

bool Network::IsValid() const
{
	return (m_socket != -1);
}

bool Network::Bind(const std::string& service)
{
	Close();

	struct addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* address_list = nullptr;
	int error = ::getaddrinfo(NULL, service.c_str(), &hints, &address_list);
	if (error != 0)
	{
		std::printf("Unable to get information about service '%s'\n", service.c_str());
		return false;
	}

	struct addrinfo* address;
	int socket = -1;
	for (address = address_list; address != nullptr; address = address->ai_next)
	{
		socket = ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		if (socket == -1)
			continue;

		if (::bind(socket, address->ai_addr, address->ai_addrlen) == 0)
			if (::listen(socket, SOMAXCONN) == 0)
				break;

		::close(socket);
	}

	::freeaddrinfo(address_list);

	if (address == nullptr)
	{
		std::printf("Unable to bind to service '%s'\n", service.c_str());
		return false;
	}

	m_socket = socket;
	return true;
}

NetworkInterface* Network::Accept()
{
	struct sockaddr_in client_address = {};
	socklen_t client_address_length = sizeof(client_address);

	int socket = ::accept(m_socket, (sockaddr*)&client_address, &client_address_length);
	if (socket == -1)
		return nullptr;

	Network* result = new Network;
	result->m_socket = socket;
	return result;
}

bool Network::Connect(const std::string& hostname, const std::string& service)
{
	Close();

	struct addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo* address_list = nullptr;
	int error = ::getaddrinfo(hostname.c_str(), service.c_str(), &hints, &address_list);
	if (error != 0)
	{
		std::printf("Unable to get information about service '%s'\n", service.c_str());
		return false;
	}

	struct addrinfo* address;
	int socket = -1;
	for (address = address_list; address != nullptr; address = address->ai_next)
	{
		socket = ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		if (socket == -1)
			continue;

		if (::connect(socket, address->ai_addr, address->ai_addrlen) == 0)
			break;

		::close(socket);
	}

	::freeaddrinfo(address_list);

	if (address == nullptr)
	{
		std::printf("Unable to connect to host '%s' service '%s'\n", hostname.c_str(), service.c_str());
		return false;
	}

	m_socket = socket;
	return true;
}

bool Network::Read(void* buffer, size_t length)
{
	while (length > 0)
	{
		ssize_t bytes_read = ::recv(m_socket, buffer, length, 0);
		if (bytes_read <= 0 || (size_t)bytes_read > length)
			return false;
		(const uint8_t*&)buffer += bytes_read;
		length -= bytes_read;
	}
	return true;
}

bool Network::Write(const void* buffer, size_t length)
{
	while (length > 0)
	{
		ssize_t bytes_sent = ::send(m_socket, buffer, length, 0);
		if (bytes_sent <= 0 || (size_t)bytes_sent > length)
			return false;
		(const uint8_t*&)buffer += bytes_sent;
		length -= bytes_sent;
	}
	return true;
}

void Network::Close()
{
	if (m_socket != -1)
		::close(m_socket);
	m_socket = -1;
}

