# Third-Party Notices

Stand: 2026-02-18

Dieses Repository dokumentiert hier externe Quellen, die bei der
Weiterentwicklung der Simulationsphysik als Referenz betrachtet wurden.

Wichtig:
- Es wurde kein Fremdquellcode per Copy/Paste in dieses Projekt uebernommen.
- Die Implementierung in `sim/headingSimulator.cpp` ist eine eigene, reduzierte
  Umsetzung auf Basis allgemeiner physikalischer Konzepte.

## Referenzierte Repositories

| Projekt | URL | Commit | Lizenz | Verwendung im Projekt |
|---|---|---|---|---|
| Yara_OVE | https://github.com/medialab-fboat/Yara_OVE | `e708703e7367993f56ff2187a06e05b09d19e696` | Apache-2.0 | Konzeptionelle Referenz fuer Segel-Aerodynamik/Hydrodynamik (vereinfachtes Yaw-Modell in `sim/headingSimulator.cpp`). Kein Quellcode uebernommen. |
| SailBoatROS | https://github.com/Plymouth-Sailboat/SailBoatROS | `9bca1cbd880454d41c76e234b6881234bde7bdbc` | GPL-3.0 | Nur zur fachlichen Gegenpruefung betrachtet. Kein Quellcode uebernommen, keine Verlinkung/Einbindung. |
| sailboat-playground | https://github.com/gabriel-milan/sailboat-playground | `f756080ce7a58093de0e444d5d7b84600aee3301` | Keine Lizenzdatei gefunden | Nicht im Produktcode verwendet. |

## Lizenzhinweis

Falls spaeter tatsaechlich Fremdcode aus einem Drittprojekt uebernommen wird,
muss die jeweilige Lizenz (inkl. ggf. NOTICE-Datei) gemaess den Lizenzbedingungen
im Repository mitgefuehrt werden.
