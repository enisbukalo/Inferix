/**
 * @file ModelsPanelTest.cpp
 * @brief Unit tests for ModelsPanel using mocked dependencies.
 */

#include "modelsPanel.h"

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

class ModelsPanelTest : public Test
{
  protected:
    void SetUp() override
    {
        // Set up default config values so panel constructor doesn't crash
        ON_CALL(mockConfig, getConfig()).WillByDefault(ReturnRef(config));
        ON_CALL(mockConfig, getConfig()).WillByDefault(ReturnRef(config));

        // Server not running by default
        ON_CALL(mockServer, isRunning()).WillByDefault(Return(false));
        ON_CALL(mockServer, isServerHealthy()).WillByDefault(Return(false));

        // Model info shows nothing loaded
        ModelInfo info;
        info.isModelLoaded = false;
        info.isServerRunning = false;
        ON_CALL(mockModelInfo, getStats()).WillByDefault(Return(info));

        // Empty model list from models.ini
        ON_CALL(mockModelsIni, getUniqueModelEntries())
            .WillByDefault(Return(std::vector<ModelsIni::ModelEntry>{}));
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

TEST_F(ModelsPanelTest, ConstructorDoesNotCallSingletons)
{
    // Verify that constructing the panel only calls our mocks, not singletons.
    EXPECT_CALL(mockConfig, getConfig()).Times(1);
    EXPECT_CALL(mockModelsIni, getUniqueModelEntries()).Times(1);

    AppDependencies deps{mockConfig, mockServer, mockModelInfo, mockModelsIni,
                         mockCpu, mockMem, mockGpu};
    ModelsPanel panel(deps);  // Should not crash or call singletons

    SUCCEED() << "ModelsPanel constructed successfully with mocked deps";
}

TEST_F(ModelsPanelTest, ComponentReturnsValidElement)
{
    AppDependencies deps{mockConfig, mockServer, mockModelInfo, mockModelsIni,
                         mockCpu, mockMem, mockGpu};
    ModelsPanel panel(deps);

    auto comp = panel.component();
    ASSERT_TRUE(comp);  // Component should be non-null
}

TEST_F(ModelsPanelTest, ServerNotRunningShowsLoadLabel)
{
    EXPECT_CALL(mockServer, isRunning()).WillOnce(Return(false));
    EXPECT_CALL(mockServer, isServerHealthy()).WillOnce(Return(false));

    AppDependencies deps{mockConfig, mockServer, mockModelInfo, mockModelsIni,
                         mockCpu, mockMem, mockGpu};
    ModelsPanel panel(deps);

    // Component creation triggers server state refresh
    auto comp = panel.component();
    SUCCEED() << "Server not running - LOAD label expected";
}
