### On Branch 
simulation

### Beobachtung 
Omega bei Überschreiten des Targets im Folgenden OT (gibt die Energie für den nächsten Fehler in die andere Richtung vor) ist in der statischen Simulation abhängig von der Intentstärke der letzten Intents (das Omega Deadband beeinflusst wie viele Intents nach Omega = 0 erlaubt sind, während omegaRobust beeinflusst, wie verzögert die Guard bleibt) 
Unter Realbedingungen könnten Umwelteinflüsse die OT sowohl erhöhen, als auch verringern. 
Solange die OT spürbar über 0 liegt, kann das System keinen Kurs dauerhaft einfangen. Ein Omega ist nötig um in Richtung Target zu regeln und größere Omegawerte sorgen sogar für schnelle Korrektur nach einem signifikanten Error Median und sind damit nicht per se schlecht. 
Ist Omega bei OT jedoch hoch, wird schnell ein Fehler in die andere Richtung aufgebaut, während der CSC erst auf signifikanten Buffer warten muss. Dies wiederholt sich und führt zur Oszillation. 

### Hypothese 
Das präventive Gegensteuern vor Erreichen des Targets könnte die Oszillationsproblematik beenden. Dazu müssten die Bedingungen für das Gegensteuern restriktiv ausgelegt werden, damit der CSC nicht bei bestehendem oder leicht oszillierendem Fehler um 0 herum initiativ in die falsche Richtung steuert(kopplung an vorherige Impulse und Auswertung des median). 

Außerdem könnte ein resetten des Buffers bei erreichen des Targets verhindern, dass bisherige Fehler nach links den Median zu stabil für die Erkennung von neuen Fehlern nach rechts machen. Das würde auch semantisch passen, weil der reset() immer dann passiert, wenn die Beobachtung aktiv verworfen wird, da das Steuerergebnis gut ist. 
Wenn das Target erreicht ist, ist der momentane Steuerkurs optimal und eine neue Beobachtung könnte gestartet werden. Damit würde oszillieren um 0 mit kleinen Fehlern auch bei kleinen observation buffers nie zu signifikantem median und Steuerimpuls führen. 

### Experiment
Stufe 1: 
Buffer reset bei HDG = target implementieren und den Einfluss auf Intentzeiten untersuchen

Stufe 2: 
Implementierung eines Gegenimpulses kurz vor Erreichen des Targets 

### Ergebnis 

### Sonstiges
