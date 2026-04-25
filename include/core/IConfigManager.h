#pragma once

#include "config.h"

#include <optional>
#include <string>
#include <vector>

/**
 * @file IConfigManager.h
 * @brief Thin interface capturing ConfigManager methods used by panels.
 *
 * Panels depend on this interface rather than the singleton directly,
 * enabling unit testing with GMock. The real ConfigManager implements
 * this interface directly (zero indirection overhead).
 */
class IConfigManager
{
  public:
    virtual ~IConfigManager() = default;

    /** @brief Get current configuration (const access). */
    virtual const Config::UserConfig &getConfig() const = 0;

    /** @brief Get current configuration (mutable access). */
    virtual Config::UserConfig &getConfig() = 0;

    /** @brief Get all terminal presets (const access). */
    virtual const std::vector<Config::TerminalPreset> &getTerminalPresets() const = 0;

    /** @brief Find a terminal preset by name. */
    virtual std::optional<Config::TerminalPreset> findTerminalPreset(const std::string &name) const = 0;

    /** @brief Add a new terminal preset. @return true if added. */
    virtual bool addTerminalPreset(Config::TerminalPreset preset) = 0;

    /** @brief Remove a terminal preset by name. @return true if removed. */
    virtual bool removeTerminalPreset(const std::string &name) = 0;

    /** @brief Update an existing terminal preset. @return true if updated. */
    virtual bool updateTerminalPreset(const std::string &oldName, Config::TerminalPreset preset) = 0;

    /** @brief Save current configuration to disk. @return true on success. */
    virtual bool save() = 0;
};
