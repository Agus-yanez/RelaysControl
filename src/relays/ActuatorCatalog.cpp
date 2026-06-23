#include "relays/ActuatorCatalog.h"

#include <cstring>

namespace {
    const char UNKNOWN_STABLE_KEY[] = "UNKNOWN";
    const char UNKNOWN_DISPLAY_NAME[] = "Desconocido";

    /*
     * Configuración de respaldo.
     *
     * Solo se utiliza si se consulta un tipo inválido y,
     * por algún motivo inesperado, tampoco se encuentra GENERIC.
     *
     * Es conservadora:
     * - máximo 5 minutos ON;
     * - mínimo 1 minuto OFF;
     * - no permite operación continua;
     * - no permite schedule ni automatización;
     * - obliga a control manual temporizado.
     */
    const RelaySafetyConfig FALLBACK_SAFETY_CONFIG = {
        300000UL,
        60000UL,

        false,
        false,
        true,

        false,
        false,

        true
    };
}

/*
 * Catálogo centralizado.
 *
 * Los valores no se buscan por índice del enum.
 * Cada búsqueda compara explícitamente ActuatorType.
 */
const ActuatorDefinition ActuatorCatalog::_definitions[] = {
    {
        ActuatorType::GENERIC,
        "GENERIC",
        "Generico",
        {
            300000UL,
            60000UL,

            false,
            false,
            true,

            false,
            false,

            true
        }
    },

    {
        ActuatorType::WATER_PUMP,
        "WATER_PUMP",
        "Bomba de agua",
        {
            600000UL,
            60000UL,

            false,
            false,
            true,

            true,
            true,

            true
        }
    },

    {
        ActuatorType::DOSING_PUMP,
        "DOSING_PUMP",
        "Bomba dosificadora",
        {
            30000UL,
            600000UL,

            false,
            false,
            true,

            true,
            true,

            true
        }
    },

    {
        ActuatorType::DRAIN_PUMP,
        "DRAIN_PUMP",
        "Bomba de drenaje",
        {
            600000UL,
            60000UL,

            false,
            false,
            true,

            true,
            true,

            true
        }
    },

    {
        ActuatorType::AIR_PUMP,
        "AIR_PUMP",
        "Bomba de aire",
        {
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS,
            0U,

            true,
            true,
            false,

            true,
            true,

            false
        }
    },

    {
        ActuatorType::EXTRACTOR,
        "EXTRACTOR",
        "Extractor",
        {
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS,
            0U,

            true,
            true,
            false,

            true,
            true,

            false
        }
    },

    {
        ActuatorType::INTRACTOR,
        "INTRACTOR",
        "Intractor",
        {
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS,
            0U,

            true,
            true,
            false,

            true,
            true,

            false
        }
    },

    {
        ActuatorType::FAN,
        "FAN",
        "Ventilador",
        {
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS,
            0U,

            true,
            true,
            false,

            true,
            true,

            false
        }
    },

    {
        ActuatorType::LIGHT,
        "LIGHT",
        "Luz",
        {
            RelaySafetyConfig::NO_MAX_ON_DURATION_MS,
            0U,

            true,
            true,
            false,

            true,
            true,

            false
        }
    },

    {
        ActuatorType::HUMIDIFIER,
        "HUMIDIFIER",
        "Humidificador",
        {
            1800000UL,
            300000UL,

            false,
            false,
            true,

            true,
            true,

            true
        }
    },

    {
        ActuatorType::SOLENOID_VALVE,
        "SOLENOID_VALVE",
        "Electrovalvula",
        {
            600000UL,
            60000UL,

            false,
            false,
            true,

            true,
            true,

            true
        }
    },

    {
        ActuatorType::HEATER,
        "HEATER",
        "Calefactor",
        {
            1800000UL,
            300000UL,

            false,
            false,
            true,

            true,
            true,

            true
        }
    }
};

const uint8_t ActuatorCatalog::_definitionCount =
    static_cast<uint8_t>(
        sizeof(ActuatorCatalog::_definitions) /
        sizeof(ActuatorCatalog::_definitions[0])
    );

const ActuatorDefinition* ActuatorCatalog::findByType(
    ActuatorType type
) {
    const ActuatorDefinition* foundDefinition = nullptr;
    uint8_t index = 0;

    while (
        index < _definitionCount &&
        foundDefinition == nullptr
    ) {
        const ActuatorDefinition& currentDefinition =
            _definitions[index];

        bool typesMatch =
            (currentDefinition.type == type);

        if (typesMatch) {
            foundDefinition = &currentDefinition;
        }

        index++;
    }

    return foundDefinition;
}

const ActuatorDefinition* ActuatorCatalog::findByStableKey(
    const char* stableKey
) {
    const ActuatorDefinition* foundDefinition = nullptr;
    uint8_t index = 0;

    bool stableKeyIsValid = (stableKey != nullptr);

    if (stableKeyIsValid) {
        while (
            index < _definitionCount &&
            foundDefinition == nullptr
        ) {
            const ActuatorDefinition& currentDefinition =
                _definitions[index];

            bool keysMatch =
                (strcmp(currentDefinition.stableKey, stableKey) == 0);

            if (keysMatch) {
                foundDefinition = &currentDefinition;
            }

            index++;
        }
    }

    return foundDefinition;
}

RelaySafetyConfig ActuatorCatalog::getDefaultSafetyConfig(
    ActuatorType type
) {
    RelaySafetyConfig safetyConfig = FALLBACK_SAFETY_CONFIG;

    const ActuatorDefinition* definition =
        findByType(type);

    if (definition == nullptr) {
        definition = findByType(ActuatorType::GENERIC);
    }

    if (definition != nullptr) {
        safetyConfig = definition->defaultSafetyConfig;
    }

    return safetyConfig;
}

const char* ActuatorCatalog::getStableKey(
    ActuatorType type
) {
    const char* stableKey = UNKNOWN_STABLE_KEY;

    const ActuatorDefinition* definition =
        findByType(type);

    if (definition != nullptr) {
        stableKey = definition->stableKey;
    }

    return stableKey;
}

const char* ActuatorCatalog::getDisplayName(
    ActuatorType type
) {
    const char* displayName = UNKNOWN_DISPLAY_NAME;

    const ActuatorDefinition* definition =
        findByType(type);

    if (definition != nullptr) {
        displayName = definition->displayName;
    }

    return displayName;
}