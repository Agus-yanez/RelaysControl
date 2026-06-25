#pragma once

#include <stdint.h>

#include "automation/commands/CommandSource.h"

/*
 * Representa una activación temporal asociada a un relé.
 *
 * No controla el relé directamente.
 * TimerManager será quien administre estos objetos.
 */
class RelayTimer {
private:
    uint8_t _relayId;
    CommandSource _source;

    uint32_t _startedAtMs;
    uint32_t _durationMs;

    bool _active;

public:
    RelayTimer();

    void start(
        uint8_t relayId,
        uint32_t durationMs,
        CommandSource source
    );

    void cancel();

    bool isExpired(uint32_t nowMs) const;
    uint32_t getRemainingDurationMs(uint32_t nowMs) const;

    bool isActive() const;
    bool matchesRelayId(uint8_t relayId) const;

    uint8_t getRelayId() const;
    CommandSource getSource() const;
};