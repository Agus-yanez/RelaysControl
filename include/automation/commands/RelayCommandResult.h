#pragma once

#include <stdint.h>

#include "automation/commands/CommandResultCode.h"

/*
 * Resultado de validar o procesar una orden.
 */
struct RelayCommandResult {
    bool accepted;

    CommandResultCode code;
    uint8_t relayId;

    /*
     * Se usará principalmente si se rechaza una orden ON
     * porque todavía no se cumplió minOffDurationMs.
     */
    uint32_t remainingWaitMs;
};