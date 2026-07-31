#pragma once

#include "navigation/navigationTypes.h"
#include "telemetry/runtimeTypes.h"

struct TelemetryError
{
  bool validFix = false;
};

struct TelemetryRuntimeLog
{
  RuntimeLog_t gps;
  RuntimeLog_t tHub;
  RuntimeLog_t inputHandler;
};

struct AccumulatedLogMessage
{
  NavigationSnapshot_t snapshot;
  TelemetryError error;
  TelemetryRuntimeLog runtime_log;
};



/*     Außerdem wird INPUT_HANDLER im TelemetryHub leer behandelt und damit verworfen: src/core/telemetryHub.cpp:100. Ein Input-Kommando sollte vermutlich nicht in derselben Queue wie Telemetrie landen.

     Wichtig für die Architektur: Eine FreeRTOS-Queue lässt sich nicht von mehreren Consumern nach ID „filtern“ – xQueueReceive entfernt die Nachricht. Pro gemeinsamer Queue sollte genau ein Dispatcher konsumieren und anhand der ID verteilen.

  6. Partielle UART-Initialisierung kann die Retry-Schleife dauerhaft festfahren.

     Wenn UART0 erfolgreich installiert wird, aber UART2 oder uart_set_pin() fehlschlägt, bleibt UART0 installiert. Beim nächsten Aufruf aus src/main/main.cpp:35 schlägt dessen erneute Installation vermutlich sofort fehl: src/drivers/uartDriver.cpp:29. init() muss idempotent sein oder Teilinitialisierungen sauber zurückrollen.

  7. Der neue InputHandler-Runtimewert wird extern nie übertragen.

     TelemetryRuntimeLog enthält nun inputHandler: include/telemetry/telemetryLogMessage.h:15. Die Wire-Offsets und Serialisierung enden aber weiterhin nach tHub: include/protocol/autopilotWireProtocol.h:69. Der Wert verschwindet beim Senden zum Gateway.

  8. Im GPS-Case fehlt ein break.

     Dadurch fällt GPS in den INPUT_HANDLER-Case durch: src/core/telemetryHub.cpp:98. Aktuell ist der Case leer, später wird das ein schwer auffindbarer Fehler.

  ## Bereits im staged Stand vorhanden

  Beim Nachverfolgen sind noch zwei bestehende kritische Probleme aufgefallen:

  - Der InputHandler übergibt einen task_context*, castet ihn im Task aber zu einem größeren, anderen Context* und liest daraus uartDriver: src/core/inputHandler/inputHandler.cpp:19. Das ist Undefined Behaviour.
  - UartDriver::read(GATEWAY) ruft cobs_decode() mit Quelle und Ziel vertauscht auf: src/drivers/uartDriver.cpp:120. Zusätzlich versucht der InputHandler anschließend noch einmal selbst COBS zu dekodieren.

  Mein Vorschlag für die Grundstruktur wäre: je eine gemeinsame Queue vom Typ TelemetryMessage, RuntimeLogMessage und ErrorMessage, jeweils ein einziger Consumer/Dispatcher, ausreichend Queue-Tiefe und typisierte Konstruktorfunktionen für alle drei Nachrichtentypen. Control-Kommandos würde ich separat halten.

  Außerdem ist include/protocol/internalMessageProtocol.h:1 noch untracked; beim nächsten Staging-Schritt darf die Datei nicht vergessen werden. Ich habe keine Dateien verändert.

*/
