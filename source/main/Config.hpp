#pragma once

#include <string>
#include <map>
#include <vector>


class ConfigSection
{
public:
	ConfigSection(const std::string& name);

	std::string Name() const;

	bool Has(const std::string& key) const;
	std::string Get(const std::string& key) const;
	std::string Get(const std::string& key, const std::string default_value) const;

	void Set(const std::string& key, const std::string& value);

	std::ostream& Write(std::ostream& out) const;

private:
	std::string m_name;
	std::map<std::string, std::string> m_settings;
};


class ConfigFile
{
public:
	ConfigFile();
	ConfigFile(std::istream& input);

	void Add(const std::string& section_name);
	void Add(const ConfigSection& config);
	bool ParseLine(const char* line, long line_number);

	std::vector<ConfigSection> Find(const std::string& section_name) const;

	size_t Count() const;
	ConfigSection Get(size_t) const;

	static bool IsValidTokenName(const std::string& token);
	static bool IsValidTokenName(const char* start, const char* end);

	std::ostream& Write(std::ostream& out) const;

private:
	std::vector<ConfigSection> m_sections;
};

extern std::string ToString(const ConfigSection& config);
extern std::string ToString(const ConfigFile& config);

