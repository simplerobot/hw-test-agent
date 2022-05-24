#include "Test.hpp"
#include "Config.hpp"
#include <sstream>


TEST_CASE(ConfigSection_Constructor_HappyCase)
{
	ConfigSection test("test-name");

	ASSERT(test.Name() == "test-name");
}

TEST_CASE(ConfigSection_Has_Empty)
{
	ConfigSection test("test-name");

	ASSERT(!test.Has("key-name"));
}

TEST_CASE(ConfigSection_Set_HappyCase)
{
	ConfigSection test("test-name");

	test.Set("key-name", "value-name");

	ASSERT(test.Has("key-name"));
}

TEST_CASE(ConfigSection_Get_HappyCase)
{
	ConfigSection test("test-name");
	test.Set("key-name", "value-name");

	ASSERT(test.Get("key-name") == "value-name");
}

TEST_CASE(ConfigSection_Get_Missing)
{
	ConfigSection test("test-name");

	ASSERT_THROWS(test.Get("key-name"));
}

TEST_CASE(ConfigSection_Get2_HappyCase)
{
	ConfigSection test("test-name");
	test.Set("key-name", "value-name");

	ASSERT(test.Get("key-name", "default-value") == "value-name");
}

TEST_CASE(ConfigSection_Get2_Missing)
{
	ConfigSection test("test-name");

	ASSERT(test.Get("key-name", "default-value") == "default-value");
}

TEST_CASE(ConfigSection_ToString_HappyCase)
{
	ConfigSection test("config-name");
	test.Set("key", "value");
	std::string expected = "ConfigSection 'config-name' { 'key' = 'value', }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}

TEST_CASE(ConfigSection_ToString_Empty)
{
	ConfigSection test("config-name");
	std::string expected = "ConfigSection 'config-name' { }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}

TEST_CASE(ConfigFile_DefaultConstructor)
{
	ConfigFile test;
}

TEST_CASE(ConfigFile_IsValidTokenName_Valid)
{
	const char* valid_tokens[] = {
		"a",
		"A",
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-",
		"abc def",
		"_abc",
	};

	for (const char* s : valid_tokens)
		ASSERT(ConfigFile::IsValidTokenName(s));
}

TEST_CASE(ConfigFile_IsValidTokenName_Invalid)
{
	const char* invalid_tokens[] = {
		"",
		" ",
		" abc",
		"-abc",
		"%abc",
		"ab%c",
	};

	for (const char* s : invalid_tokens)
		ASSERT(!ConfigFile::IsValidTokenName(s));
}

TEST_CASE(ConfigFile_Find_Missing)
{
	ConfigFile test;

	auto result = test.Find("missing-section");

	ASSERT(result.empty());
}

TEST_CASE(ConfigFile_Add_HappyCase)
{
	ConfigFile test;

	test.Add("section-name");

	auto result = test.Find("section-name");
	ASSERT(result.size() == 1);
	ASSERT(result.front().Name() == "section-name");
}

TEST_CASE(ConfigFile_Add_Duplicate)
{
	ConfigFile test;

	test.Add("section-name");
	test.Add("section-name");

	auto result = test.Find("section-name");
	ASSERT(result.size() == 2);
	ASSERT(result[0].Name() == "section-name");
	ASSERT(result[1].Name() == "section-name");
}

TEST_CASE(ConfigFile_Add2_HappyCase)
{
	ConfigFile test;
	ConfigSection section("section-name");

	test.Add(section);

	auto result = test.Find("section-name");
	ASSERT(result.size() == 1);
	ASSERT(result.front().Name() == "section-name");
}

TEST_CASE(ConfigFile_ParseLine_Empty)
{
	ConfigFile test;

	ASSERT(test.ParseLine("", 10));
	ASSERT(test.ParseLine(" ", 10));
	ASSERT(test.ParseLine("\t", 10));
}

TEST_CASE(ConfigFile_ParseLine_Comment)
{
	ConfigFile test;

	ASSERT(test.ParseLine("# ignore", 10));
	ASSERT(test.ParseLine(" # ignore it all", 10));
	ASSERT(test.ParseLine("\t # ignore more", 10));
	ASSERT(test.ParseLine("# ignore #", 10));
}

TEST_CASE(ConfigFile_ParseLine_ValidSectionHeader)
{
	ConfigFile test;

	ASSERT(test.ParseLine("[section-name]", 10));
	ASSERT(test.ParseLine("[section-two] \t", 10));
	ASSERT(test.ParseLine("[section-three] \t# ignore", 10));
	ASSERT(test.ParseLine(" \t[section-four]", 10));
	ASSERT(test.ParseLine("[ \tsection-five \t]", 10));

	ASSERT(test.Find("section-name").size() == 1);
	ASSERT(test.Find("section-two").size() == 1);
	ASSERT(test.Find("section-three").size() == 1);
	ASSERT(test.Find("section-four").size() == 1);
	ASSERT(test.Find("section-five").size() == 1);
}

TEST_CASE(ConfigFile_ParseLine_DuplicateSectionHeader)
{
	ConfigFile test;

	ASSERT(test.ParseLine("[section-name]", 10));
	ASSERT(test.ParseLine("[section-two]", 10));
	ASSERT(test.ParseLine("[section-name]", 10));

	ASSERT(test.Find("section-name").size() == 2);
	ASSERT(test.Find("section-two").size() == 1);
}

TEST_CASE(ConfigFile_ParseLine_InvalidSectionHeader)
{
	ConfigFile test;

	ASSERT(!test.ParseLine("[", 10));
	ASSERT(!test.ParseLine("[section-name", 10));
	ASSERT(!test.ParseLine("[section-name] x", 10));
	ASSERT(!test.ParseLine("[invalid-^-section-name]", 10));
}

TEST_CASE(ConfigFile_ParseLine_ValidAssignment)
{
	ConfigFile test;
	test.ParseLine("[section-name]", 10);

	ASSERT(test.ParseLine("token-name = value-name", 10));
	ASSERT(test.ParseLine("token-two = ", 10));
	ASSERT(test.ParseLine("token-three = value # comment", 10));
	ASSERT(test.ParseLine("token-four = !@$%^&*()-=[]", 10));

	auto section = test.Find("section-name");
	ASSERT(section.size() == 1);
	ASSERT(section[0].Get("token-name") == "value-name");
	ASSERT(section[0].Get("token-two") == "");
	ASSERT(section[0].Get("token-three") == "value");
	ASSERT(section[0].Get("token-four") == "!@$%^&*()-=[]");
}

TEST_CASE(ConfigFile_ParseLine_AssignmentNoSection)
{
	ConfigFile test;

	ASSERT(!test.ParseLine("token-name = value-name", 10));
}

TEST_CASE(ConfigFile_ParseLine_InvalidToken)
{
	ConfigFile test;
	test.ParseLine("[section-name]", 10);

	ASSERT(!test.ParseLine("invalid-^-token-name = value-name", 10));
}

TEST_CASE(ConfigFile_ParseLine_DuplicateAssignment)
{
	ConfigFile test;
	test.ParseLine("[section-name]", 10);

	ASSERT(test.ParseLine("token-name = value-name", 10));
	ASSERT(!test.ParseLine("token-name = value-name-2", 10));
}

TEST_CASE(ConfigFile_ParseConstructor_HappyCase)
{
	std::istringstream input("[section-name]\ntoken-name = value-name\n[section-two]\ntoken-two = value-two\ntoken-three = value-three");
	ConfigFile test(input);

	auto section_one = test.Find("section-name");
	ASSERT(section_one.size() == 1);
	ASSERT(section_one[0].Get("token-name") == "value-name");
	auto section_two = test.Find("section-two");
	ASSERT(section_two.size() == 1);
	ASSERT(section_two[0].Get("token-two") == "value-two");
	ASSERT(section_two[0].Get("token-three") == "value-three");
}

TEST_CASE(ConfigFile_ToString_HappyCase)
{
	ConfigSection section("section-name");
	section.Set("key", "value");
	ConfigFile test;
	test.Add(section);
	std::string expected = "ConfigFile { ConfigSection 'section-name' { 'key' = 'value', } }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}

TEST_CASE(ConfigFile_ToString_Empty)
{
	ConfigFile test;
	std::string expected = "ConfigFile { }";

	std::string actual = ToString(test);

	ASSERT(actual == expected);
}

