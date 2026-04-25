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
