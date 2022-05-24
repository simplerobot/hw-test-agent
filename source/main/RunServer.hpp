#pragma once


class ConfigFile;
struct Parameters;
class NetworkInterface;
class RunTestInterface;

extern int RunServer(const Parameters& params, const ConfigFile& config);
extern int RunServer(const Parameters& params, const ConfigFile& config, NetworkInterface& connection, RunTestInterface& test_runner);
