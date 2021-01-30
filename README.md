# ProjectGE-MySignals

## Projekt_MySignals_Sensoren_auslesen
Das hier ist das Orginalprojekt, welches das MySignals Board nutzt um verschiedene Sensoren auszulesen und auf einer grapfischen Oberfläche auszugeben.
Die GUI kann über einen Rotary Encoder umgeschaltet werden, die 3 Fenster in diesem dienen für folgende Zwecke:
- Körper Positions Sensor
- Elektromyograph
- Informationen

Aufgrund der Corona Pandemie war der Zugang zu dem MySignals Board eingeschränkt und das Projekt konnte nicht vervollständigt werden.

## Projekt_Home_Version
Um weiter an dem Projekt zu arbeiten, wurde der Code geportet um nicht mehr vom MySignals Board abhängig zu sein. Die Fenster sehen nun wie folgt aus:
- Aktuell wird eine Zufallszuordnung statt eines Sensors verwendet
- Electrkardiograph
- Informationen

Um das Projekt einfacher umsetzen zu können, wurde eine zuvor selbst erstellte Platine verwendet. Diese dient der besseren Struktur der Verkabelung.
Die Potentiometer auf der Platine dienen dazu den Graphen entlang der x-Achse zu skalieren und entlang der y-Achse zu skalieren und zu verschieben.
Es ist empfehlenswert eine Batterie als Stromquelle zu verwenden, da die USB-Verbindung Rauschen im EKG-Signal erzeugen kann.
### Bild der "Home Version"
![Beispielhafter Aufbau](https://github.com/dont-ask-why/ProjectGE-MySignals/blob/main/Home_Version_Example.png)
## ToDo
Das Projekt ist noch nicht fertig - in beiden Versionen fehlt eine eigentlich angedachte Ansteuerung von LEDs für eine vereinfachte Einschätzung der Messdaten.
