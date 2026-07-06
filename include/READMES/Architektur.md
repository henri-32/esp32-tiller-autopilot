# Autopilot Boot - Architektur

Dieses Dokument beschreibt die aktuelle Architektur des Projekts auf Basis des
derzeitigen Codes.

## 1. Zielbild

Das System ist als adaptive Trimmregelung aufgebaut:
- seltene, robuste Korrekturimpulse statt permanenter Momentanregelung
- klare Trennung zwischen Sensorik, Entscheidungslogik und Aktorik
- deterministischer Tick-Ablauf im `SystemController`

## 2. Zentrale Komponenten

- `SystemController`
  - Top-Level Orchestrierung des Ticks
  - besitzt Konfiguration und Module
- `NavigationSensors`
  - liest Rohwerte (Compass, GPS, Wind, STW)
- `SourceHandler`
  - bewertet Quell-Policy (Fallbacks)
  - filtert Target je Quelle
  - setzt effektive Quelle in `NavigationSensors`
- `CoreSteeringController` (CSC)
  - entscheidet ueber `SteeringIntent`
  - arbeitet auf Kurs-/Fehlerdaten, nicht auf Hardware
- `ImpulseFilter`
  - daempft abstract impulse u. a. mit STW-Kontext
  - erzeugt `PWMIntent`
- `PwmDriver`
  - mappt `PWMIntent` auf hardware-nahe PWM-Parameter
- `Diagnostics`
  - sammelt Ereignisse und Capability-Zustaende
- `UI` (`ControlPanel`, `UIContent`, `Display`)
  - Bedien-Intent und Darstellung

## 3. Aktueller Tick-Ablauf (`SystemController::tick`)

1. `nav_snapshot = NavigationSensors::createSnapshot()`
2. `panel_intent = ControlPanel::readIntent()`
3. `cscTarget = SourceHandler::tick(nav_snapshot, requestedSource, target, loopTimestamp)`
4. Falls Steering aktiv und System `OK`:
   `SteeringOrchestrator::tick(nav_snapshot, cscTarget, loopTimestamp)`
5. `Diagnostics::tick(loopTimestamp)` und Snapshot ziehen
6. `stateUpdate(diagnostics_snapshot)`
7. `UIContent::create(...)` und `Display::update(...)`

Hinweis:
- Im aktuellen Stand bekommt CSC `currentHDG` aus Compass
  (`snapshot.compass_hdg_dg.value`).

## 4. Verantwortungsgrenzen

### SystemController
- orchestriert
- enthaelt bewusst nur wenig Fachlogik

### SourceHandler
- entscheidet effektive Quelle inkl. Fallback
- transformiert `generalTarget` in quellenabhaengiges internes Ziel

### CSC
- beobachtet Fehler ueber Zeit
- nutzt Guards, Median, Omega und Counter-Intent
- gibt optional `SteeringIntent` zurueck

### Aktorpfad
- `ImpulseFilter`: fachliche Dampfungs-/Shaping-Logik
- `PwmDriver`: hardware-nahe Umsetzung

## 5. Diagnostics und State

- Source-Policy emittiert u. a. `Degraded`, `LostWithFallback`,
  `CRITICAL_ERROR`
- `Diagnostics` fuehrt Event-Puffer und Capability-States
- `SystemController::stateUpdate(...)` setzt aktuell bei Critical auf `SAFE`

## 6. Ordnerstruktur (Ist-Zustand)

```text
include/
|- core/
|  |- steering/
|  |  |- csc/
|  |  |- sourceModels/
|- sensors/
|- actuators/
|- diagnostics/
|- ui/
|- types/

src/
|- core/
|  |- steering/
|  |  |- csc/
|  |  |- sourceModels/
|- sensors/
|- actuators/
|- diagnostics/
|- ui/
```

## 7. Offene Architekturthemen (im Code als TODO markiert)

- Guidance-Output als eigener Typ statt losem Target-Rueckgabewert
- klarere State-Transition-Funktion (`prevState + inputs -> nextState`)
- vollstaendiges Event->State-Mapping in Diagnostics
- wirksame Sensor-Aktivierung fuer Stromsparmodi
