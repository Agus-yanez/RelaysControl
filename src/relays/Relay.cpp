#include "relays/Relay.h"

#include "relays/ActuatorCatalog.h"

Relay::Relay(const RelayConfig& config)
    : _config(config),
      _currentState(config.safeState),
      _operationalState(
          config.enabled
              ? RelayOperationalState::NORMAL
              : RelayOperationalState::NOTENABLE
      ),
      _lockReason(RelayLockReason::NONE),
      _stateChangedAtMs(0U),
      _initialized(false) {
}

uint8_t Relay::getPhysicalLevel(RelayState state) const {
    uint8_t physicalLevel = LOW;
    bool relayShouldBeOn = (state == RelayState::ON);

    if (_config.activeLow) {
        if (relayShouldBeOn) {
            physicalLevel = LOW;
        } else {
            physicalLevel = HIGH;
        }
    } else {
        if (relayShouldBeOn) {
            physicalLevel = HIGH;
        } else {
            physicalLevel = LOW;
        }
    }

    return physicalLevel;
}

void Relay::writePhysicalState(RelayState state) {
    bool stateChanged = (_currentState != state);
    uint8_t physicalLevel = getPhysicalLevel(state);

    digitalWrite(_config.pin, physicalLevel);

    if (stateChanged) {
        _currentState = state;
        _stateChangedAtMs = millis();
    }
}

bool Relay::isConfigurationValid() const {
    bool configurationIsValid = true;

    const ActuatorDefinition* actuatorDefinition =
        ActuatorCatalog::findByType(_config.actuatorType);

    bool actuatorTypeExists = (actuatorDefinition != nullptr);

    if (!actuatorTypeExists) {
        configurationIsValid = false;
    }

    if (_config.safetyConfig.allowContinuousOperation) {
        bool hasNoMaxDuration =
            _config.safetyConfig.maxOnDurationMs ==
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS;

        if (!hasNoMaxDuration) {
            configurationIsValid = false;
        }
    } else {
        bool hasMaxDuration =
            _config.safetyConfig.maxOnDurationMs >
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS;

        if (!hasMaxDuration) {
            configurationIsValid = false;
        }
    }

    bool requiresTimedManualControl =
        _config.safetyConfig.requireTimedManualControl;

    bool allowsContinuousManualOn =
        _config.safetyConfig.allowContinuousManualOn;

    if (
        requiresTimedManualControl &&
        allowsContinuousManualOn
    ) {
        configurationIsValid = false;
    }

    return configurationIsValid;
}

bool Relay::canReceiveCommands() const {
    bool canReceiveCommands = false;

    bool relayIsOperational =
        (_operationalState == RelayOperationalState::NORMAL);

    if (_initialized && relayIsOperational) {
        canReceiveCommands = true;
    }

    return canReceiveCommands;
}

bool Relay::hasMinOffDurationElapsed() const {
    bool minimumOffDurationElapsed = true;

    uint32_t remainingWaitMs =
        getRemainingMinOffDurationMs();

    if (remainingWaitMs > 0U) {
        minimumOffDurationElapsed = false;
    }

    return minimumOffDurationElapsed;
}

bool Relay::hasExceededMaxOnDuration() const {
    bool maxOnDurationExceeded = false;

    bool relayIsOn = (_currentState == RelayState::ON);

    if (
        relayIsOn &&
        !_config.safetyConfig.allowContinuousOperation
    ) {
        uint32_t maxOnDurationMs =
            _config.safetyConfig.maxOnDurationMs;

        if (
            maxOnDurationMs >
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS
        ) {
            uint32_t elapsedOnTimeMs =
                millis() - _stateChangedAtMs;

            if (elapsedOnTimeMs >= maxOnDurationMs) {
                maxOnDurationExceeded = true;
            }
        }
    }

    return maxOnDurationExceeded;
}

void Relay::begin() {
    uint8_t safePhysicalLevel =
        getPhysicalLevel(_config.safeState);

    /*
     * Se prepara el nivel seguro antes de configurar
     * el pin como salida para reducir activaciones accidentales.
     */
    digitalWrite(_config.pin, safePhysicalLevel);
    pinMode(_config.pin, OUTPUT);
    digitalWrite(_config.pin, safePhysicalLevel);

    _currentState = _config.safeState;
    _stateChangedAtMs = millis();
    _initialized = true;

    bool configurationIsValid = isConfigurationValid();

    if (!configurationIsValid) {
        _operationalState = RelayOperationalState::FAULTED;
        _lockReason = RelayLockReason::INVALID_CONFIGURATION;

        writePhysicalState(RelayState::OFF);
    } else if (!_config.enabled) {
        _operationalState = RelayOperationalState::NOTENABLE;
        _lockReason = RelayLockReason::NONE;

        applySafeState();
    } else {
        _operationalState = RelayOperationalState::NORMAL;
        _lockReason = RelayLockReason::NONE;

        applySafeState();
    }
}

void Relay::update() {
    bool relayCanCheckSafety =
        _initialized &&
        _operationalState == RelayOperationalState::NORMAL &&
        _currentState == RelayState::ON;

    if (relayCanCheckSafety) {
        bool maxOnDurationExceeded =
            hasExceededMaxOnDuration();

        if (maxOnDurationExceeded) {
            bool shouldLockRelay =
                _config.safetyConfig.lockWhenMaxOnExceeded;

            if (shouldLockRelay) {
                lock(
                    RelayLockReason::MAX_ON_TIME_EXCEEDED
                );
            } else {
                writePhysicalState(RelayState::OFF);
            }
        }
    }
}

bool Relay::turnOn() {
    bool commandApplied = setState(RelayState::ON);

    return commandApplied;
}

bool Relay::turnOff() {
    bool commandApplied = setState(RelayState::OFF);

    return commandApplied;
}

bool Relay::setState(RelayState state) {
    bool commandApplied = false;

    bool requestedStateIsValid =
        state == RelayState::ON ||
        state == RelayState::OFF;

    if (_initialized && requestedStateIsValid) {
        bool isOffCommand = (state == RelayState::OFF);

        /*
         * Una orden OFF siempre se acepta.
         * Apagar debe seguir siendo posible aunque el relé
         * esté bloqueado, deshabilitado o en falla.
         */
        if (isOffCommand) {
            writePhysicalState(RelayState::OFF);
            commandApplied = true;
        } else {
            bool relayCanReceiveCommands =
                canReceiveCommands();

            bool minOffDurationElapsed =
                hasMinOffDurationElapsed();

            if (
                relayCanReceiveCommands &&
                minOffDurationElapsed
            ) {
                writePhysicalState(RelayState::ON);
                commandApplied = true;
            }
        }
    }

    return commandApplied;
}

void Relay::applySafeState() {
    if (_initialized) {
        writePhysicalState(_config.safeState);
    }
}

void Relay::lock(RelayLockReason reason) {
    if (_initialized) {
        RelayLockReason effectiveReason = reason;

        if (effectiveReason == RelayLockReason::NONE) {
            effectiveReason = RelayLockReason::UNKNOWN;
        }

        bool relayIsFaulted =
            (_operationalState ==
             RelayOperationalState::FAULTED);

        /*
         * Un relé FAULTED conserva su motivo original,
         * pero siempre se fuerza físicamente a OFF.
         */
        if (!relayIsFaulted) {
            _operationalState =
                RelayOperationalState::SAFETY_LOCKED;

            _lockReason = effectiveReason;
        }

        writePhysicalState(RelayState::OFF);
    }
}

bool Relay::unlock() {
    bool relayUnlocked = false;

    bool relayCanBeUnlocked =
        _initialized &&
        _config.enabled &&
        isConfigurationValid();

    bool relayIsLockedOrFaulted =
        _operationalState ==
            RelayOperationalState::SAFETY_LOCKED ||
        _operationalState ==
            RelayOperationalState::FAULTED;

    bool invalidConfigurationLock =
        (_lockReason ==
         RelayLockReason::INVALID_CONFIGURATION);

    if (
        relayCanBeUnlocked &&
        relayIsLockedOrFaulted &&
        !invalidConfigurationLock
    ) {
        _operationalState = RelayOperationalState::NORMAL;
        _lockReason = RelayLockReason::NONE;

        /*
         * Desbloquear nunca vuelve a encender el relé.
         */
        writePhysicalState(RelayState::OFF);

        relayUnlocked = true;
    }

    return relayUnlocked;
}

RelayState Relay::getState() const {
    RelayState currentState = _currentState;

    return currentState;
}

RelayOperationalState Relay::getOperationalState() const {
    RelayOperationalState operationalState =
        _operationalState;

    return operationalState;
}

RelayLockReason Relay::getLockReason() const {
    RelayLockReason lockReason = _lockReason;

    return lockReason;
}

bool Relay::isOn() const {
    bool relayIsOn = (_currentState == RelayState::ON);

    return relayIsOn;
}

bool Relay::isOff() const {
    bool relayIsOff = (_currentState == RelayState::OFF);

    return relayIsOff;
}

bool Relay::isEnabled() const {
    bool relayIsEnabled = _config.enabled;

    return relayIsEnabled;
}

bool Relay::isLocked() const {
    bool relayIsLocked =
        _operationalState ==
            RelayOperationalState::SAFETY_LOCKED ||
        _operationalState ==
            RelayOperationalState::FAULTED;

    return relayIsLocked;
}

bool Relay::isInitialized() const {
    bool relayIsInitialized = _initialized;

    return relayIsInitialized;
}

uint8_t Relay::getId() const {
    uint8_t relayId = _config.id;

    return relayId;
}

const char* Relay::getName() const {
    const char* relayName = _config.name;

    return relayName;
}

uint8_t Relay::getPin() const {
    uint8_t relayPin = _config.pin;

    return relayPin;
}

ActuatorType Relay::getActuatorType() const {
    ActuatorType actuatorType = _config.actuatorType;

    return actuatorType;
}

RelaySafetyConfig Relay::getSafetyConfig() const {
    RelaySafetyConfig safetyConfig = _config.safetyConfig;

    return safetyConfig;
}

uint32_t Relay::getRemainingMinOffDurationMs() const {
    uint32_t remainingWaitMs = 0U;

    bool relayIsOff = (_currentState == RelayState::OFF);

    if (relayIsOff) {
        uint32_t minimumOffDurationMs =
            _config.safetyConfig.minOffDurationMs;

        if (minimumOffDurationMs > 0U) {
            uint32_t elapsedOffTimeMs =
                millis() - _stateChangedAtMs;

            if (elapsedOffTimeMs < minimumOffDurationMs) {
                remainingWaitMs =
                    minimumOffDurationMs - elapsedOffTimeMs;
            }
        }
    }

    return remainingWaitMs;
}