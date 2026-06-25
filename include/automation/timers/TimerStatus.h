#pragma once

#include <stdint.h>

#include "automation/commands/CommandSource.h"

struct TimerStatus {
    bool active;
    uint8_t relayId;
    CommandSource source;
    uint32_t remainingDurationMs;
};