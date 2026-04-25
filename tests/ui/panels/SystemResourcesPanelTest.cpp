/**
 * @file SystemResourcesPanelTest.cpp
 * @brief Unit tests for SystemResourcesPanel::render() with mocked monitors.
 */

#include "systemResourcesPanel.h"

#include "MockCpuMonitor.h"
#include "MockMemoryMonitor.h"
#include "MockGpuMonitor.h"
#include "MockModelInfoMonitor.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

class SystemResourcesPanelTest : public Test
{
  protected:
    NiceMock<MockCpuMonitor> mockCpu;
    NiceMock<MockMemoryMonitor> mockMem;
    NiceMock<MockGpuMonitor> mockGpu;
    NiceMock<MockModelInfoMonitor> mockModelInfo;
};

TEST_F(SystemResourcesPanelTest, RenderWithZeroStats)
{
    // Default stats are all zeros — should not crash
    EXPECT_CALL(mockCpu, getStats()).Times(1);
    EXPECT_CALL(mockMem, getStats()).Times(1);
    EXPECT_CALL(mockGpu, getStats()).Times(1);
    EXPECT_CALL(mockGpu, getLoadStats()).Times(1);

    auto element = SystemResourcesPanel::render(mockCpu, mockMem, mockGpu, mockModelInfo);
    ASSERT_TRUE(element);  // Element should be non-null
}

TEST_F(SystemResourcesPanelTest, RenderWithCustomRamStats)
{
    MemoryStats ram;
    ram.totalMb = 32768;
    ram.usedMb = 16384;
    ram.availableMb = 16384;
    ram.usagePercentage = 50.0;

    EXPECT_CALL(mockMem, getStats()).WillOnce(Return(ram));
    EXPECT_CALL(mockCpu, getStats()).Times(1);
    EXPECT_CALL(mockGpu, getStats()).Times(1);
    EXPECT_CALL(mockGpu, getLoadStats()).Times(1);

    auto element = SystemResourcesPanel::render(mockCpu, mockMem, mockGpu, mockModelInfo);
    ASSERT_TRUE(element);
}

TEST_F(SystemResourcesPanelTest, RenderWithSingleGpu)
{
    MemoryStats vram;
    vram.id = 0;
    vram.totalMb = 24576;
    vram.usedMb = 12288;
    vram.availableMb = 12288;
    vram.usagePercentage = 50.0;

    ProcessorStats gpuLoad;
    gpuLoad.usagePercentage = 75.0;

    EXPECT_CALL(mockGpu, getStats()).WillOnce(Return(std::vector<MemoryStats>{vram}));
    EXPECT_CALL(mockGpu, getLoadStats())
        .WillOnce(Return(std::vector<ProcessorStats>{gpuLoad}));
    EXPECT_CALL(mockCpu, getStats()).Times(1);
    EXPECT_CALL(mockMem, getStats()).Times(1);

    auto element = SystemResourcesPanel::render(mockCpu, mockMem, mockGpu, mockModelInfo);
    ASSERT_TRUE(element);
}

TEST_F(SystemResourcesPanelTest, RenderWithMultipleGpus)
{
    std::vector<MemoryStats> vrams;
    for (int i = 0; i < 4; ++i) {
        MemoryStats s;
        s.id = i;
        s.totalMb = 24576;
        s.usedMb = 8192;
        s.availableMb = 16384;
        s.usagePercentage = 33.3;
        vrams.push_back(s);
    }

    std::vector<ProcessorStats> loads(4);
    for (int i = 0; i < 4; ++i) {
        loads[i].usagePercentage = static_cast<double>(i * 25);
    }

    EXPECT_CALL(mockGpu, getStats()).WillOnce(Return(vrams));
    EXPECT_CALL(mockGpu, getLoadStats()).WillOnce(Return(loads));
    EXPECT_CALL(mockCpu, getStats()).Times(1);
    EXPECT_CALL(mockMem, getStats()).Times(1);

    auto element = SystemResourcesPanel::render(mockCpu, mockMem, mockGpu, mockModelInfo);
    ASSERT_TRUE(element);
}
