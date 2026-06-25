#pragma once

#include <stdint.h>

#include "automation/commands/CommandSource.h"
#include "automation/timers/RelayTimer.h"
#include "core/RelaySystemLimits.h"
#include "automation/timers/TimerStatus.h"

/*
 * Administra timers temporales.
 *
 * Regla:
 * - Solo puede existir un timer activo por relé.
 * - Un timer vencido es entregado al consumidor y luego cancelado.
 */
class TimerManager {
private:
    static constexpr int16_t INVALID_TIMER_INDEX = -1;

    RelayTimer _timers[
        RelaySystemLimits::MAX_ACTIVE_TIMERS
    ];

    int16_t findActiveTimerIndexByRelayId(
        uint8_t relayId
    ) const;

    int16_t findAvailableTimerIndex() const;

public:
    TimerManager();

    bool startTimer(
        uint8_t relayId,
        uint32_t durationMs,
        CommandSource source
    );

    bool cancelTimer(uint8_t relayId);

    bool hasActiveTimer(uint8_t relayId) const;
    bool hasAvailableTimerSlot() const;

    uint8_t getActiveTimerCount() const;

    bool getActiveTimerStatusByIndex(
        uint8_t activeTimerIndex,
        uint32_t nowMs,
        TimerStatus& timerStatus
    ) const;

    bool takeNextExpiredTimer(
        uint32_t nowMs,
        RelayTimer& expiredTimer
    );
};