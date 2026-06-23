#include "automation/commands/RelayCommandValidator.h"

#include "automation/timers/TimerManager.h"
#include "relays/Relay.h"

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

    bool isKnownAction(RelayCommandAction action) {
        bool actionIsKnown = false;

        switch (action) {
            case RelayCommandAction::TURN_ON:
            case RelayCommandAction::TURN_OFF:
            case RelayCommandAction::LOCK:
            case RelayCommandAction::UNLOCK:
                actionIsKnown = true;
                break;

            default:
                actionIsKnown = false;
                break;
        }

        return actionIsKnown;
    }

    bool isKnownSource(CommandSource source) {
        bool sourceIsKnown = false;

        switch (source) {
            case CommandSource::MANUAL:
            case CommandSource::SCHEDULE:
            case CommandSource::AUTOMATION:
            case CommandSource::TIMER:
            case CommandSource::SAFETY:
            case CommandSource::EMERGENCY:
            case CommandSource::SYSTEM_RECOVERY:
                sourceIsKnown = true;
                break;

            default:
                sourceIsKnown = false;
                break;
        }

        return sourceIsKnown;
    }

    bool hasUsableLockReason(RelayLockReason reason) {
        bool lockReasonIsUsable = false;

        switch (reason) {
            case RelayLockReason::MANUAL_LOCK:
            case RelayLockReason::MAX_ON_TIME_EXCEEDED:
            case RelayLockReason::INVALID_CONFIGURATION:
            case RelayLockReason::SENSOR_FAULT:
            case RelayLockReason::DRY_RUN_RISK:
            case RelayLockReason::EMERGENCY_STOP:
            case RelayLockReason::UNKNOWN:
                lockReasonIsUsable = true;
                break;

            case RelayLockReason::NONE:
            default:
                lockReasonIsUsable = false;
                break;
        }

        return lockReasonIsUsable;
    }

    CommandResultCode getDurationValidationCode(
        const RelayCommand& command
    ) {
        CommandResultCode resultCode =
            CommandResultCode::ACCEPTED;

        bool actionIsTurnOn =
            (command.action == RelayCommandAction::TURN_ON);

        if (actionIsTurnOn) {
            if (command.hasDuration) {
                if (command.durationMs == 0U) {
                    resultCode =
                        CommandResultCode::INVALID_DURATION;
                }
            } else {
                if (command.durationMs != 0U) {
                    resultCode =
                        CommandResultCode::INVALID_DURATION;
                }
            }
        } else {
            bool commandContainsDuration =
                command.hasDuration ||
                command.durationMs != 0U;

            if (commandContainsDuration) {
                resultCode =
                    CommandResultCode::INVALID_DURATION;
            }
        }

        return resultCode;
    }

    CommandResultCode getOperationalStateCode(
        RelayOperationalState operationalState
    ) {
        CommandResultCode resultCode =
            CommandResultCode::INVALID_COMMAND;

        switch (operationalState) {
            case RelayOperationalState::NORMAL:
                resultCode = CommandResultCode::ACCEPTED;
                break;

            case RelayOperationalState::NOTENABLE:
                resultCode = CommandResultCode::RELAY_DISABLED;
                break;

            case RelayOperationalState::SAFETY_LOCKED:
                resultCode =
                    CommandResultCode::RELAY_SAFETY_LOCKED;
                break;

            case RelayOperationalState::FAULTED:
                resultCode = CommandResultCode::RELAY_FAULTED;
                break;

            default:
                resultCode = CommandResultCode::INVALID_COMMAND;
                break;
        }

        return resultCode;
    }

    bool isTurnOnSourceAllowed(
        CommandSource source,
        const RelaySafetyConfig& safetyConfig,
        CommandResultCode& rejectionCode
    ) {
        bool sourceAllowed = false;

        rejectionCode =
            CommandResultCode::CONTROL_SOURCE_NOT_ALLOWED;

        switch (source) {
            case CommandSource::MANUAL:
                sourceAllowed = true;
                break;

            case CommandSource::SCHEDULE:
                if (safetyConfig.allowScheduleControl) {
                    sourceAllowed = true;
                } else {
                    rejectionCode =
                        CommandResultCode::
                            SCHEDULE_CONTROL_NOT_ALLOWED;
                }
                break;

            case CommandSource::AUTOMATION:
                if (safetyConfig.allowAutomationControl) {
                    sourceAllowed = true;
                } else {
                    rejectionCode =
                        CommandResultCode::
                            AUTOMATION_CONTROL_NOT_ALLOWED;
                }
                break;

            case CommandSource::SYSTEM_RECOVERY:
                if (
                    safetyConfig.allowScheduleControl ||
                    safetyConfig.allowAutomationControl
                ) {
                    sourceAllowed = true;
                }
                break;

            case CommandSource::TIMER:
            case CommandSource::SAFETY:
            case CommandSource::EMERGENCY:
            default:
                sourceAllowed = false;
                break;
        }

        return sourceAllowed;
    }

    bool isLockSourceAllowed(CommandSource source) {
        bool sourceAllowed = false;

        switch (source) {
            case CommandSource::MANUAL:
            case CommandSource::SAFETY:
            case CommandSource::EMERGENCY:
                sourceAllowed = true;
                break;

            default:
                sourceAllowed = false;
                break;
        }

        return sourceAllowed;
    }

    bool isUnlockSourceAllowed(CommandSource source) {
        bool sourceAllowed = false;

        if (source == CommandSource::MANUAL) {
            sourceAllowed = true;
        }

        return sourceAllowed;
    }
}

RelayCommandResult RelayCommandValidator::validate(
    const Relay& relay,
    const RelayCommand& command,
    const TimerManager& timerManager
) {
    RelayCommandResult result = createResult(
        command,
        CommandResultCode::INVALID_COMMAND,
        0U
    );

    bool relayMatchesCommand =
        (relay.getId() == command.relayId);

    bool actionIsKnown =
        isKnownAction(command.action);

    bool sourceIsKnown =
        isKnownSource(command.source);

    CommandResultCode durationValidationCode =
        getDurationValidationCode(command);

    bool durationIsValid =
        (durationValidationCode ==
         CommandResultCode::ACCEPTED);

    bool basicValidationPassed =
        relayMatchesCommand &&
        actionIsKnown &&
        sourceIsKnown &&
        durationIsValid;

    if (!relayMatchesCommand) {
        result = createResult(
            command,
            CommandResultCode::RELAY_NOT_FOUND,
            0U
        );
    } else if (!actionIsKnown || !sourceIsKnown) {
        result = createResult(
            command,
            CommandResultCode::INVALID_COMMAND,
            0U
        );
    } else if (!durationIsValid) {
        result = createResult(
            command,
            durationValidationCode,
            0U
        );
    } else if (basicValidationPassed) {
        if (!relay.isInitialized()) {
            result = createResult(
                command,
                CommandResultCode::RELAY_NOT_INITIALIZED,
                0U
            );
        } else {
            RelayOperationalState operationalState =
                relay.getOperationalState();

            RelaySafetyConfig safetyConfig =
                relay.getSafetyConfig();

            switch (command.action) {
                case RelayCommandAction::TURN_ON: {
                    CommandResultCode stateCode =
                        getOperationalStateCode(
                            operationalState
                        );

                    if (
                        stateCode !=
                        CommandResultCode::ACCEPTED
                    ) {
                        result = createResult(
                            command,
                            stateCode,
                            0U
                        );
                    } else {
                        CommandResultCode sourceRejectionCode =
                            CommandResultCode::
                                CONTROL_SOURCE_NOT_ALLOWED;

                        bool sourceAllowed =
                            isTurnOnSourceAllowed(
                                command.source,
                                safetyConfig,
                                sourceRejectionCode
                            );

                        if (!sourceAllowed) {
                            result = createResult(
                                command,
                                sourceRejectionCode,
                                0U
                            );
                        } else {
                            bool manualCommand =
                                (command.source ==
                                 CommandSource::MANUAL);

                            bool manualDurationRequired =
                                safetyConfig
                                    .requireTimedManualControl ||
                                !safetyConfig
                                    .allowContinuousManualOn;

                            bool requiresDurationButMissing =
                                manualCommand &&
                                manualDurationRequired &&
                                !command.hasDuration;

                            if (requiresDurationButMissing) {
                                result = createResult(
                                    command,
                                    CommandResultCode::
                                        TIMED_COMMAND_REQUIRED,
                                    0U
                                );
                            } else {
                                bool activeTimerExists =
                                    timerManager.hasActiveTimer(
                                        command.relayId
                                    );

                                if (activeTimerExists) {
                                    result = createResult(
                                        command,
                                        CommandResultCode::
                                            ACTIVE_TIMER_EXISTS,
                                        0U
                                    );
                                } else {
                                    bool requiresTimerSlot =
                                        command.hasDuration;

                                    bool timerCapacityAvailable =
                                        timerManager
                                            .hasAvailableTimerSlot();

                                    bool timerCapacityUnavailable =
                                        requiresTimerSlot &&
                                        !timerCapacityAvailable;

                                    if (
                                        timerCapacityUnavailable
                                    ) {
                                        result = createResult(
                                            command,
                                            CommandResultCode::
                                                TIMER_CAPACITY_REACHED,
                                            0U
                                        );
                                    } else {
                                        bool exceedsMaximumDuration =
                                            false;

                                        if (
                                            command.hasDuration &&
                                            !safetyConfig
                                                .allowContinuousOperation
                                        ) {
                                            if (
                                                command.durationMs >
                                                safetyConfig
                                                    .maxOnDurationMs
                                            ) {
                                                exceedsMaximumDuration =
                                                    true;
                                            }
                                        }

                                        if (
                                            exceedsMaximumDuration
                                        ) {
                                            result = createResult(
                                                command,
                                                CommandResultCode::
                                                    DURATION_EXCEEDS_MAXIMUM,
                                                0U
                                            );
                                        } else {
                                            uint32_t remainingWaitMs =
                                                relay
                                                    .getRemainingMinOffDurationMs();

                                            if (
                                                remainingWaitMs > 0U
                                            ) {
                                                result = createResult(
                                                    command,
                                                    CommandResultCode::
                                                        MIN_OFF_DURATION_NOT_ELAPSED,
                                                    remainingWaitMs
                                                );
                                            } else {
                                                result = createResult(
                                                    command,
                                                    CommandResultCode::
                                                        ACCEPTED,
                                                    0U
                                                );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                }

                case RelayCommandAction::TURN_OFF:
                    result = createResult(
                        command,
                        CommandResultCode::ACCEPTED,
                        0U
                    );
                    break;

                case RelayCommandAction::LOCK: {
                    bool lockSourceAllowed =
                        isLockSourceAllowed(
                            command.source
                        );

                    bool lockReasonIsUsable =
                        hasUsableLockReason(
                            command.lockReason
                        );

                    if (!lockSourceAllowed) {
                        result = createResult(
                            command,
                            CommandResultCode::
                                CONTROL_SOURCE_NOT_ALLOWED,
                            0U
                        );
                    } else if (!lockReasonIsUsable) {
                        result = createResult(
                            command,
                            CommandResultCode::INVALID_COMMAND,
                            0U
                        );
                    } else {
                        CommandResultCode stateCode =
                            getOperationalStateCode(
                                operationalState
                            );

                        if (
                            stateCode ==
                            CommandResultCode::RELAY_DISABLED
                        ) {
                            result = createResult(
                                command,
                                stateCode,
                                0U
                            );
                        } else if (
                            stateCode ==
                            CommandResultCode::RELAY_FAULTED
                        ) {
                            result = createResult(
                                command,
                                stateCode,
                                0U
                            );
                        } else {
                            result = createResult(
                                command,
                                CommandResultCode::ACCEPTED,
                                0U
                            );
                        }
                    }
                    break;
                }

                case RelayCommandAction::UNLOCK: {
                    bool unlockSourceAllowed =
                        isUnlockSourceAllowed(
                            command.source
                        );

                    if (!unlockSourceAllowed) {
                        result = createResult(
                            command,
                            CommandResultCode::
                                CONTROL_SOURCE_NOT_ALLOWED,
                            0U
                        );
                    } else if (
                        operationalState ==
                        RelayOperationalState::NOTENABLE
                    ) {
                        result = createResult(
                            command,
                            CommandResultCode::RELAY_DISABLED,
                            0U
                        );
                    } else if (
                        operationalState ==
                        RelayOperationalState::NORMAL
                    ) {
                        result = createResult(
                            command,
                            CommandResultCode::INVALID_COMMAND,
                            0U
                        );
                    } else if (
                        operationalState ==
                        RelayOperationalState::FAULTED &&
                        relay.getLockReason() ==
                        RelayLockReason::INVALID_CONFIGURATION
                    ) {
                        result = createResult(
                            command,
                            CommandResultCode::RELAY_FAULTED,
                            0U
                        );
                    } else {
                        result = createResult(
                            command,
                            CommandResultCode::ACCEPTED,
                            0U
                        );
                    }
                    break;
                }

                default:
                    result = createResult(
                        command,
                        CommandResultCode::INVALID_COMMAND,
                        0U
                    );
                    break;
            }
        }
    }

    return result;
}