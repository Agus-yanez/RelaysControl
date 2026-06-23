#pragma once

#include <stdint.h>

/*
 * Acción solicitada sobre un relé.
 */
enum class RelayCommandAction : uint8_t {
    TURN_ON = 0,
    TURN_OFF,
    LOCK,
    UNLOCK
};