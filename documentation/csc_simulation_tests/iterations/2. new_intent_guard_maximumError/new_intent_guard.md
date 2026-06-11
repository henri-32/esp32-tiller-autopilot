## Commit on Branch 
simulation 
d75d2da

### Beobachtung
Ausgangslage unverändert zu Ergebnis von 1. additional_mean_parameter_for_csc. 

Drei relevant erscheinende Beobachtungen zum Thema Aufschwingen: 

-- Das Reduzieren der steeringTolerance_deg führt jetzt zu einer reduktion der Amplitude. 
Das war vor der letzten Änderung anders. 
Dabei ist erkennbar, dass bei geringeren Toleranzen die Intents weniger asymmetrisch verteilt sind. Das würde die bisherigen Annahmen bestätigen, dass die asymmetrische Verteilung und das verzögerte Eingreifen (bzw. das zusätzliche Eingreifen, nachdem sich das System bereits in die richtige Richtung bewegt.) das aktive Aufschwingen verursacht. 

Dabei wirken folgende Parameter dämpfend auf die Amplitude der Schwingung 
-- steeringCooldown (reduziert Eingriffe bei bestehender korrekter Systemträgheit)
-- mean window (s. steeringCooldown, Intents und Intent auslösender Error sind zeitlich näher beieinander)
-- Der eingeführte mean Parameter skaliert den Intent wie vorgesehen. Je größer der Bereich ist in dem skaliert wird, desto kleiner die Amplitude.  

### Hypothese 
Die Berücksichtigung nicht signifikanter Errors (innerhalb der steeringTolerance) hat den median Buffer robuster gemacht und damit Reaktionen verzögert. Dadurch lässt sich im Output erkennen, dass das System deutlich zu lange Steuerimpulse setzt, obwohl die Winkelgeschwindigkeit und trägheit des physischen systems sich in die richtige Richtung bewegen. 
Dabei wirkt der neu eingeführte Mean positiv dämpfend auf dieses neue aktive Aufschwingen (je größer [heißt hier in großem Bereich wird die intent Stärke verringert] desto weniger amplitude.)

Das in der letzten Iteration beschriebene Problem hat sich durch die getroffenen Maßnahmen nicht verbessern lassen. 
Es zeigt sich deutlicher, dass Steering Intents bei gewünschter Dynamikrichtung begrenzt werden müssen um übersteuern zu verhindern. 

### Vorgaben 
Keine

### Experiment 
Es wird Omega als zusätzlicher Parameter in die CSC Steuerung aufgenommen, um abzubilden, wann Intents ausreichend Wirkung gezeigt haben. Es soll wenn ein negativer Median (negative Winkelbeschleunigung) einen Intent auslöst, Omega überwacht werden und bei Vorzeichenwechsel + Deadband die steeringguard aktiviert werden. Danmit soll erreicht werden, dass die Winkelgeschwindigkeit beim Überschreiten des Targets geringer ist und damit mit wenigeren (jetzt an der Fehlergröße orientierten) Intents korrigiert werden kann. 

## Ergebnis 
Es wurde auf Basis von OpenSource physics Modellen der Simulator angepasst. Dieser bildet vereinfachte hydrodynamische Dämpfug ab. Steering Intents wirken jetzt auf eine API, welche eine simulierte Ruderstellung als Input enthält, statt wie bisher direkt auf die Winkelbeschleunigung zu wirken. Positive Auswirkungen der Anpassungen zeigen sich dabei sowohl bei dem alten, als auch bei dem neuen Simulationsmodell. 
Der zusätzliche Parameter hat wie gewünscht dazu geführt, dass Intents bei korrekter Systemdynamik beschränkt werden und mit weniger Intents bessere Steuerergebnisse im Sinne von geringerer Amplitude erreicht werden. 
Es werden in allen Szenarien große Startfehler eingefangen und auf eine relativ stabile Amplitude von ca. 5-10 Grad pro Richtung verringert. 
Dabei fällt auf, dass der stabile Buffer für verzögerte Reaktionen verantwortlich ist. Es wird erst mehrere Sekunden nach überschreiten der steeringTolerance der erste Intent ausgelöst. 
Dies wäre in der Simulation verbesserungswürdig. Es soll allerdings im Fokus behalten werden, dass der Sinn des Systems eine Trimmregelung mit gummigedämpftem Steuerverhalten ist. Es soll bewusst nicht jeder Fehler direkt korrigiert werden sondern im Besten Fall abgewartet werden, bis das Gummi den Fehler selbst korrigiert und wenn das nicht klappt auf Evidenz über Zeit reagiert werden. 
Daher sind weitere Optimierungen des CSC auf diese einfachen Simulationen vermutlich eher ungeeignet das reale Steuerergebnis zu verbessern. 
Insbesondere wirken einige der Timer Guards negativ auf die Regelung im Simulator, während davon auszugehen ist, dass sie in der Realität übermäßig häufige Impulse begrenzen. 
Insbesondere die Feststellung, wie sich reale Intents tatsächlich auf die Winkelgeschwindigkeit auswirken ist nötig für weitere Config Optimierungen. 

Lediglich ein frühzeitiges Abbremsen der Winkelgeschwindigkeit vor dem Überschreiten des Targets, soll noch geprüft werden, da dies auch in Realanwendung Übersteuern und Oszillation vermindern könnte.  


### Sonstiges 
