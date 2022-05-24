#include "Test.hpp"

#include "SystemExecute.hpp"


TEST_CASE(SystemExecute_Constructor)
{
	SystemExecute test;
}

TEST_CASE(SystemExecute_Run_HappyCase)
{
	SystemExecute test;

	ASSERT(test.Run("echo", { "hello", "world" }));

	const char* expected = "hello world\n";
	for (size_t i = 0; expected[i] != 0; i++)
		ASSERT(test.Read() == expected[i]);
	ASSERT(test.Read() == -1);
}

TEST_CASE(SystemExecute_Run_MissingProgram)
{
	SystemExecute test;

	ASSERT(test.Run("missing-program", { "hello", "world" }));

	const char* expected = "ERROR:";
	for (size_t i = 0; expected[i] != 0; i++)
		ASSERT(test.Read() == expected[i]);
}

TEST_CASE(SystemExecute_Run_Multiple)
{
	SystemExecute test;

	ASSERT(test.Run("echo", { "hello", "world" }));
	ASSERT(test.Run("echo", { "second", "call" }));

	const char* expected = "second call\n";
	for (size_t i = 0; expected[i] != 0; i++)
		ASSERT(test.Read() == expected[i]);
	ASSERT(test.Read() == -1);
}

TEST_CASE(SystemExecute_Kill)
{
	SystemExecute test;
	test.Run("sleep", { "5" });
	time_t start_time = ::time(nullptr);

	test.Kill();
	test.Wait();

	ASSERT(::time(nullptr) - start_time <= 1);
}

