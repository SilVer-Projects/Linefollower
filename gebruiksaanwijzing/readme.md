# Gebruiksaanwijzing

### opladen / vervangen batterijen
De geselecteerde batterijen uit de Bill of Materials (BOM) zijn oplaadbaar en beschikken over een hoge capaciteit, wat zorgt voor een lange autonomie binnen deze toepassing. Om de batterijen na ontlading opnieuw te kunnen gebruiken, wordt aanbevolen om een compatibele batterijlader mee te bestellen, tenzij deze reeds beschikbaar is.

### draadloze communicatie
#### verbinding maken
1. Firmware uploaden: Zorg ervoor dat de Arduino-code met Bluetooth-functionaliteit correct is gecompileerd en geüpload naar de Arduino Nano.

2. Hardware aansluiten: Sluit de HC-05-module aan op de Arduino Nano. Gebruik hierbij een spanningsdeler op de RX-lijn van de module om het 5V-signaal van de Arduino veilig om te zetten naar 3,3V.

3. Systeem inschakelen: Schakel de voeding van de schakeling in. De status-LED op de HC-05 zal nu snel knipperen om aan te geven dat de module klaar is om te koppelen.

4. Applicatie installeren: Download en installeer de app Serial Bluetooth Terminal op uw smartphone.

5. Bluetooth koppelen: Schakel Bluetooth in op uw smartphone, zoek naar beschikbare apparaten en maak verbinding met HC-05.

6. Verbinding verifiëren: Open de geïnstalleerde terminal-app en selecteer de gekoppelde HC-05-module om de seriële verbinding te starten. Zodra de verbinding tot stand is gebracht, wijzigt het knipperpatroon van de status-LED op de module naar een trager interval.

7. Communicatie starten: De verbinding is nu actief. U kunt via de terminal rechtstreeks commando's versturen om de robot aan te sturen en parameters op te vragen of te wijzigen.

#### commando's

debug [on/off]      - Schakelt live debug-uitvoer van fout, correctie en motorsnelheid in of uit
start               - Start de lijnvolger (zet robotRunning actief)
stop                - Stopt de lijnvolger onmiddellijk en zet beide motoren op 0
set cycle [µs]      - Stelt de cyclustijd van de regellus in microseconden in (bv. set cycle 10000 = 10 ms)
set power [0..255]  - Stelt het maximale motorvermogen (PWM-waarde) in
set diff [0..1]     - Stelt de vertragingsfactor voor bochten in (0 = geen vertraging, 1 = maximale vertraging)
set kp [0..]        - Stelt de proportionele versterking (Kp) van de PID-regelaar in
set ki [0..]        - Stelt de integrerende versterking (Ki) van de PID-regelaar in
set kd [0..]        - Stelt de differentiërende versterking (Kd) van de PID-regelaar in
calibrate black     - Kalibreert de zwarte lijn onder de sensoren en slaat de drempelwaarden op in EEPROM
calibrate white     - Kalibreert de witte ondergrond onder de sensoren en slaat de drempelwaarden op in EEPROM
get                 - Toont alle huidige ingestelde parameters en kalibratiewaarden
save                - Slaat de huidige parameters permanent op in het EEPROM-geheugen
reset               - Herstelt alle parameters naar de standaardwaarden

### kalibratie
De 8-kanaals TCRT5000 IR-sensorarray leest via de pinnen A5, A4, A3, A2, A1 en D7 het gereflecteerde infraroodlicht van de ondergrond. Omdat omgevingslicht en vloerreflectie per parcours verschillen, voer je vóór het rijden een tweestapskalibratie uit:

Witte ondergrond kalibreren: Plaats de robot met alle sensoren volledig boven het witte (lichte) gedeelte van het parcours en stuur het commando calibrate white. De robot leest de sensorwaarden in en bewaart deze als wit-referentie.

Zwarte lijn kalibreren: Plaats de robot met alle sensoren boven de zwarte lijn (of beweeg de sensorarray kort over de zwarte lijn) en stuur het commando calibrate black. De robot berekent automatisch het midden tussen de witte en zwarte meetwaarden als schakeldrempel en slaat deze op in het EEPROM-geheugen.

### settings

De robot rijdt stabiel met volgende parameters:
Kp: 10.0
Ki: 0.0
Kd: 2.0
Power (maxSpeed): 55 (op een schaal van 0..255)
Diff (turnSlowdown): 0.4 (op een schaal van 0..1)
Cycle (cycleTime): 10000 µs (10 ms)
Debug: off

### start/stop button
Locatie: Achteraan naast HC05 aan batterijhouder.

Werking: De knop maakt gebruik van een hardware-interrupt (FALLING edge) met een softwarematige debounce-tijd van 250 ms om contactdender te voorkomen.

Eerste druk: Als de robot stilstaat (RUN=0), wordt de PID-regelaar gereset (integral = 0, lastError = 0) en start de robot direct met het volgen van de lijn (OK,START=1).

Tweede druk: Als de robot rijdt (RUN=1), stopt de robot onmiddellijk en worden beide motoren aangestuurd met PWM 0 (OK,START=0).
