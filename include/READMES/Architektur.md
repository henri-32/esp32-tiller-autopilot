# Autopilot Boot – Architektur

Dieses Dokument beschreibt die **konzeptionelle und strukturelle Architektur**
des Autopilot-Boot-Projekts.  
Es ist **kein Implementierungsleitfaden**, sondern ein stabiler Referenzrahmen
für Struktur, Verantwortlichkeiten und Abgrenzungen.

---

## 1. Grundidee & Zielsetzung

Das System ist **kein klassischer Autopilot**, sondern eine **adaptive Trimmregelung**.

Ziel ist nicht die exakte Momentanregelung, sondern die **Korrektur der
langfristigen Mittellage (Bias)**.

> **Leitsatz:**  
> Das Boot darf oszillieren – entscheidend ist, dass es nicht driftet.

Eigenschaften:
- kurzfristige mechanische Oszillation ist erlaubt und erwünscht
- Robustheit entsteht durch Ebenentrennung, nicht durch aggressive Regelung
- Entscheidungen sind selten, sanft und deterministisch

---

## 2. Gesamtarchitektur (Top-Level)

Die Architektur ist bewusst **flach und linear** gehalten.

### Zentrale Komponenten

- **SystemController**  
  Zentrale Orchestrierungseinheit, besitzt alle Subsysteme.

- **CoreSteeringController (CSC)**  
  Alleiniger Entscheider über Trimmimpulse.

- **CourseGuidance**  
  Ableitung von Kursen aus Sensoren, Intent und Filtern.

- **NavigationSensors**  
  Aggregator für Rohsensorik (Compass, GPS, Wind).

- **PWMController**  
  Reine Aktor-Ansteuerung ohne Kontext.

- **ControlPanel / Display**  
  Menschlicher Intent & Anzeige, keine Regelung.

---

## 3. Fester Systemablauf (System-Tick)

Der Ablauf im `SystemController` ist **fixiert** und bildet die Architekturachse:

1. Intent lesen (ControlPanel)
2. Politik anwenden (Modus, Quelle, Ziel)
3. Rohwerte lesen (NavigationSensors)
4. Kurs ableiten & validieren (CourseGuidance)
5. Trimmentscheidung treffen (CoreSteeringController)
6. Aktor ausführen (PWMController)

Dieser Ablauf darf **nicht implizit durchbrochen** werden.

---

## 4. Rollen & Verantwortlichkeiten

### SystemController
- besitzt alle Module
- orchestriert den Ablauf
- enthält **keine Fachlogik**

> **Architekturregel:**  
> SystemController darf besitzen, aber nicht steuern.

---

### CoreSteeringController (CSC)
Der CSC ist **bewusst minimalistisch und rein**.

Aufgaben:
- Beobachtung des Heading Errors (HDG)
- Sammeln von Evidenz über Zeit (Observation Buffer)
- Entscheidung über seltene Trimmimpulse
- Richtung ja/nein (keine Aktor-Parametrisierung)

Nicht-Aufgaben:
- keine Sensorvalidierung
- keine Navigation
- kein Logging
- kein Error-Handling
- keine Zielinterpretation

Der CSC kennt **nur Kurse**, keine Sensoren.

> **Merksatz:**  
> Der CSC trimmt die Mittellage, nicht den Momentanwert.

---

### CourseGuidance
Zwischenschicht zwischen Sensorik und Regelung.

Aufgaben:
- Sensorvalidierung & Fallbacks
- Filterlogik (GPS, Wind, COG)
- Ableitung von:
  - `currentHDG`
  - `internalTargetHDG`
- Modulation von Geduld / Toleranzen (z. B. Hull Speed)

CourseGuidance trifft **keine Aktorentscheidungen**.

---

### NavigationSensors
- liefert Rohdaten
- aggregiert Sensorquellen
- entscheidet **nicht**, warum eine Quelle aktiv ist
- keine Interpretation, keine Regelung

---

### PWMController
- reine Hardware-Ansteuerung
- kennt keinen Kurs
- kennt keine Regelung
- führt exakt aus, was angefordert wird

---

## 5. Beobachtungs- & Entscheidungsmodell (CSC)

### Beobachtung
- Heading Errors werden zeitlich diskret gesammelt
- Trennung in Left/Right Errors
- Zusammenführung in signierten Fehlerraum
- Median als robuste Abstraktion

### Entscheidung
- Entscheidung nur bei:
  - signifikanter Abweichung
  - über ausreichende Dauer
- Zeit dient nur als Guard (Physik, Messhygiene)

Konsequenz:
- Wellen & Böen werden ignoriert
- Dauerhafte Bias-Effekte führen zu sanfter Korrektur

---

## 6. Heading Error – semantische Einordnung

Der HDG Error liegt **oberhalb der mechanischen Ebene**:

- Pinne, Gummi, Ruder dürfen schwingen
- Oszillation innerhalb der Toleranz ist korrekt
- zu feine Toleranzen führen zu Fehlinterpretation

Zusätzlich:
- Ziel-HDG kann selbst bewegt sein (z. B. COG)
- der CSC reagiert auf ein **bewegtes Ziel**

---

## 7. GPS / COG / XTE – Ebenentrennung

Es existieren zwei strikt getrennte Regelräume:

### Winkelraum (HDG / COG)
- lokal
- schnell
- geeignet für Trimm

### Geometrischer Raum (XTE / Distanz)
- global
- langsam
- geeignet für Navigation

Regeln:
- COG darf Ziel-HDG ersetzen
- XTE darf **niemals direkt** auf Aktoren wirken
- XTE moduliert nur das interne Ziel-HDG

Fehler äußern sich dadurch als **Oszillation**, nicht als Drift.

---

## 8. Hull Speed

Hull Speed ist **kein Regelparameter**, sondern ein Modulator.

Darf beeinflussen:
- Toleranzen
- Cooldowns
- Geduld

Darf nicht:
- Ziele filtern
- Regelentscheidungen ersetzen

---

## 9. Error Handling & Diagnostics

- Error-Handling ist **explizit modelliert**
- Fehler sind Zustände, keine impliziten Rückgabewerte
- Diagnostics & Logging sind reine Beobachtung

> Beobachtung ≠ Entscheidung

Der CSC bleibt frei von Logging und Error-Logik.

---

## 10. Ordnerstruktur & Abhängigkeiten

Die Ordner spiegeln Domänen wider, nicht Technik.

```text
include/
├── core/
├── guidance/
├── sensors/
├── actuators/
├── ui/
├── diagnostics/
├── errors/
└── types/

src/
├── core/
├── guidance/
├── sensors/
├── actuators/
├── ui/
├── diagnostics/
└── errors/
