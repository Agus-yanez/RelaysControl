#pragma once

#include "automation/commands/RelayCommand.h"
#include "automation/commands/RelayCommandResult.h"

class Relay;
class TimerManager;

/*
 * Clase utilitaria.
 *
 * Solo valida órdenes.
 * No modifica GPIO, relés ni timers.
 */
class RelayCommandValidator {
private:
    RelayCommandValidator() = delete;

public:
    static RelayCommandResult validate(
        const Relay& relay,
        const RelayCommand& command,
        const TimerManager& timerManager
    );
};