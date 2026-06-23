#pragma once

#include <stdint.h>

/*
 * Define las restricciones de seguridad de un actuador.
 */
struct RelaySafetyConfig {
    /*
     * Valor especial:
     *
     * maxOnDurationMs = 0
     * significa que el actuador puede quedar encendido
     * sin límite temporal de seguridad.
     */
    static constexpr uint32_t NO_MAX_ON_DURATION_MS = 0U;

    uint32_t maxOnDurationMs;
    uint32_t minOffDurationMs;

    bool allowContinuousOperation;
    bool allowContinuousManualOn;
    bool requireTimedManualControl;

    bool allowScheduleControl;
    bool allowAutomationControl;

    bool lockWhenMaxOnExceeded;
};