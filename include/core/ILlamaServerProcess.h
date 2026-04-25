#pragma once

#include "config.h"

#include <string>

/**
 * @file ILlamaServerProcess.h
 * @brief Thin interface capturing LlamaServerProcess methods used by panels.
 *
 * Panels depend on this interface rather than the singleton directly,
 * enabling unit testing with GMock. The real LlamaServerProcess implements
 * this interface directly (zero indirection overhead).
 */
class ILlamaServerProcess
{
  public:
    virtual ~ILlamaServerProcess() = default;

    /** @brief Check if the process is currently running. */
    virtual bool isRunning() const = 0;

    /** @brief Launch llama-server with the given configuration. @return true on success. */
    virtual bool launch(const std::string &modelPath,
                        const Config::LoadSettings &load,
                        const Config::InferenceSettings &inference,
                        const Config::ServerSettings &server) = 0;

    /** @brief Terminate the running llama-server process. @return true on success. */
    virtual bool terminate() = 0;

    /** @brief Check if a model is currently loaded via API. */
    virtual bool isModelLoaded() = 0;

    /** @brief Get the path of the currently loaded model. */
    virtual std::string getLoadedModelPath() = 0;

    /** @brief Unload the currently loaded model via API. @return true on success. */
    virtual bool unloadModel() = 0;

    /** @brief Load a model via API (hot-swap). @return true on success. */
    virtual bool loadModel(const std::string &modelPath) = 0;

    /** @brief Check if llama-server is responding via HTTP. */
    virtual bool isServerHealthy() = 0;
};
