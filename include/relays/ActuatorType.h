#pragma once

#include <stdint.h>

/*
 * Representa qué tipo de carga controla un Relay.
 *
 * IMPORTANTE:
 * El sistema no dependerá del orden numérico de estos valores.
 * COUNT se utiliza solo como referencia interna y nunca debe persistirse.
 */
enum class ActuatorType : uint8_t {
    GENERIC = 0,

    WATER_PUMP,
    DOSING_PUMP,
    DRAIN_PUMP,
    AIR_PUMP,

    EXTRACTOR,
    INTRACTOR,
    FAN,

    LIGHT,
    HUMIDIFIER,
    SOLENOID_VALVE,
    HEATER,

    COUNT
};