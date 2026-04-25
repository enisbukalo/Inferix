#pragma once

#include "modelInfo.h"

/**
 * @file IModelInfoMonitor.h
 * @brief Thin interface capturing ModelInfoMonitor methods used by panels.
 *
 * Panels depend on this interface rather than the singleton directly,
 * enabling unit testing with GMock. The real ModelInfoMonitor implements
 * this interface directly (zero indirection overhead).
 */
class IModelInfoMonitor
{
  public:
    virtual ~IModelInfoMonitor() = default;

    /** @brief Returns the latest ModelInfo snapshot. */
    virtual ModelInfo getStats() const = 0;

    /** @brief Signals that the model was intentionally unloaded. */
    virtual void setUnloaded() = 0;

    /** @brief Clears the force-unloaded flag so monitoring resumes. */
    virtual void clearForceUnloaded() = 0;
};
