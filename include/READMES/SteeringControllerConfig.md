# Core Steering Controller – Konfigurationsreferenz

Dieses Dokument beschreibt ausschließlich die **Konfigurationsparameter**
des Core Steering Controllers (CSC) und deren **Auswirkungen auf Verhalten,
Robustheit und Energieverbrauch**.

Architektur, Rollen und Systemgrenzen sind in `Architektur.md` definiert
und werden hier **nicht wiederholt**.

---

## Config-Lifecycle (aktueller Stand)

Die Konfiguration wird im `SystemController` gehalten und beim Bau der
Module als `const`-Referenzen auf die benoetigten Config-Slices verdrahtet.
Ein Runtime-Hot-Reload ueber pending-Struktur ist im aktuellen Code noch
nicht implementiert.

---

## Grundannahmen für das Tuning

- Alle Parameter beeinflussen **Geduld und Robustheit**, nicht das Regelprinzip.
- Zeitparameter sind **Messhygiene und Physikschutz**, keine Entscheidungsgrößen.
- Der CSC reagiert auf **dominante Bedeutung**, nicht auf Momentanabweichung.

---

## Parameterübersicht

- `steeringTolerance_deg`
- `counterNearTargetWindow_deg`
- `steeringMinImpulse_ms`
- `steeringMaxImpulse_ms`
- `steeringCooldown_ms`
- `minimumTimeBtwObs_ms`
- `observationBufferSize`
- `activeObservationBufferSize`
- `minimumSampleSize`
- `calculationWindowSmoothedMean`
- `smoothedMeanApplicationWindow_deg`
- `omegaRobust`
- `omegaDeadband`
- `pauseForValidObsAfterImpulse_ms`

---

## Parameter im Detail

---

### `steeringTolerance_deg`

**Bedeutung**  
Toleranzbereich um 0° Heading-Error (Median).  
Solange die beobachtete Mittellage innerhalb dieser Toleranz liegt,
gilt das System als ausreichend getrimmt.

**Größer einstellen**
- weniger Trimmimpulse
- ruhigeres Verhalten
- geringerer Energieverbrauch
- größere Kursabweichung akzeptiert

**Kleiner einstellen**
- präziserer Kurs
- häufigere Impulse
- höhere Aktorlast
- Risiko von Nervosität

**Typische Werte**
- Minimum: ca. 2–3°
- Praxis: 5–15°
- Maximum: ca. 25–30°

---

### `steeringMinImpulse_ms`

**Bedeutung**  
Untergrenze für einen Trimmimpuls.  
Stellt sicher, dass ein Impuls mechanisch wirksam ist
(Reibung, Spiel, Elastizität).

**Größer einstellen**
- garantiert mechanische Wirkung
- kann bei feiner Mechanik zu grob sein

**Kleiner einstellen**
- sanftere Eingriffe
- Risiko: Impuls verpufft wirkungslos

**Typische Werte**
- ca. 20–200 ms (stark hardwareabhängig)

---

### `steeringMaxImpulse_ms`

**Bedeutung**  
Obergrenze für einen einzelnen Trimmimpuls.
Dient als Sicherheits- und Begrenzungsparameter.

**Größer einstellen**
- stärkere Einzelkorrekturen
- schnelleres Bias-Abfangen
- erhöhtes Überschwing- und Stressrisiko

**Kleiner einstellen**
- sehr sanfte Korrekturen
- ggf. mehrere Zyklen notwendig

**Typische Werte**
- ca. 200–2000 ms

**Hinweis**  
Die Impulsdauer ist **kein Regelparameter**, sondern eine Aktorgrenze.

---

### `SteeringCooldown_ms`

**Bedeutung**  
Mindestzeit zwischen zwei Trimmimpulsen.
Dient ausschließlich dem Schutz von Aktor und Energiehaushalt.

**Größer einstellen**
- deutlich ruhigeres System
- weniger Energieverbrauch
- langsamere Reaktion auf Drift

**Kleiner einstellen**
- reaktiveres Verhalten
- Risiko von Nachtrimmen und Aktorstress

**Typische Werte**
- Minimum: ca. 500–1500 ms
- Praxis: 2000–8000 ms
- Maximum: ca. 15000–30000 ms

**Hinweis zum aktuellen Implementierungsstand**
- Der Cooldown hat derzeit einen staerkeren Systemeinfluss als reine
  Aktorschonung, weil der Counter-Intent im `tick()` vor den regulaeren
  Intent-Guards geprueft wird.
- In Kombination mit engem Counter-Fenster und Omega-Logik kann ein hoher
  Cooldown alternierende Counter/Normal-Impulse beguenstigen und damit als
  Regimeparameter wirken.

---

### `minimumTimeBtwObs_ms`

**Bedeutung**  
Abtastrate der Observation.
Definiert, wie oft ein neuer Heading-Wert als **eigenständige Evidenz**
akzeptiert wird.

**Wichtig**  
Messhygiene-Parameter, **kein Regelparameter**.

**Größer einstellen**
- weniger korrelierte Samples
- stabilere Evidenz
- längeres Beobachtungsfenster

**Kleiner einstellen**
- mehr Samples pro Zeit
- Risiko von Scheindominanz durch Korrelation

**Typische Werte**
- Minimum: ca. 100–200 ms (nur bei sehr ruhigen Sensoren)
- Praxis: 500–1500 ms
- Maximum: ca. 3000–5000 ms

---

### `observationBufferSize`

**Bedeutung**  
Compile-time Maximalgroesse des Observation-Buffers.  
Bestimmt die feste Obergrenze der intern allozierten CSC-Arrays.

**Zusammenhang**
max. Beobachtungsdauer
~= minimumTimeBtwObs_ms * observationBufferSize

### `activeObservationBufferSize`

**Bedeutung**  
Laufzeit-aktive Kapazitaet des Observation-Buffers.  
Der Wert steuert, wie viele Samples im aktuellen Run gesammelt werden,
bevor der Evidenzraum als voll gilt.

**Hinweis Sim-Tuning**  
In der Simulation kann dieser Wert per JSON gesetzt werden
(`csc.observationBufferSize`).
Im Produktiv-Default bleibt `activeObservationBufferSize` auf dem
compile-time Maximum `observationBufferSize`.

---

### `calculationWindowSmoothedMean`

**Bedeutung**  
Anzahl der Error-Samples fuer einen geglaetteten Mean der Fehlergroesse.
Die Richtungsentscheidung bleibt median-basiert; der Mean ist als
zusaetzlicher Groessenindikator gedacht.

**Ziel im Tuning-Kontext**  
Bei kleinen Fehlern soll weniger aggressiv reagiert werden, um
Nachtrimmen und Ueberschwingen um das Target zu reduzieren.

**Einheit / Bezug**  
Sample-Anzahl (kein Zeitwert).  
Effektive Zeitspanne grob:
`calculationWindowSmoothedMean * minimumTimeBtwObs_ms`

**Groesser einstellen**
- staerker geglaettete Fehlergroesse
- robuster gegen einzelne Ausreisser
- traeger bei schnellen Aenderungen

**Kleiner einstellen**
- reaktiver auf aktuelle Fehlergroesse
- empfindlicher gegen Messrauschen

**Hinweis Implementierungsstand**  
Die Berechnung liegt im ObservationBuffer (`getSmoothedCurrentError()`).
Der CSC bestimmt die Richtung weiterhin ueber den Median.

---

### `smoothedMeanApplicationWindow_deg`

**Bedeutung**  
Symmetrisches Fenster um 0 fuer den geglaetteten aktuellen Fehler
(`getSmoothedCurrentError()`).

**Verhalten im CSC**  
- Wenn `|currentError| > smoothedMeanApplicationWindow_deg`, bleibt der
  abstrakte Impuls bei 100.
- Wenn `|currentError| <= smoothedMeanApplicationWindow_deg`, wird der Impuls
  linear skaliert:
  `abstractImpulse_0_100 = 100 * |currentError| / smoothedMeanApplicationWindow_deg`

**Groesser einstellen**
- mehr weicher Uebergang nahe dem Target
- weniger abrupte Impulsspruenge

**Kleiner einstellen**
- schneller voller Impuls
- aggressiveres Verhalten bei mittleren Fehlern

**Hinweis**
- Der Wert muss groesser als 0 bleiben, damit die lineare Skalierung sinnvoll
  bleibt.

---

### `omegaRobust`

**Bedeutung**  
Anzahl der letzten Error-Samples, die in die Omega-Schaetzung eingehen.
Die effektive Fenstergroesse ist `min(sampleSize, omegaRobust)`.

**Groesser einstellen**
- robustere Omega-Schaetzung
- traeger bei schnellen Richtungswechseln

**Kleiner einstellen**
- reaktiver auf kurzfristige Aenderungen
- empfindlicher gegen Rauschen

**Hinweis**
- `0` deaktiviert effektiv die Omega-Schaetzung (`omega = 0`).

---

### `omegaDeadband`

**Bedeutung**  
Toleranzband fuer die Omega-Guard-Entscheidung im `SteeringGuard`.
Liegt die gemessene Winkelgeschwindigkeit innerhalb dieses Bands, blockiert
Omega den Intent nicht.

**Groesser einstellen**
- weniger Blockierung durch kleine Restrotationen
- konservativeres Nachsteuern

**Kleiner einstellen**
- fruehere Blockierung bei Rotation in Zielrichtung
- kann zu weniger Nachtrimmen fuehren

**Hinweis**
- Muss `>= 0` sein.

---

### `counterNearTargetWindow_deg`

**Bedeutung**  
Absolutes Error-Fenster fuer den Counter-Intent (`|error| <= window`).
Dieser Parameter ist bewusst von `steeringTolerance_deg` entkoppelt, damit
Counter-Tuning und Deadband-Tuning getrennt erfolgen koennen.

**Groesser einstellen**
- Counter greift frueher und haeufiger ein
- besseres fruehes Abfangen moeglich
- hoeheres Risiko fuer alternierende Gegenimpulse

**Kleiner einstellen**
- Counter greift spaeter und seltener ein
- konservativeres Verhalten nahe Target
- hoehere Restenergie beim Target-UEbergang moeglich

**Typische Startwerte**
- 1-3 Grad (Default: 2 Grad)
