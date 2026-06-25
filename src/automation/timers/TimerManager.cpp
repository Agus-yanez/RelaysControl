#include "automation/timers/TimerManager.h"

TimerManager::TimerManager() {
    uint8_t index = 0U;

    while (
        index < RelaySystemLimits::MAX_ACTIVE_TIMERS
    ) {
        _timers[index].cancel();

        index++;
    }
}

int16_t TimerManager::findActiveTimerIndexByRelayId(
    uint8_t relayId
) const {
    int16_t foundIndex = INVALID_TIMER_INDEX;
    uint8_t index = 0U;

    while (
        index < RelaySystemLimits::MAX_ACTIVE_TIMERS &&
        foundIndex == INVALID_TIMER_INDEX
    ) {
        bool timerMatchesRelay =
            _timers[index].matchesRelayId(relayId);

        if (timerMatchesRelay) {
            foundIndex = static_cast<int16_t>(index);
        }

        index++;
    }

    return foundIndex;
}

int16_t TimerManager::findAvailableTimerIndex() const {
    int16_t availableIndex = INVALID_TIMER_INDEX;
    uint8_t index = 0U;

    while (
        index < RelaySystemLimits::MAX_ACTIVE_TIMERS &&
        availableIndex == INVALID_TIMER_INDEX
    ) {
        bool timerIsInactive = !_timers[index].isActive();

        if (timerIsInactive) {
            availableIndex = static_cast<int16_t>(index);
        }

        index++;
    }

    return availableIndex;
}

bool TimerManager::startTimer(
    uint8_t relayId,
    uint32_t durationMs,
    CommandSource source
) {
    bool timerStarted = false;

    bool relayIdIsValid = (relayId > 0U);
    bool durationIsValid = (durationMs > 0U);

    if (relayIdIsValid && durationIsValid) {
        int16_t activeTimerIndex =
            findActiveTimerIndexByRelayId(relayId);

        bool relayHasNoActiveTimer =
            (activeTimerIndex == INVALID_TIMER_INDEX);

        if (relayHasNoActiveTimer) {
            int16_t availableTimerIndex =
                findAvailableTimerIndex();

            bool capacityAvailable =
                (availableTimerIndex != INVALID_TIMER_INDEX);

            if (capacityAvailable) {
                RelayTimer& timer =
                    _timers[availableTimerIndex];

                timer.start(
                    relayId,
                    durationMs,
                    source
                );

                timerStarted = timer.isActive();
            }
        }
    }

    return timerStarted;
}

bool TimerManager::cancelTimer(uint8_t relayId) {
    bool timerCancelled = false;

    int16_t activeTimerIndex =
        findActiveTimerIndexByRelayId(relayId);

    bool activeTimerExists =
        (activeTimerIndex != INVALID_TIMER_INDEX);

    if (activeTimerExists) {
        RelayTimer& timer =
            _timers[activeTimerIndex];

        timer.cancel();

        timerCancelled = true;
    }

    return timerCancelled;
}

bool TimerManager::hasActiveTimer(uint8_t relayId) const {
    int16_t activeTimerIndex =
        findActiveTimerIndexByRelayId(relayId);

    bool activeTimerExists =
        (activeTimerIndex != INVALID_TIMER_INDEX);

    return activeTimerExists;
}

bool TimerManager::takeNextExpiredTimer(
    uint32_t nowMs,
    RelayTimer& expiredTimer
) {
    bool expiredTimerFound = false;
    uint8_t index = 0U;

    while (
        index < RelaySystemLimits::MAX_ACTIVE_TIMERS &&
        !expiredTimerFound
    ) {
        RelayTimer& currentTimer = _timers[index];

        bool timerExpired =
            currentTimer.isExpired(nowMs);

        if (timerExpired) {
            expiredTimer = currentTimer;

            currentTimer.cancel();

            expiredTimerFound = true;
        }

        index++;
    }

    return expiredTimerFound;
}

bool TimerManager::hasAvailableTimerSlot() const {
    int16_t availableTimerIndex =
        findAvailableTimerIndex();

    bool availableTimerSlotExists =
        (availableTimerIndex != INVALID_TIMER_INDEX);

    return availableTimerSlotExists;
}

uint8_t TimerManager::getActiveTimerCount() const {
    uint8_t activeTimerCount = 0U;
    uint8_t timerIndex = 0U;

    while (
        timerIndex <
        RelaySystemLimits::MAX_ACTIVE_TIMERS
    ) {
        bool timerIsActive =
            _timers[timerIndex].isActive();

        if (timerIsActive) {
            activeTimerCount++;
        }

        timerIndex++;
    }

    return activeTimerCount;
}

bool TimerManager::getActiveTimerStatusByIndex(
    uint8_t activeTimerIndex,
    uint32_t nowMs,
    TimerStatus& timerStatus
    ) const {
    timerStatus.active = false;
    timerStatus.relayId = 0U;
    timerStatus.source = CommandSource::MANUAL;
    timerStatus.remainingDurationMs = 0U;

    bool timerStatusFound = false;
    uint8_t timerIndex = 0U;
    uint8_t foundActiveTimerCount = 0U;

    while (
        timerIndex <
        RelaySystemLimits::MAX_ACTIVE_TIMERS &&
        !timerStatusFound
    ) {
        const RelayTimer& currentTimer =
            _timers[timerIndex];

        bool timerIsActive =
            currentTimer.isActive();

        if (timerIsActive) {
            bool currentTimerMatchesRequestedIndex =
                (foundActiveTimerCount ==
                 activeTimerIndex);

            if (currentTimerMatchesRequestedIndex) {
                timerStatus.active = true;
                timerStatus.relayId =
                    currentTimer.getRelayId();

                timerStatus.source =
                    currentTimer.getSource();

                timerStatus.remainingDurationMs =
                    currentTimer
                        .getRemainingDurationMs(nowMs);

                timerStatusFound = true;
            }

            foundActiveTimerCount++;
        }

        timerIndex++;
    }

    return timerStatusFound;
}