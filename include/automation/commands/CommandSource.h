#pragma once

#include <stdint.h>

/*
 * Origen de una orden.
 *
 * Más adelante servirá para aplicar permisos distintos
 * según sea una orden manual, de horario, automatización, etc.
 */
enum class CommandSource : uint8_t {
    MANUAL = 0,
    SCHEDULE,
    AUTOMATION,
    TIMER,
    SAFETY,
    EMERGENCY,
    SYSTEM_RECOVERY
};