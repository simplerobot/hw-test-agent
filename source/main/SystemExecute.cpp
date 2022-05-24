#include <unistd.h>
#include <sys/wait.h>
#include "SystemExecute.hpp"


SystemExecute::SystemExecute()
	: m_input_pipe(-1)
	, m_child_pid(0)
{
}

SystemExecute::~SystemExecute()
{
	Kill();
	Wait();
}

bool SystemExecute::Run(const std::string& application, const std::vector<std::string>& params)
{
	Kill();
	Wait();

	int fd[2];
	if (::pipe(fd) != 0)
		return false;

	int input_fd = fd[0];
	int output_fd = fd[1];

	int pid = ::fork();
	if (pid == 0)
	{
		::close(input_fd);
		::dup2(output_fd, 1);
		::close(output_fd);

		std::vector<std::string> local_args;
		local_args.push_back(application);
		for (auto& p : params)
			local_args.push_back(p);

		std::vector<char*> raw_args;
		for (auto& s : local_args)
			raw_args.push_back(&s[0]);
		raw_args.push_back(nullptr);

		::execvp(application.c_str(), raw_args.data());
	}
	if (pid <= 0)
	{
		printf("ERROR: Unable to create process st-trace (%d)\n", errno);
		exit(1);
	}

	::close(output_fd);
	m_input_pipe = input_fd;
	m_child_pid = pid;
	return true;
}

int SystemExecute::Read()
{
	uint8_t buf = 0;
	ssize_t size = ::read(m_input_pipe, &buf, 1);
	if (size <= 0)
		return -1;
	return (int)(unsigned int)buf;
}

void SystemExecute::Kill()
{
	if (m_child_pid != 0)
		::kill(m_child_pid, SIGINT);
}

int SystemExecute::Wait()
{
	if (m_input_pipe >= 0)
		::close(m_input_pipe);
	m_input_pipe = -1;
	int status = 0;
	int pid = 0;
	if (m_child_pid != 0)
		while (pid != m_child_pid)
			pid = ::wait(&status);
	m_child_pid = 0;
	return status;
}

