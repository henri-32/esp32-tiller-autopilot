## Commit on Branch 
simulation 
2bb1a50

### Beobachtung
Verschiedene Config Einstellungen verändern zwar Amplitude Frequenz, Häufigkeit der Intents etc. 
Allderding ist das System nicht in der Lage das Target abzufangen sondern endet in einer stabilen aber zu großen Oszillation ohne runaway.
Dabei ist die Amplitude scheinbar unabhängig von dem HDG Error zum Sim Start.
Größere HDG Toleranzen führen bis ca. 7 deg zu weniger Oszillation. 

### Hypothese 
Fehlender Parameter für Größe des Fehlers führen zu immer gleich starken Impulsen. Damit kann das System das Target nicht abfangen, wenn bei kleinem Fehler der Standardimpuls eine Korrektur über das Target hinaus verursacht. 
Der positive Einfluss von HDG Toleranz selbst in sehr einfacher Simulation spricht dafür, dass bei kleinem Fehler zu stark gesteuert wird. 

Der Error zum Zeitpunkt des Intents ist als Parameter durch Störungen vermutlich zu zufällig. 
Das einfache glätten durch Nutzung des means der letzten Errors von ca. 1-2 Sekunden ist vermutlich ein besserer Indikator. 
Die Entscheidung für Intent und Richtungsvorgabe soll weiter über den robusten median erfolgen !!

Das Signifikanzfiltern von Errors vor dem Schreiben in den Buffer könnte dazu führen, dass gute Steuerergebnisse das System nicht ausreichend stabilisieren. --> Im Hinterkopf behalten.  

### Vorgaben 
Der CSC soll wenn möglich erst einmal nichts von Omega als Winkelgeschwindigkeit wissen um nicht zu viele Regelungsebenen zu implementieren. 

### Experiment 
- Anpassung des CSC in Form eines zusätzlichen Parameters. 
- Definieren ab welchem Fehler 100% des Impulses wirken dürfen 
- Mean der letzten Errors als Fehlergröße implementieren 
- Diesen Fehler linear auf abstractIntent_0_100 anwenden 

--- Während dieser Implementierung ist Folgendes aufgefallen : 
Ein neues Array für mean berechnung wirkt wie Speicher / Komplexitäts Overhead weil bereits ein Error Array vorhanden ist.
Wenn ich dieses aber zur Berechnung der Fehlergröße nutze, müssen auch für die Richtungsentscheidung nicht signifikante Fehler im Array sein. Deswegen kommentiere ich die signifikanzfilterung der Richtungsentscheidung für den nächsten Run erstmal wieder aus. 
Sollte sich herausstellen, dass die RICHTUNGSENTSCHEIDUNG schlechte Ergebnisse liefert, könnte ich es wieder implementieren. Bisher spricht aber alles dafür, dass das Array für den Median zur Richtungsentscheidung (bis auf den Fall s.u.) robust ist, und andere Parameter problematisch sind. 



## Ergebnis 
-- Das Einfließen von nicht signifikaten Werten in den median hat zu einem flüssigeren und akkuraterem median Verlauf geführt. 
Das wird erst einmal als gut beurteilt. Er sollte damit in dieser symmetrischen und cleanen Simulation sehr genau sein und bei sensor Rauschen trotzdem noch robust. 

-- Das Einführen des zusätzlichen means zur Bestimmung der Fehlerstärke hat das Problem, dass das System stabil oszilliert nicht gelöst. 
Stattdessen schwingt es sich aktiv nun aktiv auf.
Allerdings lässt sich jetzt eine Assymmetrie in der Verteilung der Intents auf jeder Seite des Targets erkennen. 
Auf jeder Seite der Verteilung beginnen die Intents nach Überschreiten der Toleranzgrenze. Sie setzen sich jedoch weiter fort, nachdem die Toleranzgrenze durch Korrektur wieder unterschritten wird. 
Dies umgeht die Toleranzgrenze einseitig, sorgt dafür, dass auf jeder Seite des targets mehr Energie durch Korrektur entsteht, als durch Fehler hineingegeben wurde und erklärt das aktive Aufschwingen. 
Das deutet stark darauf hin, dass die Reaktion mit Intents zu verzögert passiert, weil der Buffer zu stabil ist. 

### Entscheidung 
- zusätzliche (hier nicht begründete) Implementation von mean abs(error)  für die Sim  um einen GROBEN Qualitätsmarker der angewendeten config zu bekommen
- nicht signifikante Werte fließen in median ein. 
- Der mean wird erst einmal behalten, da keine unmittelbaren negativen Auswirkungen beobachtet werden können und er als Fehlergröße theoretisch wichtig erscheint. 
- Weiterere Analyse hinsichtlich dem Blocken von Intents. 

### Sonstiges 
In mehreren Fällen kam es zu nach Fehler links (führt zurecht zu mehreren Intents SteeringDirection::Right) zu einem weiteren Intent SteeringDirection::Right, obwohl das HDG zum Zeitpunkt des weiteren Intents bereits ein Fehler rechts beinhaltete. 
Das ist kritisch weil es teilweise bei kleinerer Oszillation deutlich nach dem Überschreiten des Targets passiert. 
--> Problem beobachten, ob es sich mit zusätzlichem Parameter der Errorgröße ändert oder ob ein zusätzlicher Guard nötig sein könnte.  

