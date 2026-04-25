#pragma once

#include "IConfigManager.h"

#include <gmock/gmock.h>

class MockConfigManager : public IConfigManager
{
  public:
    MOCK_CONST_METHOD0(getConfig, const Config::UserConfig &());
    MOCK_METHOD0(getConfig, Config::UserConfig &());
    MOCK_CONST_METHOD0(getTerminalPresets, const std::vector<Config::TerminalPreset> &());
    MOCK_CONST_METHOD1(findTerminalPreset, std::optional<Config::TerminalPreset>(const std::string &name));
    MOCK_METHOD1(addTerminalPreset, bool(Config::TerminalPreset preset));
    MOCK_METHOD1(removeTerminalPreset, bool(const std::string &name));
    MOCK_METHOD2(updateTerminalPreset, bool(const std::string &oldName, Config::TerminalPreset preset));
    MOCK_METHOD0(save, bool());
};
