#include <Arduino.h>
#include "automation/timers/RelayTimer.h"

RelayTimer::RelayTimer()
    : _relayId(0U),
      _source(CommandSource::MANUAL),
      _startedAtMs(0U),
      _durationMs(0U),
      _active(false) {
}

void RelayTimer::start(
    uint8_t relayId,
    uint32_t durationMs,
    CommandSource source
) {
    bool relayIdIsValid = (relayId > 0U);
    bool durationIsValid = (durationMs > 0U);

    if (relayIdIsValid && durationIsValid) {
        _relayId = relayId;
        _source = source;

        _startedAtMs = millis();
        _durationMs = durationMs;

        _active = true;
    }
}

void RelayTimer::cancel() {
    _relayId = 0U;
    _source = CommandSource::MANUAL;

    _startedAtMs = 0U;
    _durationMs = 0U;

    _active = false;
}

bool RelayTimer::isExpired(uint32_t nowMs) const {
    bool timerExpired = false;

    if (_active) {
        uint32_t elapsedTimeMs = nowMs - _startedAtMs;

        if (elapsedTimeMs >= _durationMs) {
            timerExpired = true;
        }
    }

    return timerExpired;
}

bool RelayTimer::isActive() const {
    bool timerIsActive = _active;

    return timerIsActive;
}

bool RelayTimer::matchesRelayId(uint8_t relayId) const {
    bool timerMatchesRelay = false;

    if (_active) {
        timerMatchesRelay = (_relayId == relayId);
    }

    return timerMatchesRelay;
}

uint8_t RelayTimer::getRelayId() const {
    uint8_t relayId = _relayId;

    return relayId;
}

CommandSource RelayTimer::getSource() const {
    CommandSource source = _source;

    return source;
}

