#pragma once

#include "IModelInfoMonitor.h"

#include <gmock/gmock.h>

class MockModelInfoMonitor : public IModelInfoMonitor
{
  public:
    MOCK_CONST_METHOD0(getStats, ModelInfo());
    MOCK_METHOD0(setUnloaded, void());
    MOCK_METHOD0(clearForceUnloaded, void());
};
