#include "Config.hpp"
#include <iostream>
#include <cctype>
#include <sstream>


ConfigSection::ConfigSection(const std::string& name)
	: m_name(name)
{
}

std::string ConfigSection::Name() const
{
	return m_name;
}

bool ConfigSection::Has(const std::string& key) const
{
	return (m_settings.find(key) != m_settings.end());
}

std::string ConfigSection::Get(const std::string& key, const std::string default_value) const
{
	auto i = m_settings.find(key);
	if (i == m_settings.end())
		return default_value;
	return i->second;
}

std::string ConfigSection::Get(const std::string& key) const
{
	auto i = m_settings.find(key);
	if (i == m_settings.end())
		throw std::runtime_error("Getting a non-existant key from a ConfigSection.");
	return i->second;
}

void ConfigSection::Set(const std::string& key, const std::string& value)
{
	m_settings[key] = value;
}

std::ostream& ConfigSection::Write(std::ostream& out) const
{
	out << "ConfigSection '" << m_name << "' {";
	for (auto& setting : m_settings)
		out << " '" << setting.first << "' = '" << setting.second << "',";
	out << " }";

	return out;
}

ConfigFile::ConfigFile()
{
}

static void ltrim(const char*& start, const char* end = nullptr)
{
	while (start != end && std::isspace(*start))
		start++;
}

static void rtrim(const char* start, const char*& end)
{
	while (end != start && std::isspace(*(end - 1)))
		end--;
}

static void trim(const char*& start, const char*& end)
{
	ltrim(start, end);
	rtrim(start, end);
}

static bool IsValidTokenStart(char c)
{
	return std::isalpha(c) || c == '_';
}

static bool IsValidTokenPart(char c)
{
	return std::isalnum(c) || c == '_' || c == '-' || c == ' ';
}

bool ConfigFile::IsValidTokenName(const std::string& token)
{
	return IsValidTokenName(&*token.begin(), &*token.end());
}

bool ConfigFile::IsValidTokenName(const char* start, const char* end)
{
	if (start == end)
		return false;
	if (!IsValidTokenStart(*start))
		return false;
	for (const char* cursor = start + 1; cursor < end; cursor++)
		if (!IsValidTokenPart(*cursor))
			return false;
	return true;
}

bool ConfigFile::ParseLine(const char* line, long line_number)
{
	// Trim spaces from the front of the word.
	const char* start = line;
	ltrim(start);
	// Ignore everything after a comment character and trim any spaces from the end of the string.
	const char* end = start;
	while (*end != 0 && *end != '#')
		end++;
	rtrim(start, end);

	// Skip empty lines.
	if (start == end)
		return true;

	// Look for section headers.
	if (*start == '[')
	{
		if (*(end - 1) != ']')
		{
			std::printf("Line %ld: Error: Misformed section header.\n", line_number);
			return false;
		}
		// Remove the section tokens.
		start++;
		end--;
		// Validate the section name.
		trim(start, end);
		if (!IsValidTokenName(start, end))
		{
			std::printf("Line %ld: Error: Invalid section name.\n", line_number);
			return false;
		}
		Add(std::string(start, end));
		return true;
	}

	// This line should be token = value.
	const char* cursor = start;
	while (cursor < end && *cursor != '=')
		cursor++;
	if (cursor == end)
	{
		std::printf("Line %ld: Error: Invalid line with no assignment.\n", line_number);
		return false;
	}

	const char* token_end = cursor;
	rtrim(start, token_end);
	if (!IsValidTokenName(start, token_end))
	{
		std::printf("Line %ld: Error: Invalid assignment token character.\n", line_number);
		return false;
	}

	const char* value_start = cursor + 1;
	ltrim(value_start, end);

	std::string token(start, token_end);
	std::string value(value_start, end);

	if (m_sections.empty())
	{
		std::printf("Line %ld: Error: Assignment token '%s' outside of all sections.\n", line_number, token.c_str());
		return false;
	}

	if (m_sections.back().Has(token))
	{
		std::printf("Line %ld: Error: Duplicate token '%s'.\n", line_number, token.c_str());
		return false;
	}

	m_sections.back().Set(token, value);
	return true;
}

ConfigFile::ConfigFile(std::istream& input)
{
	std::string line;
	for (long line_number = 1; std::getline(input, line); line_number++)
		ParseLine(line.c_str(), line_number);
}

void ConfigFile::Add(const std::string& section_name)
{
	Add(ConfigSection(section_name));
}

void ConfigFile::Add(const ConfigSection& config)
{
	m_sections.push_back(config);
}

std::vector<ConfigSection> ConfigFile::Find(const std::string& section_name) const
{
	std::vector<ConfigSection> result;
	for (auto& s : m_sections)
		if (s.Name() == section_name)
			result.push_back(s);
	return result;
}

size_t ConfigFile::Count() const
{
	return m_sections.size();
}

ConfigSection ConfigFile::Get(size_t i) const
{
	return m_sections[i];
}

std::ostream& ConfigFile::Write(std::ostream& out) const
{
	out << "ConfigFile {";
	for (auto& s : m_sections)
	{
		out << " ";
		s.Write(out);
	}
	out << " }";
	return out;
}

extern std::string ToString(const ConfigSection& config)
{
	std::ostringstream out;
	config.Write(out);
	return out.str();
}

extern std::string ToString(const ConfigFile& config)
{
	std::ostringstream out;
	config.Write(out);
	return out.str();
}

