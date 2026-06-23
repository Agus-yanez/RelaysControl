#pragma once

#include <stdint.h>

/*
 * Indica la causa por la cual un relé fue bloqueado
 * o marcado como fallido.
 */
enum class RelayLockReason : uint8_t {
    NONE = 0,
    MANUAL_LOCK,
    MAX_ON_TIME_EXCEEDED,
    INVALID_CONFIGURATION,
    SENSOR_FAULT,
    DRY_RUN_RISK,
    EMERGENCY_STOP,
    UNKNOWN
};