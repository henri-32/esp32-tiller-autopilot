# Core Steering Controller – Konfigurationsreferenz

Dieses Dokument beschreibt ausschließlich die **Konfigurationsparameter**
des Core Steering Controllers (CSC) und deren **Auswirkungen auf Verhalten,
Robustheit und Energieverbrauch**.

Architektur, Rollen und Systemgrenzen sind in `ARCHITECTURE.md` definiert
und werden hier **nicht wiederholt**.

---

## Grundannahmen für das Tuning

- Alle Parameter beeinflussen **Geduld und Robustheit**, nicht das Regelprinzip.
- Zeitparameter sind **Messhygiene und Physikschutz**, keine Entscheidungsgrößen.
- Der CSC reagiert auf **dominante Bedeutung**, nicht auf Momentanabweichung.

---

## Parameterübersicht

- `steeringTolerance_deg`
- `SteeringMinImpulse_ms`
- `SteeringMaxImpulse_ms`
- `SteeringCooldown_ms`
- `minimumTimeBtwObs_ms`
- `observationBufferSize`
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

### `SteeringMinImpulse_ms`

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

### `SteeringMaxImpulse_ms`

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
Maximale Anzahl von Observations im Evidenzraum.
Bestimmt, wie viel Dominanz nötig ist, bevor getrimmt wird.

**Zusammenhang**
max. Beobachtungsdauer
≈ minimumTimeBtwObs_ms × observationBufferSize
