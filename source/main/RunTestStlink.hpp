#pragma once

class SystemExecuteInterface;
class ConfigFile;
class ConfigSection;
struct Parameters;


extern int RunTestStlink(const Parameters& params, const ConfigSection& config);
extern int RunTestStlink(const Parameters& params, const ConfigSection& config, SystemExecuteInterface& exec);
