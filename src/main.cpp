#include <Arduino.h>
#include <cstring>
#include <cstdlib>

#include "relays/RelayConfig.h"
#include "relays/Relay.h"
#include "relays/RelayManager.h"
#include "relays/ActuatorCatalog.h"
#include "automation/timers/TimerManager.h"
#include "automation/commands/RelayCommandValidator.h"
#include "automation/commands/RelayCommandProcessor.h"

constexpr uint8_t WATER_PUMP_RELAY_PIN = 16;
constexpr uint8_t EXTRACTOR_RELAY_PIN = 17;
constexpr uint8_t LIGHT_RELAY_PIN = 18;
constexpr uint8_t HUMIDIFIER_RELAY_PIN = 19;

RelayManager relayManager;
TimerManager timerManager;
RelayCommandProcessor commandProcessor(relayManager,timerManager);

constexpr bool ENABLE_TIMER_MANAGER_TEST = false;
constexpr bool ENABLE_VALIDATOR_TEST = false;

constexpr uint32_t PRIMARY_TIMER_DURATION_MS = 5000U;
constexpr uint32_t SECONDARY_TIMER_DURATION_MS = 8000U;

const RelaySafetyConfig waterPumpSafetyConfig =
    ActuatorCatalog::getDefaultSafetyConfig(
        ActuatorType::WATER_PUMP
    );

const RelaySafetyConfig extractorSafetyConfig =
    ActuatorCatalog::getDefaultSafetyConfig(
        ActuatorType::EXTRACTOR
    );

const RelaySafetyConfig lightSafetyConfig =
    ActuatorCatalog::getDefaultSafetyConfig(
        ActuatorType::LIGHT
    );

const RelaySafetyConfig humidifierSafetyConfig =
    ActuatorCatalog::getDefaultSafetyConfig(
        ActuatorType::HUMIDIFIER
    );

RelayConfig waterPumpConfig = {
    1,
    "Bomba agua",
    16,
    true,
    RelayState::OFF,
    true,
    ActuatorType::WATER_PUMP,
    waterPumpSafetyConfig
};

RelayConfig extractorConfig = {
    2,
    "Extractor",
    17,
    true,
    RelayState::OFF,
    true,
    ActuatorType::EXTRACTOR,
    extractorSafetyConfig
};

RelayConfig lightConfig = {
    3,
    "Luz",
    18,
    true,
    RelayState::OFF,
    true,
    ActuatorType::LIGHT,
    lightSafetyConfig
};

RelayConfig humidifierConfig = {
    4,
    "Humidificador",
    19,
    true,
    RelayState::OFF,
    true,
    ActuatorType::HUMIDIFIER,
    humidifierSafetyConfig
};

Relay waterPumpRelay(waterPumpConfig);
Relay extractorRelay(extractorConfig);
Relay lightRelay(lightConfig);
Relay humidifierRelay(humidifierConfig);

const char* getCommandResultCodeText(
    CommandResultCode code
) {
    const char* codeText = "UNKNOWN";

    switch (code) {
        case CommandResultCode::ACCEPTED:
            codeText = "ACCEPTED";
            break;

        case CommandResultCode::RELAY_NOT_FOUND:
            codeText = "RELAY_NOT_FOUND";
            break;

        case CommandResultCode::RELAY_NOT_INITIALIZED:
            codeText = "RELAY_NOT_INITIALIZED";
            break;

        case CommandResultCode::RELAY_DISABLED:
            codeText = "RELAY_DISABLED";
            break;

        case CommandResultCode::RELAY_SAFETY_LOCKED:
            codeText = "RELAY_SAFETY_LOCKED";
            break;

        case CommandResultCode::RELAY_FAULTED:
            codeText = "RELAY_FAULTED";
            break;

        case CommandResultCode::INVALID_COMMAND:
            codeText = "INVALID_COMMAND";
            break;

        case CommandResultCode::CONTROL_SOURCE_NOT_ALLOWED:
            codeText = "CONTROL_SOURCE_NOT_ALLOWED";
            break;

        case CommandResultCode::INVALID_DURATION:
            codeText = "INVALID_DURATION";
            break;

        case CommandResultCode::TIMED_COMMAND_REQUIRED:
            codeText = "TIMED_COMMAND_REQUIRED";
            break;

        case CommandResultCode::DURATION_EXCEEDS_MAXIMUM:
            codeText = "DURATION_EXCEEDS_MAXIMUM";
            break;

        case CommandResultCode::MIN_OFF_DURATION_NOT_ELAPSED:
            codeText = "MIN_OFF_DURATION_NOT_ELAPSED";
            break;

        case CommandResultCode::SCHEDULE_CONTROL_NOT_ALLOWED:
            codeText = "SCHEDULE_CONTROL_NOT_ALLOWED";
            break;

        case CommandResultCode::AUTOMATION_CONTROL_NOT_ALLOWED:
            codeText = "AUTOMATION_CONTROL_NOT_ALLOWED";
            break;

        case CommandResultCode::ACTIVE_TIMER_EXISTS:
            codeText = "ACTIVE_TIMER_EXISTS";
            break;

        case CommandResultCode::TIMER_CAPACITY_REACHED:
            codeText = "TIMER_CAPACITY_REACHED";
            break;

        case CommandResultCode::COMMAND_EXECUTION_FAILED:
            codeText = "COMMAND_EXECUTION_FAILED";
            break;

        default:
            codeText = "UNKNOWN";
            break;
    }

    return codeText;
}

RelayCommand createCommand(
    uint8_t relayId,
    RelayCommandAction action,
    CommandSource source,
    bool hasDuration,
    uint32_t durationMs,
    RelayLockReason lockReason
) {
    RelayCommand command = {
        relayId,
        action,
        source,
        hasDuration,
        durationMs,
        lockReason
    };

    return command;
}

void printValidationResult(
    const char* testName,
    const RelayCommandResult& result
) {
    Serial.println();
    Serial.print("Prueba: ");
    Serial.println(testName);

    Serial.print("Aceptado: ");
    Serial.println(result.accepted ? "SI" : "NO");

    Serial.print("Codigo: ");
    Serial.println(
        getCommandResultCodeText(result.code)
    );

    Serial.print("Espera restante: ");
    Serial.print(result.remainingWaitMs);
    Serial.println(" ms");
}

void testRelayCommandValidator() {
    RelayCommand lightManualCommand = createCommand(
        3,
        RelayCommandAction::TURN_ON,
        CommandSource::MANUAL,
        false,
        0U,
        RelayLockReason::NONE
    );

    RelayCommand pumpWithoutDurationCommand =
        createCommand(
            1,
            RelayCommandAction::TURN_ON,
            CommandSource::MANUAL,
            false,
            0U,
            RelayLockReason::NONE
        );
    
    RelayCommand pumpZeroDurationCommand =
        createCommand(
            1,
            RelayCommandAction::TURN_ON,
            CommandSource::MANUAL,
            true,
            0U,
            RelayLockReason::NONE
        );

    RelayCommand pumpExceedingDurationCommand =
        createCommand(
            1,
            RelayCommandAction::TURN_ON,
            CommandSource::MANUAL,
            true,
            900000U,
            RelayLockReason::NONE
        );

    RelayCommand pumpValidTimedCommand =
        createCommand(
            1,
            RelayCommandAction::TURN_ON,
            CommandSource::MANUAL,
            true,
            30000U,
            RelayLockReason::NONE
        );

    RelayCommandResult lightManualResult =
        RelayCommandValidator::validate(
            lightRelay,
            lightManualCommand,
            timerManager
        );

    RelayCommandResult pumpWithoutDurationResult =
        RelayCommandValidator::validate(
            waterPumpRelay,
            pumpWithoutDurationCommand,
            timerManager
        );

    RelayCommandResult pumpZeroDurationResult =
        RelayCommandValidator::validate(
            waterPumpRelay,
            pumpZeroDurationCommand,
            timerManager
        );

    RelayCommandResult pumpExceedingDurationResult =
        RelayCommandValidator::validate(
            waterPumpRelay,
            pumpExceedingDurationCommand,
            timerManager
        );

    RelayCommandResult pumpValidTimedResult =
        RelayCommandValidator::validate(
            waterPumpRelay,
            pumpValidTimedCommand,
            timerManager
        );

    bool timerStarted = timerManager.startTimer(
        3,
        30000U,
        CommandSource::MANUAL
    );

    RelayCommand lightWithActiveTimerCommand =
        createCommand(
            3,
            RelayCommandAction::TURN_ON,
            CommandSource::MANUAL,
            false,
            0U,
            RelayLockReason::NONE
        );

    RelayCommandResult lightWithActiveTimerResult =
        RelayCommandValidator::validate(
            lightRelay,
            lightWithActiveTimerCommand,
            timerManager
        );

    bool timerCancelled = timerManager.cancelTimer(3);

    Serial.println();
    Serial.println(
        "----- PRUEBA RELAY COMMAND VALIDATOR -----"
    );

    printValidationResult(
        "Luz ON manual continuo",
        lightManualResult
    );

    printValidationResult(
        "Bomba sin duracion",
        pumpWithoutDurationResult
    );

    printValidationResult(
    "Bomba con duracion cero",
        pumpZeroDurationResult
    );

    printValidationResult(
        "Bomba excede maximo",
        pumpExceedingDurationResult
    );

    printValidationResult(
        "Bomba temporizada al iniciar",
        pumpValidTimedResult
    );

    Serial.print("Timer de prueba creado: ");
    Serial.println(timerStarted ? "SI" : "NO");

    printValidationResult(
        "Luz con timer activo",
        lightWithActiveTimerResult
    );

    Serial.print("Timer de prueba cancelado: ");
    Serial.println(timerCancelled ? "SI" : "NO");

    Serial.println(
        "------------------------------------------"
    );
}

const char* getCommandSourceText(
    CommandSource source
) {
    const char* sourceText = "UNKNOWN";

    switch (source) {
        case CommandSource::MANUAL:
            sourceText = "MANUAL";
            break;

        case CommandSource::SCHEDULE:
            sourceText = "SCHEDULE";
            break;

        case CommandSource::AUTOMATION:
            sourceText = "AUTOMATION";
            break;

        case CommandSource::TIMER:
            sourceText = "TIMER";
            break;

        case CommandSource::SAFETY:
            sourceText = "SAFETY";
            break;

        case CommandSource::EMERGENCY:
            sourceText = "EMERGENCY";
            break;

        case CommandSource::SYSTEM_RECOVERY:
            sourceText = "SYSTEM_RECOVERY";
            break;

        default:
            sourceText = "UNKNOWN";
            break;
    }

    return sourceText;
}

void startTimerManagerTest() {
    bool primaryTimerStarted =
        timerManager.startTimer(
            1,
            PRIMARY_TIMER_DURATION_MS,
            CommandSource::MANUAL
        );

    bool duplicateTimerStarted =
        timerManager.startTimer(
            1,
            PRIMARY_TIMER_DURATION_MS,
            CommandSource::MANUAL
        );

    bool secondaryTimerStarted =
        timerManager.startTimer(
            2,
            SECONDARY_TIMER_DURATION_MS,
            CommandSource::SCHEDULE
        );

    bool secondaryTimerCancelled =
        timerManager.cancelTimer(2);

    Serial.println();
    Serial.println("----- PRUEBA TIMER MANAGER -----");

    Serial.print("Timer relay 1 creado: ");
    Serial.println(primaryTimerStarted ? "SI" : "NO");

    Serial.print("Timer duplicado relay 1 creado: ");
    Serial.println(duplicateTimerStarted ? "SI" : "NO");

    Serial.print("Timer relay 2 creado: ");
    Serial.println(secondaryTimerStarted ? "SI" : "NO");

    Serial.print("Timer relay 2 cancelado: ");
    Serial.println(secondaryTimerCancelled ? "SI" : "NO");

    Serial.println(
        "Esperando vencimiento del timer del relay 1..."
    );

    Serial.println("--------------------------------");
}

void updateTimerManagerTest() {
    RelayTimer expiredTimer;

    bool expiredTimerFound = true;

    while (expiredTimerFound) {
        expiredTimerFound =
            timerManager.takeNextExpiredTimer(
                millis(),
                expiredTimer
            );

        if (expiredTimerFound) {
            Serial.println();
            Serial.println("----- TIMER VENCIDO -----");

            Serial.print("Relay ID: ");
            Serial.println(expiredTimer.getRelayId());

            Serial.print("Origen: ");
            Serial.println(
                getCommandSourceText(
                    expiredTimer.getSource()
                )
            );

            Serial.println("-------------------------");
        }
    }
}

void printActuatorDefinition(
    const ActuatorDefinition* definition
) {
    if (definition != nullptr) {
        const RelaySafetyConfig& safetyConfig =
            definition->defaultSafetyConfig;

        Serial.println();
        Serial.println("----- ACTUADOR -----");

        Serial.print("Clave: ");
        Serial.println(definition->stableKey);

        Serial.print("Nombre: ");
        Serial.println(definition->displayName);

        Serial.print("Maximo ON (ms): ");
        Serial.println(safetyConfig.maxOnDurationMs);

        Serial.print("Minimo OFF (ms): ");
        Serial.println(safetyConfig.minOffDurationMs);

        Serial.print("Operacion continua: ");
        Serial.println(
            safetyConfig.allowContinuousOperation ? "SI" : "NO"
        );

        Serial.print("Manual continuo: ");
        Serial.println(
            safetyConfig.allowContinuousManualOn ? "SI" : "NO"
        );

        Serial.print("Manual temporizado obligatorio: ");
        Serial.println(
            safetyConfig.requireTimedManualControl ? "SI" : "NO"
        );

        Serial.println("---------------------");
    }
}

void testActuatorCatalog() {
    const ActuatorDefinition* waterPumpDefinition =
        ActuatorCatalog::findByType(
            ActuatorType::WATER_PUMP
        );

    const ActuatorDefinition* lightDefinition =
        ActuatorCatalog::findByStableKey(
            "LIGHT"
        );

    printActuatorDefinition(waterPumpDefinition);
    printActuatorDefinition(lightDefinition);
}

constexpr uint8_t SERIAL_COMMAND_BUFFER_SIZE = 32;

char serialCommandBuffer[SERIAL_COMMAND_BUFFER_SIZE];
uint8_t serialCommandLength = 0;
bool serialCommandOverflow = false;

bool parseRelayId(const char* valueText, uint8_t& relayId) {
    bool relayIdIsValid = false;

    char* endPosition = nullptr;
    unsigned long parsedValue = strtoul(valueText, &endPosition, 10);

    bool numericTextIsValid = (endPosition != valueText && *endPosition == '\0');
    bool valueWithinRange = (parsedValue > 0 && parsedValue <= 255);

    if (numericTextIsValid && valueWithinRange) {
        relayId = static_cast<uint8_t>(parsedValue);
        relayIdIsValid = true;
    }

    return relayIdIsValid;
}

bool parseDurationMs(
    const char* valueText,
    uint32_t& durationMs
) {
    bool durationIsValid = false;

    if (valueText != nullptr) {
        char* endPosition = nullptr;

        unsigned long parsedValue = strtoul(
            valueText,
            &endPosition,
            10
        );

        bool numericTextIsValid =
            (endPosition != valueText &&
             *endPosition == '\0');

        if (numericTextIsValid) {
            durationMs = static_cast<uint32_t>(
                parsedValue
            );

            durationIsValid = true;
        }
    }

    return durationIsValid;
}

void printProcessedCommandResult(
    const RelayCommand& command,
    const RelayCommandResult& result
) {
    if (result.accepted) {
        Serial.print("Orden aceptada para rele ");
        Serial.print(command.relayId);

        if (command.action == RelayCommandAction::TURN_ON) {
            Serial.print(": ON");

            if (command.hasDuration) {
                Serial.print(" durante ");
                Serial.print(command.durationMs);
                Serial.print(" ms");
            }
        } else if (
            command.action ==
            RelayCommandAction::TURN_OFF
        ) {
            Serial.print(": OFF");
        } else if (
            command.action ==
            RelayCommandAction::LOCK
        ) {
            Serial.print(": BLOQUEADO");
        } else if (
            command.action ==
            RelayCommandAction::UNLOCK
        ) {
            Serial.print(": DESBLOQUEADO y OFF");
        }

        Serial.println(".");
    } else {
        Serial.print("Orden rechazada para rele ");
        Serial.print(command.relayId);

        Serial.print(". Codigo: ");
        Serial.println(
            getCommandResultCodeText(result.code)
        );

        if (result.remainingWaitMs > 0U) {
            Serial.print("Debe esperar ");
            Serial.print(result.remainingWaitMs);
            Serial.println(" ms antes de reintentar.");
        }

        if (
            result.code ==
            CommandResultCode::TIMED_COMMAND_REQUIRED
        ) {
            Serial.println(
                "Ejemplo: on 1 30000"
            );
        }
    }
}

void processTurnOnCommand(
    const char* arguments
) {
    char commandCopy[SERIAL_COMMAND_BUFFER_SIZE];

    strncpy(
        commandCopy,
        arguments,
        SERIAL_COMMAND_BUFFER_SIZE - 1
    );

    commandCopy[
        SERIAL_COMMAND_BUFFER_SIZE - 1
    ] = '\0';

    char* parsingContext = nullptr;

    char* relayIdText = strtok_r(
        commandCopy,
        " ",
        &parsingContext
    );

    char* durationText = strtok_r(
        nullptr,
        " ",
        &parsingContext
    );

    char* unexpectedText = strtok_r(
        nullptr,
        " ",
        &parsingContext
    );

    bool relayIdIsValid = false;
    bool argumentsAreValid = false;

    uint8_t relayId = 0U;
    uint32_t durationMs = 0U;
    bool hasDuration = false;

    if (relayIdText != nullptr) {
        relayIdIsValid = parseRelayId(
            relayIdText,
            relayId
        );
    }

    if (relayIdIsValid && unexpectedText == nullptr) {
        argumentsAreValid = true;

        if (durationText != nullptr) {
            bool durationIsValid = parseDurationMs(
                durationText,
                durationMs
            );

            if (durationIsValid) {
                hasDuration = true;
            } else {
                argumentsAreValid = false;
            }
        }
    }

    if (argumentsAreValid) {
        RelayCommand command = {
            relayId,
            RelayCommandAction::TURN_ON,
            CommandSource::MANUAL,
            hasDuration,
            durationMs,
            RelayLockReason::NONE
        };

        RelayCommandResult result =
            commandProcessor.process(command);

        printProcessedCommandResult(
            command,
            result
        );
    } else {
        Serial.println(
            "Uso correcto: on <id> [duracionMs]"
        );
    }
}

void processSimpleRelayCommand(
    const char* arguments,
    RelayCommandAction action,
    RelayLockReason lockReason
) {
    char commandCopy[SERIAL_COMMAND_BUFFER_SIZE];

    strncpy(
        commandCopy,
        arguments,
        SERIAL_COMMAND_BUFFER_SIZE - 1
    );

    commandCopy[
        SERIAL_COMMAND_BUFFER_SIZE - 1
    ] = '\0';

    char* parsingContext = nullptr;

    char* relayIdText = strtok_r(
        commandCopy,
        " ",
        &parsingContext
    );

    char* unexpectedText = strtok_r(
        nullptr,
        " ",
        &parsingContext
    );

    bool relayIdIsValid = false;
    uint8_t relayId = 0U;

    if (relayIdText != nullptr) {
        relayIdIsValid = parseRelayId(
            relayIdText,
            relayId
        );
    }

    if (
        relayIdIsValid &&
        unexpectedText == nullptr
    ) {
        RelayCommand command = {
            relayId,
            action,
            CommandSource::MANUAL,
            false,
            0U,
            lockReason
        };

        RelayCommandResult result =
            commandProcessor.process(command);

        printProcessedCommandResult(
            command,
            result
        );
    } else {
        Serial.println("Uso incorrecto.");
    }
}

void printCommandHelp() {
    Serial.println();
    Serial.println("Comandos disponibles:");
    Serial.println("  help              -> Muestra esta ayuda");
    Serial.println("  status            -> Muestra estado de reles");
    Serial.println("  on <id>           -> ON manual continuo");
    Serial.println("  on <id> <ms>      -> ON temporizado");
    Serial.println("  off <id>          -> OFF y cancela timer");
    Serial.println("  lock <id>         -> Bloquea y apaga rele");
    Serial.println("  unlock <id>       -> Desbloquea y mantiene OFF");
    Serial.println();
}

void processRelayCommand(
    const char* command
) {
    bool commandWasHandled = false;

    if (strcmp(command, "help") == 0) {
        printCommandHelp();
        commandWasHandled = true;
    } else if (strcmp(command, "status") == 0) {
        relayManager.printStatus();
        commandWasHandled = true;
    } else if (strncmp(command, "on ", 3) == 0) {
        processTurnOnCommand(command + 3);
        commandWasHandled = true;
    } else if (strncmp(command, "off ", 4) == 0) {
        processSimpleRelayCommand(
            command + 4,
            RelayCommandAction::TURN_OFF,
            RelayLockReason::NONE
        );

        commandWasHandled = true;
    } else if (strncmp(command, "lock ", 5) == 0) {
        processSimpleRelayCommand(
            command + 5,
            RelayCommandAction::LOCK,
            RelayLockReason::MANUAL_LOCK
        );

        commandWasHandled = true;
    } else if (strncmp(command, "unlock ", 7) == 0) {
        processSimpleRelayCommand(
            command + 7,
            RelayCommandAction::UNLOCK,
            RelayLockReason::NONE
        );

        commandWasHandled = true;
    }

    if (!commandWasHandled) {
        Serial.println(
            "Comando no reconocido. Escribi: help"
        );
    }
}

void processSerialInput() {
    while (Serial.available() > 0) {
        char receivedCharacter = static_cast<char>(Serial.read());

        bool commandEnded =
            (receivedCharacter == '\n' || receivedCharacter == '\r');

        if (commandEnded) {
            if (serialCommandLength > 0 && !serialCommandOverflow) {
                serialCommandBuffer[serialCommandLength] = '\0';

                processRelayCommand(serialCommandBuffer);
            } else if (serialCommandOverflow) {
                Serial.println("Comando demasiado largo.");
            }

            serialCommandLength = 0;
            serialCommandOverflow = false;
        } else {
            if (!serialCommandOverflow) {
                bool bufferHasAvailableSpace =
                    (serialCommandLength < SERIAL_COMMAND_BUFFER_SIZE - 1);

                if (bufferHasAvailableSpace) {
                    serialCommandBuffer[serialCommandLength] = receivedCharacter;
                    serialCommandLength++;
                } else {
                    serialCommandOverflow = true;
                }
            }
        }
    }
}

void setup() {
    bool waterPumpAdded = relayManager.addRelay(&waterPumpRelay);
    bool extractorAdded = relayManager.addRelay(&extractorRelay);
    bool lightAdded = relayManager.addRelay(&lightRelay);
    bool humidifierAdded = relayManager.addRelay(&humidifierRelay);

    /*
     * Primero se inicializan los GPIO y se aplica el estado seguro.
     * Solo luego iniciamos la interfaz serial.
     */
    relayManager.begin();
    relayManager.applySafeStateToAll();

    Serial.begin(115200);
    delay(250);

    Serial.println();
    Serial.println("Sistema de reles iniciado.");

    Serial.print("Bomba agregada: ");
    Serial.println(waterPumpAdded ? "SI" : "NO");

    Serial.print("Extractor agregado: ");
    Serial.println(extractorAdded ? "SI" : "NO");

    Serial.print("Luz agregada: ");
    Serial.println(lightAdded ? "SI" : "NO");

    Serial.print("Humidificador agregado: ");
    Serial.println(humidifierAdded ? "SI" : "NO");

    relayManager.printStatus();
    testActuatorCatalog();
    printCommandHelp();
    if (ENABLE_TIMER_MANAGER_TEST) {
    startTimerManagerTest();
    }

    if (ENABLE_VALIDATOR_TEST) {
        testRelayCommandValidator();
    }    

}

void loop() {
    commandProcessor.update();
    processSerialInput();
}
