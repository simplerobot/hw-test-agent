#pragma once

#include <iostream>


class ConfigSection;
struct Parameters;
class NetworkInterface;


extern int RunTestNetwork(const Parameters& params, const ConfigSection& config);
extern int RunTestNetwork(const Parameters& params, const ConfigSection& config, std::istream& firmware, NetworkInterface& network);
