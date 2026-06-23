#include "relays/RelayManager.h"

#include "relays/ActuatorCatalog.h"

namespace {
    const char* getOperationalStateText(
        RelayOperationalState state
    ) {
        const char* stateText = "UNKNOWN";

        switch (state) {
            case RelayOperationalState::NORMAL:
                stateText = "NORMAL";
                break;

            case RelayOperationalState::NOTENABLE:
                stateText = "NOTENABLE";
                break;

            case RelayOperationalState::SAFETY_LOCKED:
                stateText = "SAFETY_LOCKED";
                break;

            case RelayOperationalState::FAULTED:
                stateText = "FAULTED";
                break;

            default:
                stateText = "UNKNOWN";
                break;
        }

        return stateText;
    }

    const char* getLockReasonText(
        RelayLockReason reason
    ) {
        const char* reasonText = "UNKNOWN";

        switch (reason) {
            case RelayLockReason::NONE:
                reasonText = "NONE";
                break;

            case RelayLockReason::MANUAL_LOCK:
                reasonText = "MANUAL_LOCK";
                break;

            case RelayLockReason::MAX_ON_TIME_EXCEEDED:
                reasonText = "MAX_ON_TIME_EXCEEDED";
                break;

            case RelayLockReason::INVALID_CONFIGURATION:
                reasonText = "INVALID_CONFIGURATION";
                break;

            case RelayLockReason::SENSOR_FAULT:
                reasonText = "SENSOR_FAULT";
                break;

            case RelayLockReason::DRY_RUN_RISK:
                reasonText = "DRY_RUN_RISK";
                break;

            case RelayLockReason::EMERGENCY_STOP:
                reasonText = "EMERGENCY_STOP";
                break;

            case RelayLockReason::UNKNOWN:
                reasonText = "UNKNOWN";
                break;

            default:
                reasonText = "UNKNOWN";
                break;
        }

        return reasonText;
    }
}

RelayManager::RelayManager()
    : _relayCount(0) {
    uint8_t index = 0;

    while (index < MAX_RELAYS) {
        _relays[index] = nullptr;
        index++;
    }
}

bool RelayManager::addRelay(Relay* relay) {
    bool relayAdded = false;
    bool relayPointerIsValid = (relay != nullptr);

    if (relayPointerIsValid) {
        bool relayIdIsValid = (relay->getId() > 0);
        bool capacityAvailable = (_relayCount < MAX_RELAYS);

        if (relayIdIsValid && capacityAvailable) {
            Relay* relayWithSameId =
                findRelayById(relay->getId());

            Relay* relayWithSamePin =
                findRelayByPin(relay->getPin());

            bool relayIdAvailable =
                (relayWithSameId == nullptr);

            bool relayPinAvailable =
                (relayWithSamePin == nullptr);

            if (relayIdAvailable && relayPinAvailable) {
                _relays[_relayCount] = relay;
                _relayCount++;

                relayAdded = true;
            }
        }
    }

    return relayAdded;
}

Relay* RelayManager::findRelayById(uint8_t id) {
    Relay* foundRelay = nullptr;
    uint8_t index = 0;

    while (
        index < _relayCount &&
        foundRelay == nullptr
    ) {
        Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            bool relayIdMatches =
                (currentRelay->getId() == id);

            if (relayIdMatches) {
                foundRelay = currentRelay;
            }
        }

        index++;
    }

    return foundRelay;
}

Relay* RelayManager::findRelayByPin(uint8_t pin) {
    Relay* foundRelay = nullptr;
    uint8_t index = 0;

    while (
        index < _relayCount &&
        foundRelay == nullptr
    ) {
        Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            bool relayPinMatches =
                (currentRelay->getPin() == pin);

            if (relayPinMatches) {
                foundRelay = currentRelay;
            }
        }

        index++;
    }

    return foundRelay;
}

void RelayManager::begin() {
    uint8_t index = 0;

    while (index < _relayCount) {
        Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            currentRelay->begin();
        }

        index++;
    }
}

void RelayManager::update() {
    uint8_t index = 0;

    while (index < _relayCount) {
        Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            currentRelay->update();
        }

        index++;
    }
}

bool RelayManager::turnOn(uint8_t relayId) {
    bool commandApplied =
        setState(relayId, RelayState::ON);

    return commandApplied;
}

bool RelayManager::turnOff(uint8_t relayId) {
    bool commandApplied =
        setState(relayId, RelayState::OFF);

    return commandApplied;
}

bool RelayManager::setState(
    uint8_t relayId,
    RelayState state
) {
    bool commandApplied = false;

    Relay* relay = findRelayById(relayId);

    if (relay != nullptr) {
        commandApplied = relay->setState(state);
    }

    return commandApplied;
}

bool RelayManager::lockRelay(
    uint8_t relayId,
    RelayLockReason reason
) {
    bool relayLocked = false;

    Relay* relay = findRelayById(relayId);

    if (relay != nullptr) {
        bool relayIsInitialized = relay->isInitialized();

        if (relayIsInitialized) {
            relay->lock(reason);
            relayLocked = relay->isLocked();
        }
    }

    return relayLocked;
}

bool RelayManager::unlockRelay(uint8_t relayId) {
    bool relayUnlocked = false;

    Relay* relay = findRelayById(relayId);

    if (relay != nullptr) {
        relayUnlocked = relay->unlock();
    }

    return relayUnlocked;
}

void RelayManager::applySafeStateToAll() {
    uint8_t index = 0;

    while (index < _relayCount) {
        Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            currentRelay->applySafeState();
        }

        index++;
    }
}

void RelayManager::turnOffAll() {
    uint8_t index = 0;

    while (index < _relayCount) {
        Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            currentRelay->turnOff();
        }

        index++;
    }
}

uint8_t RelayManager::count() const {
    uint8_t relayCount = _relayCount;

    return relayCount;
}

void RelayManager::printStatus() const {
    uint8_t index = 0;

    Serial.println();
    Serial.println("----- ESTADO DE RELES -----");

    while (index < _relayCount) {
        const Relay* currentRelay = _relays[index];

        if (currentRelay != nullptr) {
            const char* relayStateText =
                currentRelay->isOn() ? "ON" : "OFF";

            const char* enabledText =
                currentRelay->isEnabled() ? "SI" : "NO";

            const char* actuatorName =
                ActuatorCatalog::getDisplayName(
                    currentRelay->getActuatorType()
                );

            const char* operationalStateText =
                getOperationalStateText(
                    currentRelay->getOperationalState()
                );

            const char* lockReasonText =
                getLockReasonText(
                    currentRelay->getLockReason()
                );

            Serial.print("ID: ");
            Serial.print(currentRelay->getId());

            Serial.print(" | Nombre: ");
            Serial.print(currentRelay->getName());

            Serial.print(" | Tipo: ");
            Serial.print(actuatorName);

            Serial.print(" | GPIO: ");
            Serial.print(currentRelay->getPin());

            Serial.print(" | Estado: ");
            Serial.print(relayStateText);

            Serial.print(" | Operacion: ");
            Serial.print(operationalStateText);

            Serial.print(" | Motivo: ");
            Serial.print(lockReasonText);

            Serial.print(" | Habilitado: ");
            Serial.println(enabledText);
        }

        index++;
    }

    Serial.println("---------------------------");
}