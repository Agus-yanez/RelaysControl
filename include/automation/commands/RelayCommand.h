#pragma once

#include <stdint.h>

#include "automation/commands/CommandSource.h"
#include "automation/commands/RelayCommandAction.h"
#include "relays/RelayLockReason.h"

/*
 * Representa una intención de control.
 *
 * No controla GPIO.
 * No busca relés.
 * No valida reglas.
 */
struct RelayCommand {
    uint8_t relayId;

    RelayCommandAction action;
    CommandSource source;

    bool hasDuration;
    uint32_t durationMs;

    RelayLockReason lockReason;
};