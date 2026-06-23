#pragma once

#include "automation/commands/RelayCommand.h"
#include "automation/commands/RelayCommandResult.h"

class RelayManager;
class TimerManager;

class RelayCommandProcessor {
private:
    RelayManager& _relayManager;
    TimerManager& _timerManager;

public:
    RelayCommandProcessor(
        RelayManager& relayManager,
        TimerManager& timerManager
    );

    RelayCommandResult process(
        const RelayCommand& command
    );

    void update();
};