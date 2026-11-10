# draadloze communicatie proof of concept
minimale hard- en software waarmee aangetoond wordt dat duplex kan gecommuniceerd worden tussen de microcontroller en een [laptop|smartphone] (schappen wat niet past), gebruik makend van [programma] (in te vullen)
<br />
**Draadloze Communicatie Proof of Concept**

Minimale hard- en software waarmee aangetoond wordt dat duplex kan gecommuniceerd worden tussen de microcontroller en een **smartphone**, gebruik makend van **Serial Bluetooth Terminal**.

---

### Configuratie

* **Hardware:** Microcontroller (bijv. Arduino), HC-05 Bluetooth-module (TX aangesloten op digitale pin 2, RX via een spanningsdeler op digitale pin 3).
* **Software:** Arduino-code met de `SoftwareSerial`-bibliotheek en een Bluetooth Serial-terminal app op de smartphone.

---

### Opmerkingen

* **Spanningsdeler:** Noodzakelijk op de RX-pin van de HC-05 om het 5V-signaal van de microcontroller te verlagen naar 3.3V.
* **Duplex communicatie:** De verbinding is tweerichtingsverkeer; de microcontroller ontvangt commando's en stuurt gelijktijdig instellingen en statusmeldingen terug naar het externe apparaat.
* **Baudrate:** De HC-05 is ingesteld op een communicatiesnelheid van 38400 baud.

---

### Gebruiksaanwijzing

1. **Verbinden:** Zet de microcontroller aan. Open de Bluetooth-instellingen van je smartphone, koppel met de HC-05 
2. **Terminal openen:** Start de **Serial Bluetooth Terminal** app op je smartphone en maak verbinding met de gekoppelde HC-05. Bij een geslaagde verbinding zie je direct de huidige instellingen (`READY`, `KP=...`, enz.) in het scherm verschijnen.
3. **Commando's versturen:** Typ commando's in de terminal om de parameters of status van de robot direct aan te passen:
* **PID-waarden en snelheid aanpassen:** Typ bijvoorbeeld `KP=2.5`, `KD=15.0` of `MAX=180` en verstuur.
* **Besturen:** Typ `START` om de robot te starten of `STOP` om hem te pauzeren.
* **Status opvragen:** Typ `GET` om alle actuele instellingen opnieuw op te vragen.
