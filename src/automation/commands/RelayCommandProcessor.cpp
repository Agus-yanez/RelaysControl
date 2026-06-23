#include <Arduino.h>

#include "automation/commands/RelayCommandProcessor.h"

#include "automation/commands/RelayCommandValidator.h"
#include "automation/timers/RelayTimer.h"
#include "automation/timers/TimerManager.h"
#include "relays/RelayManager.h"

namespace {
    RelayCommandResult createResult(
        const RelayCommand& command,
        CommandResultCode code,
        uint32_t remainingWaitMs
    ) {
        RelayCommandResult result = {
            false,
            code,
            command.relayId,
            remainingWaitMs
        };

        if (code == CommandResultCode::ACCEPTED) {
            result.accepted = true;
        }

        return result;
    }
}

RelayCommandProcessor::RelayCommandProcessor(
    RelayManager& relayManager,
    TimerManager& timerManager
)
    : _relayManager(relayManager),
      _timerManager(timerManager) {
}

RelayCommandResult RelayCommandProcessor::process(
    const RelayCommand& command
) {
    RelayCommandResult result = createResult(
        command,
        CommandResultCode::INVALID_COMMAND,
        0U
    );

    Relay* relay = _relayManager.findRelayById(
        command.relayId
    );

    if (relay == nullptr) {
        result = createResult(
            command,
            CommandResultCode::RELAY_NOT_FOUND,
            0U
        );
    } else {
        result = RelayCommandValidator::validate(
            *relay,
            command,
            _timerManager
        );

        if (result.accepted) {
            bool commandExecuted = false;

            switch (command.action) {
                case RelayCommandAction::TURN_ON: {
                    commandExecuted = _relayManager.turnOn(
                        command.relayId
                    );

                    if (
                        commandExecuted &&
                        command.hasDuration
                    ) {
                        bool timerStarted =
                            _timerManager.startTimer(
                                command.relayId,
                                command.durationMs,
                                command.source
                            );

                        if (!timerStarted) {
                            /*
                             * Nunca permitimos que un relé quede ON
                             * si se solicitó una duración y el timer
                             * no pudo crearse.
                             */
                            _relayManager.turnOff(
                                command.relayId
                            );

                            commandExecuted = false;
                        }
                    }

                    break;
                }

                case RelayCommandAction::TURN_OFF:
                    _timerManager.cancelTimer(
                        command.relayId
                    );

                    commandExecuted = _relayManager.turnOff(
                        command.relayId
                    );

                    break;

                case RelayCommandAction::LOCK:
                    _timerManager.cancelTimer(
                        command.relayId
                    );

                    commandExecuted =
                        _relayManager.lockRelay(
                            command.relayId,
                            command.lockReason
                        );

                    break;

                case RelayCommandAction::UNLOCK:
                    commandExecuted =
                        _relayManager.unlockRelay(
                            command.relayId
                        );

                    break;

                default:
                    commandExecuted = false;
                    break;
            }

            if (!commandExecuted) {
                result = createResult(
                    command,
                    CommandResultCode::COMMAND_EXECUTION_FAILED,
                    0U
                );
            }
        }
    }

    return result;
}

void RelayCommandProcessor::update() {
    _relayManager.update();

    RelayTimer expiredTimer;

    bool expiredTimerFound = true;

    while (expiredTimerFound) {
        expiredTimerFound =
            _timerManager.takeNextExpiredTimer(
                millis(),
                expiredTimer
            );

        if (expiredTimerFound) {
            RelayCommand timerOffCommand = {
                expiredTimer.getRelayId(),
                RelayCommandAction::TURN_OFF,
                CommandSource::TIMER,
                false,
                0U,
                RelayLockReason::NONE
            };

            process(timerOffCommand);
        }
    }
}