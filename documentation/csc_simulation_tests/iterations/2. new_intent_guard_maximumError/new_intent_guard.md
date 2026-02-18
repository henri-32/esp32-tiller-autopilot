## Commit on Branch 
simulation 
d75d2da

### Beobachtung
Ausgangslage unverändert zu Ergebnis von 1. additional_mean_parameter_for_csc. 

Drei relevant erscheinende Beobachtungen zum Thema aufschwingen: 

-- Das Reduzieren der steeringTolerance_deg führt jetzt zu einer reduktion der Amplitude. 
Das war vor der letzten Änderung anders. 
Dabei ist erkennbar, dass bei geringeren Toleranzen die Intents weniger asymmetrisch verteilt sind. Das würde die bisherigen Annahmen bestätigen, dass die asymmetrische Verteilung und das verzögerte Eingreifen (bzw. das zusätzliche Eingreifen, nachdem sich das System bereits in die richtige Richtung bewegt.) das aktive Aufschwingen verursacht. 

Dabei wirken folgende Parameter dämpfend auf die Amplitude der Schwingung 
-- steeringCooldown (reduziert Eingriffe bei bestehender korrekter Systemträgheit)
-- mean window (s. steeringCooldown, Intents und Intent auslösender Error sind zeitlich näher beieinander)
-- Der eingeführte mean Parameter skaliert den Intent wie vorgesehen. Je größer der Bereich ist in dem Skaliert wird, desto kleiner die Amplitude.  

### Hypothese 
Die Berücksichtigung nicht signifikanter Errors (innerhalb der steeringTolerance) hat den median Buffer robuster gemacht und damit reaktionen verzögert. Dadurch lässt sich im Output erkennen, dass das System deutlich zu lange Steuerimpulse setzt, obwohl die Winkelgeschwindigkeit und trägheit des physischen systems sich in die richtige Richtung bewegen. 
Dabei wirkt der neu eingeführte Mean positiv dämpfend auf dieses neue aktive Aufschwingen (je größer [heißt hier in großem Bereich wird die intent Stärke verringert] desto weniger amplitude.)

Das in der letzten Iteration beschriebene Problem hat sich durch die getroffenen Maßnahmen nicht verbessern lassen. 
Es zeigt sich deutlicher, dass Steering Intents bei gewünschter Dynamikrichtung begrenzt werden müssen um übersteuern zu verhindern. 

### Vorgaben 
Keine

### Experiment 
Es wird Omega als zusätzlicher Parameter in die CSC Steuerung aufgenommen, um abzubilden, wann Intents ausreichend Wirkung gezeigt haben. Es soll wenn ein negativer Median (negative Winkelbeschleunigung) einen Intent auslöst, Omega überwacht werden und bei Vorzeichenwechsel + Deadband die steering Guard aktiviert werden. Danmit soll erreicht werden, dass die Winkelgeschwindigkeit beim Überschreiten des Targets geringer ist und damit mit wenigeren (jetzt an der Fehlergröße orientierten) Intents korrigiert werden kann. 

## Ergebnis 

### Sonstiges 
