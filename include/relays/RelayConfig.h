#pragma once

#include <stdint.h>
#include "ActuatorType.h"
#include "RelaySafetyConfig.h"
#include "RelayState.h"

struct RelayConfig {
    static constexpr uint8_t NAME_MAX_LENGTH = 32;

    uint8_t id;
    char name[NAME_MAX_LENGTH];

    uint8_t pin;
    bool activeLow;

    RelayState safeState;
    bool enabled;
    
    ActuatorType actuatorType;
    RelaySafetyConfig safetyConfig;
};