#pragma once

#include <Arduino.h>

#include "ActuatorType.h"
#include "RelayConfig.h"
#include "RelayLockReason.h"
#include "RelayOperationalState.h"
#include "RelayState.h"

class Relay {
private:
    RelayConfig _config;

    RelayState _currentState;
    RelayOperationalState _operationalState;
    RelayLockReason _lockReason;

    uint32_t _stateChangedAtMs;
    bool _initialized;

    uint8_t getPhysicalLevel(RelayState state) const;
    void writePhysicalState(RelayState state);

    bool isConfigurationValid() const;
    bool canReceiveCommands() const;
    bool hasMinOffDurationElapsed() const;
    bool hasExceededMaxOnDuration() const;

public:
    Relay(const RelayConfig& config);

    void begin();
    void update();

    bool turnOn();
    bool turnOff();
    bool setState(RelayState state);

    void applySafeState();

    void lock(RelayLockReason reason);
    bool unlock();

    RelayState getState() const;
    RelayOperationalState getOperationalState() const;
    RelayLockReason getLockReason() const;

    bool isOn() const;
    bool isOff() const;
    bool isEnabled() const;
    bool isLocked() const;
    bool isInitialized() const;

    uint32_t getRemainingMinOffDurationMs() const;

    uint8_t getId() const;
    const char* getName() const;
    uint8_t getPin() const;

    ActuatorType getActuatorType() const;
    RelaySafetyConfig getSafetyConfig() const;

};