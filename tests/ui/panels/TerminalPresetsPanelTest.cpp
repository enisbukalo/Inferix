/**
 * @file TerminalPresetsPanelTest.cpp
 * @brief Unit tests for TerminalPresetsPanel with mocked ConfigManager.
 */

#include "terminalPresetsPanel.h"

#include "MockConfigManager.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

class TerminalPresetsPanelTest : public Test
{
  protected:
    NiceMock<MockConfigManager> mockConfig;
    std::vector<Config::TerminalPreset> m_emptyPresets;
};

TEST_F(TerminalPresetsPanelTest, AddPresetSuccess)
{
    EXPECT_CALL(mockConfig, addTerminalPreset(_)).WillOnce(Return(true));

    bool result = TerminalPresetsPanel::addPreset(mockConfig, "GitUI", "gitui");
    ASSERT_TRUE(result);
}

TEST_F(TerminalPresetsPanelTest, AddPresetEmptyNameFails)
{
    // Empty name should return false without calling config
    EXPECT_CALL(mockConfig, addTerminalPreset(_)).Times(0);

    bool result = TerminalPresetsPanel::addPreset(mockConfig, "", "gitui");
    ASSERT_FALSE(result);
}

TEST_F(TerminalPresetsPanelTest, AddPresetEmptyCommandFails)
{
    EXPECT_CALL(mockConfig, addTerminalPreset(_)).Times(0);

    bool result = TerminalPresetsPanel::addPreset(mockConfig, "GitUI", "");
    ASSERT_FALSE(result);
}

TEST_F(TerminalPresetsPanelTest, RemovePresetSuccess)
{
    EXPECT_CALL(mockConfig, removeTerminalPreset("GitUI")).WillOnce(Return(true));

    bool result = TerminalPresetsPanel::removePreset(mockConfig, "GitUI");
    ASSERT_TRUE(result);
}

TEST_F(TerminalPresetsPanelTest, RemovePresetNotFound)
{
    EXPECT_CALL(mockConfig, removeTerminalPreset("NonExistent")).WillOnce(Return(false));

    bool result = TerminalPresetsPanel::removePreset(mockConfig, "NonExistent");
    ASSERT_FALSE(result);
}

TEST_F(TerminalPresetsPanelTest, RenderWithEmptyPresets)
{
    EXPECT_CALL(mockConfig, getTerminalPresets())
        .WillOnce(ReturnRef(m_emptyPresets));

    auto element = TerminalPresetsPanel::render(mockConfig);
    ASSERT_TRUE(element);
}

TEST_F(TerminalPresetsPanelTest, RenderWithMultiplePresets)
{
    std::vector<Config::TerminalPreset> presets;
    presets.push_back(Config::TerminalPreset{"Bash", "bash"});
    presets.push_back(Config::TerminalPreset{"HTOP", "htop"});

    EXPECT_CALL(mockConfig, getTerminalPresets()).WillOnce(ReturnRef(presets));

    auto element = TerminalPresetsPanel::render(mockConfig);
    ASSERT_TRUE(element);
}
