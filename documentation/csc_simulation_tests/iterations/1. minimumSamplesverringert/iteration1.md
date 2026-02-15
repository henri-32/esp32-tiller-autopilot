### Beobachtung
In der ersten Simulation steuert der CSC einen Intent nach, nachdem das HDG das Target überschossen hat, obwohl die Systemdynamik sich schon in die richtige Richtung entwickelt. 

### Hypothese 
Der Median Buffer enthält historische Dominanz. Das aktuelle HDG hat bisher keinen Direkten Einfluss auf Steuerentscheidungen. 
Deshalb erfolgt die Reaktion verzögert. 

### Experiment 
minimumSampleSize von 5 auf 3 verringern. 


## Ergebnis 
- Oszillation verringert
- Median raw nimmt geringere Extreme an 
- Kein zusätzlicher Impuls nach überschreiten des Target HDGs 

### Entscheidung 
- minimumSampleSize erstmal auf 3 lassen 

### Sonstiges 
Dämpfung auf angular Velocity bisher nur einseitig --> sofort ändern 
Angular velocity darf geglättet dargestellt werden --> sofort ändern
Angular velocity bleibt nicht bestehen. Das ist Implementierungsfehler der Simulation und entspricht nicht den mechanischen Eigenschaften meines Systems. 
Deswegen Anpassung der Sim Logik auf intent--> Trimparameter-->ngular velocity  anstatt einmaligemImpuls