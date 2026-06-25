#include <Arduino.h>
#include <cstring>
#include <cstdlib>

#include "relays/RelayConfig.h"
#include "relays/Relay.h"
#include "relays/RelayManager.h"
#include "relays/ActuatorCatalog.h"

#include "automation/timers/TimerManager.h"
#include "automation/timers/TimerStatus.h"

#include "automation/commands/RelayCommandProcessor.h"

// --------------------------------------------------
// Pines físicos
// --------------------------------------------------

constexpr uint8_t WATER_PUMP_RELAY_PIN = 16U;
constexpr uint8_t EXTRACTOR_RELAY_PIN = 17U;
constexpr uint8_t LIGHT_RELAY_PIN = 18U;
constexpr uint8_t HUMIDIFIER_RELAY_PIN = 19U;

// --------------------------------------------------
// Consola serial
// --------------------------------------------------

constexpr uint8_t SERIAL_COMMAND_BUFFER_SIZE = 32U;

char serialCommandBuffer[SERIAL_COMMAND_BUFFER_SIZE];
uint8_t serialCommandLength = 0U;
bool serialCommandOverflow = false;

// --------------------------------------------------
// Managers principales
// --------------------------------------------------

RelayManager relayManager;
TimerManager timerManager;

RelayCommandProcessor commandProcessor(
    relayManager,
    timerManager
);

// --------------------------------------------------
// Configuraciones de seguridad por actuador
// --------------------------------------------------

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

// --------------------------------------------------
// Configuración de relés
// --------------------------------------------------

RelayConfig waterPumpConfig = {
    1U,
    "Bomba agua",
    WATER_PUMP_RELAY_PIN,
    true,
    RelayState::OFF,
    true,
    ActuatorType::WATER_PUMP,
    waterPumpSafetyConfig
};

RelayConfig extractorConfig = {
    2U,
    "Extractor",
    EXTRACTOR_RELAY_PIN,
    true,
    RelayState::OFF,
    true,
    ActuatorType::EXTRACTOR,
    extractorSafetyConfig
};

RelayConfig lightConfig = {
    3U,
    "Luz",
    LIGHT_RELAY_PIN,
    true,
    RelayState::OFF,
    true,
    ActuatorType::LIGHT,
    lightSafetyConfig
};

RelayConfig humidifierConfig = {
    4U,
    "Humidificador",
    HUMIDIFIER_RELAY_PIN,
    true,
    RelayState::OFF,
    true,
    ActuatorType::HUMIDIFIER,
    humidifierSafetyConfig
};

// --------------------------------------------------
// Objetos Relay
// --------------------------------------------------

Relay waterPumpRelay(waterPumpConfig);
Relay extractorRelay(extractorConfig);
Relay lightRelay(lightConfig);
Relay humidifierRelay(humidifierConfig);

// --------------------------------------------------
// Conversión de enums a texto
// --------------------------------------------------

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

// --------------------------------------------------
// Estado del sistema
// --------------------------------------------------

void printActiveTimersStatus() {
    uint8_t activeTimerCount =
        timerManager.getActiveTimerCount();

    Serial.println();
    Serial.println("----- TIMERS ACTIVOS -----");

    if (activeTimerCount == 0U) {
        Serial.println("No hay timers activos.");
    } else {
        uint32_t nowMs = millis();
        uint8_t activeTimerIndex = 0U;

        while (activeTimerIndex < activeTimerCount) {
            TimerStatus timerStatus;

            bool timerStatusFound =
                timerManager.getActiveTimerStatusByIndex(
                    activeTimerIndex,
                    nowMs,
                    timerStatus
                );

            if (timerStatusFound) {
                Serial.print("Relay ID: ");
                Serial.print(timerStatus.relayId);

                Serial.print(" | Origen: ");
                Serial.print(
                    getCommandSourceText(
                        timerStatus.source
                    )
                );

                Serial.print(" | Restante: ");
                Serial.print(
                    timerStatus.remainingDurationMs
                );

                Serial.println(" ms");
            }

            activeTimerIndex++;
        }
    }

    Serial.println("--------------------------");
}

void printSystemStatus() {
    relayManager.printStatus();
    printActiveTimersStatus();
}

// --------------------------------------------------
// Parser de parámetros
// --------------------------------------------------

bool parseRelayId(
    const char* valueText,
    uint8_t& relayId
) {
    bool relayIdIsValid = false;

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

        bool valueWithinRange =
            (parsedValue > 0U &&
             parsedValue <= 255U);

        if (numericTextIsValid && valueWithinRange) {
            relayId = static_cast<uint8_t>(parsedValue);
            relayIdIsValid = true;
        }
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

// --------------------------------------------------
// Resultado de comandos
// --------------------------------------------------

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

// --------------------------------------------------
// Procesamiento de comandos seriales
// --------------------------------------------------

void processTurnOnCommand(
    const char* arguments
) {
    char commandCopy[SERIAL_COMMAND_BUFFER_SIZE];

    strncpy(
        commandCopy,
        arguments,
        SERIAL_COMMAND_BUFFER_SIZE - 1U
    );

    commandCopy[
        SERIAL_COMMAND_BUFFER_SIZE - 1U
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
        SERIAL_COMMAND_BUFFER_SIZE - 1U
    );

    commandCopy[
        SERIAL_COMMAND_BUFFER_SIZE - 1U
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
    Serial.println(
        "  help              -> Muestra esta ayuda"
    );
    Serial.println(
        "  status            -> Muestra estado de reles y timers"
    );
    Serial.println(
        "  on <id>           -> ON manual continuo"
    );
    Serial.println(
        "  on <id> <ms>      -> ON temporizado"
    );
    Serial.println(
        "  off <id>          -> OFF y cancela timer"
    );
    Serial.println(
        "  lock <id>         -> Bloquea y apaga rele"
    );
    Serial.println(
        "  unlock <id>       -> Desbloquea y mantiene OFF"
    );
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
        printSystemStatus();
        commandWasHandled = true;
    } else if (strncmp(command, "on ", 3U) == 0) {
        processTurnOnCommand(command + 3);
        commandWasHandled = true;
    } else if (strncmp(command, "off ", 4U) == 0) {
        processSimpleRelayCommand(
            command + 4,
            RelayCommandAction::TURN_OFF,
            RelayLockReason::NONE
        );

        commandWasHandled = true;
    } else if (strncmp(command, "lock ", 5U) == 0) {
        processSimpleRelayCommand(
            command + 5,
            RelayCommandAction::LOCK,
            RelayLockReason::MANUAL_LOCK
        );

        commandWasHandled = true;
    } else if (strncmp(command, "unlock ", 7U) == 0) {
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
        char receivedCharacter =
            static_cast<char>(Serial.read());

        bool commandEnded =
            (receivedCharacter == '\n' ||
             receivedCharacter == '\r');

        if (commandEnded) {
            if (
                serialCommandLength > 0U &&
                !serialCommandOverflow
            ) {
                serialCommandBuffer[
                    serialCommandLength
                ] = '\0';

                processRelayCommand(serialCommandBuffer);
            } else if (serialCommandOverflow) {
                Serial.println("Comando demasiado largo.");
            }

            serialCommandLength = 0U;
            serialCommandOverflow = false;
        } else {
            if (!serialCommandOverflow) {
                bool bufferHasAvailableSpace =
                    (serialCommandLength <
                     SERIAL_COMMAND_BUFFER_SIZE - 1U);

                if (bufferHasAvailableSpace) {
                    serialCommandBuffer[
                        serialCommandLength
                    ] = receivedCharacter;

                    serialCommandLength++;
                } else {
                    serialCommandOverflow = true;
                }
            }
        }
    }
}

// --------------------------------------------------
// Arduino setup / loop
// --------------------------------------------------

void setup() {
    bool waterPumpAdded =
        relayManager.addRelay(&waterPumpRelay);

    bool extractorAdded =
        relayManager.addRelay(&extractorRelay);

    bool lightAdded =
        relayManager.addRelay(&lightRelay);

    bool humidifierAdded =
        relayManager.addRelay(&humidifierRelay);

    /*
     * Primero: GPIO y estado seguro.
     * Después: interfaz serial.
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

    printSystemStatus();
    printCommandHelp();
}

void loop() {
    commandProcessor.update();
    processSerialInput();
}