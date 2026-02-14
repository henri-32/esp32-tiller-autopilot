
3. Zeitkonstanten explizit machen
3.1 Dokumentation

 Frequenzräume als Tabelle dokumentieren:

Ebene	Zeitbereich	Aufgabe
Mechanik	< 1s	Dämpfung
CSC	~30–70s	Bias-Erkennung
WindTrend	> CSC	Umwelttrend
COGTrend	> WindTrend	Geometrische Drift
3.2 Konfigurationsklarheit

 Default-Werte bewusst definieren

 Kommentieren, warum diese gewählt wurden

 Sicherstellen, dass Zeitkonstanten sich nicht überlappen

4. Steering-Pipeline explizit machen
4.1 Datenfluss dokumentieren

Explizit festhalten:

SourceEvaluator
    ↓
CoreSteeringController
    ↓
ImpulseFilter
    ↓
PWMController

4.2 Kontraktdefinitionen hinzufügen

Kurz dokumentieren:

Was garantiert CSC?

Was garantiert ImpulseFilter?

Was garantiert SourceEvaluator?

Ziel:
Review ohne Code-Tiefenstudium ermöglichen.

5. Diagnostics – strukturelle Klarheit

 Dokumentieren:

Diagnostics darf keinen Einfluss auf Regelung haben.

 Sicherstellen, dass kein Regelpfad über Diagnostics läuft.

6. Wind/GPS-Logik – Vorbereitung für saubere Zeittrennung

 Zieländerungen rate-limitieren (nicht sprunghaft)

 Sicherstellen:

Zielbewegung innerhalb CSC-Fenster bleibt unter Toleranzschwelle

Optional vorbereiten:

float maxTargetRate_deg_per_s;

7. Native-Test-Vorbereitung
7.1 Test-Szenarien definieren

 Stationärer Bias

 Kurzzeitiger Spike

 Langsame Drift

 Wanderndes Ziel

7.2 Kleine Simulationsschicht bauen
struct CSCScenario { ... };
struct CSCResult { ... };