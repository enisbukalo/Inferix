/**
 * @file SettingsPanelTest.cpp
 * @brief Unit tests for SettingsPanel using mocked ConfigManager.
 */

#include "settingsPanel.h"

#include "MockConfigManager.h"
#include "MockLlamaServerProcess.h"
#include "MockModelInfoMonitor.h"
#include "MockModelsIni.h"
#include "MockCpuMonitor.h"
#include "MockMemoryMonitor.h"
#include "MockGpuMonitor.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

class SettingsPanelTest : public Test
{
  protected:
    void SetUp() override
    {
        ON_CALL(mockConfig, getConfig()).WillByDefault(ReturnRef(config));
        ON_CALL(mockConfig, getConfig()).WillByDefault(ReturnRef(config));
    }

    NiceMock<MockConfigManager> mockConfig;
    NiceMock<MockLlamaServerProcess> mockServer;
    NiceMock<MockModelInfoMonitor> mockModelInfo;
    NiceMock<MockModelsIni> mockModelsIni;
    NiceMock<MockCpuMonitor> mockCpu;
    NiceMock<MockMemoryMonitor> mockMem;
    NiceMock<MockGpuMonitor> mockGpu;
    Config::UserConfig config{};
};

TEST_F(SettingsPanelTest, ConstructorLoadsFromMockConfig)
{
    EXPECT_CALL(mockConfig, getConfig()).Times(1);

    AppDependencies deps{mockConfig, mockServer, mockModelInfo, mockModelsIni,
                         mockCpu, mockMem, mockGpu};

    SettingsPanel panel(deps);
    SUCCEED() << "SettingsPanel constructed with mocked config";
}

TEST_F(SettingsPanelTest, ComponentReturnsValidElement)
{
    AppDependencies deps{mockConfig, mockServer, mockModelInfo, mockModelsIni,
                          mockCpu, mockMem, mockGpu};

    SettingsPanel panel(deps);
    auto comp = panel.component();
    ASSERT_TRUE(comp);
}
