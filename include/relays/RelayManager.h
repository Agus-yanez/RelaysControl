#pragma once

#include <Arduino.h>

#include "Relay.h"
#include "RelayLockReason.h"
#include "RelayState.h"

class RelayCommandProcessor;

class RelayManager {
    friend class RelayCommandProcessor;

private:
    static constexpr uint8_t MAX_RELAYS = 16;

    Relay* _relays[MAX_RELAYS];
    uint8_t _relayCount;

    Relay* findRelayById(uint8_t id);
    Relay* findRelayByPin(uint8_t pin);

    bool turnOn(uint8_t relayId);
    bool turnOff(uint8_t relayId);
    bool setState(uint8_t relayId, RelayState state);

    bool lockRelay(
        uint8_t relayId,
        RelayLockReason reason
    );

    bool unlockRelay(uint8_t relayId);

public:
    RelayManager();

    bool addRelay(Relay* relay);

    void begin();
    void update();

    /*
     * Son operaciones globales de seguridad.
     * No sirven para encender relés ni eludir validaciones.
     */
    void applySafeStateToAll();
    void turnOffAll();

    uint8_t count() const;

    void printStatus() const;
};