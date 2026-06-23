#pragma once

#include "ActuatorType.h"
#include "RelaySafetyConfig.h"

/*
 * Es la definición central de un tipo de actuador.
 *
 * stableKey:
 * - Se usará en persistencia futura.
 * - No depende del orden del enum.
 * - Ejemplo: "WATER_PUMP".
 *
 * displayName:
 * - Nombre visible para monitor serial, dashboard o API.
 */
struct ActuatorDefinition {
    ActuatorType type;

    const char* stableKey;
    const char* displayName;

    RelaySafetyConfig defaultSafetyConfig;
};