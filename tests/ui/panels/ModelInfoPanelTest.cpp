/**
 * @file ModelInfoPanelTest.cpp
 * @brief Unit tests for ModelInfoPanel::render() with mocked monitor.
 */

#include "modelInfoPanel.h"

#include "MockModelInfoMonitor.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

class ModelInfoPanelTest : public Test
{
  protected:
    NiceMock<MockModelInfoMonitor> mockMonitor;
};

TEST_F(ModelInfoPanelTest, RenderWithNoServer)
{
    ModelInfo info;
    info.isServerRunning = false;
    info.isModelLoaded = false;

    EXPECT_CALL(mockMonitor, getStats()).WillOnce(Return(info));

    auto element = ModelInfoPanel::render(mockMonitor);
    ASSERT_TRUE(element);
}

TEST_F(ModelInfoPanelTest, RenderWithIdleModel)
{
    ModelInfo info;
    info.isServerRunning = true;
    info.isModelLoaded = true;
    info.isIdle = true;
    info.loadedModel = "test-model";
    info.generationTokensPerSec = 45.2;
    info.processingTokensPerSec = 120.5;

    EXPECT_CALL(mockMonitor, getStats()).WillOnce(Return(info));

    auto element = ModelInfoPanel::render(mockMonitor);
    ASSERT_TRUE(element);
}

TEST_F(ModelInfoPanelTest, RenderWithProcessingModel)
{
    ModelInfo info;
    info.isServerRunning = true;
    info.isModelLoaded = true;
    info.isIdle = false;
    info.activeRequestCount = 2;
    info.loadedModel = "processing-model";
    info.generationTokensPerSec = 30.0;
    info.processingTokensPerSec = 50.0;

    EXPECT_CALL(mockMonitor, getStats()).WillOnce(Return(info));

    auto element = ModelInfoPanel::render(mockMonitor);
    ASSERT_TRUE(element);
}

TEST_F(ModelInfoPanelTest, RenderWithNoModelLoaded)
{
    ModelInfo info;
    info.isServerRunning = true;
    info.isModelLoaded = false;

    EXPECT_CALL(mockMonitor, getStats()).WillOnce(Return(info));

    auto element = ModelInfoPanel::render(mockMonitor);
    ASSERT_TRUE(element);
}

TEST_F(ModelInfoPanelTest, FormatDoublePrecision)
{
    // Test static helper methods directly
    std::string result = ModelInfoPanel::formatDouble(3.14159, 2);
    EXPECT_EQ(result, "3.14");

    result = ModelInfoPanel::formatDouble(0.0, 1);
    EXPECT_EQ(result, "0.0");
}

TEST_F(ModelInfoPanelTest, FormatNumberWithCommas)
{
    std::string result = ModelInfoPanel::formatNumber(1234567);
    EXPECT_EQ(result, "1,234,567");

    result = ModelInfoPanel::formatNumber(0);
    EXPECT_EQ(result, "0");
}

TEST_F(ModelInfoPanelTest, GetStatusString)
{
    ModelInfo info;

    // N/A when server offline
    info.isServerRunning = false;
    EXPECT_EQ(ModelInfoPanel::getStatusString(info), "N/A");

    // N/A when no model loaded
    info.isServerRunning = true;
    info.isModelLoaded = false;
    EXPECT_EQ(ModelInfoPanel::getStatusString(info), "N/A");

    // Idle
    info.isServerRunning = true;
    info.isModelLoaded = true;
    info.isIdle = true;
    EXPECT_EQ(ModelInfoPanel::getStatusString(info), "Idle");

    // Processing
    info.isIdle = false;
    EXPECT_EQ(ModelInfoPanel::getStatusString(info), "Processing");
}
