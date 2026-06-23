#pragma once

#include <stdint.h>

#include "ActuatorDefinition.h"

class ActuatorCatalog {
private:
    /*
     * No se deben crear instancias de esta clase.
     * Se utiliza solamente con métodos estáticos.
     */
    ActuatorCatalog() = delete;

    static const ActuatorDefinition _definitions[];
    static const uint8_t _definitionCount;

public:
    static const ActuatorDefinition* findByType(ActuatorType type);

    static const ActuatorDefinition* findByStableKey(
        const char* stableKey
    );

    static RelaySafetyConfig getDefaultSafetyConfig(
        ActuatorType type
    );

    static const char* getStableKey(
        ActuatorType type
    );

    static const char* getDisplayName(
        ActuatorType type
    );
};