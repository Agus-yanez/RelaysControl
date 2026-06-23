#pragma once

#include <stdint.h>

/*
 * Límites globales del subsistema de relés.
 *
 * No representan estados persistibles.
 * Solo definen capacidad interna de arreglos fijos.
 */
namespace RelaySystemLimits {
    constexpr uint8_t MAX_RELAYS = 16U;
    constexpr uint8_t MAX_ACTIVE_TIMERS = MAX_RELAYS;
}