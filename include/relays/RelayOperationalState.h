#pragma once

#include <stdint.h>

/*
 * Indica si el relé está autorizado a operar.
 *
 * Es distinto de RelayState:
 *
 * RelayState              -> ON / OFF físico.
 * RelayOperationalState   -> si puede o no aceptar órdenes.
 */
enum class RelayOperationalState : uint8_t {
    NORMAL = 0,
    NOTENABLE,
    SAFETY_LOCKED,
    FAULTED
};