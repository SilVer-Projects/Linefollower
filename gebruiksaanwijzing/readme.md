# Gebruiksaanwijzing

### opladen / vervangen batterijen
De geselecteerde batterijen uit de Bill of Materials (BOM) zijn oplaadbaar en beschikken over een hoge capaciteit, wat zorgt voor een lange autonomie binnen deze toepassing. Om de batterijen na ontlading opnieuw te kunnen gebruiken, wordt aanbevolen om een compatibele batterijlader mee te bestellen, tenzij deze reeds beschikbaar is.

### draadloze communicatie
#### verbinding maken
Firmware uploaden: Zorg ervoor dat de Arduino-code met Bluetooth-functionaliteit correct is gecompileerd en geüpload naar de Arduino Nano.

Hardware aansluiten: Sluit de HC-05-module aan op de Arduino Nano. Gebruik hierbij een spanningsdeler op de RX-lijn van de module om het 5V-signaal van de Arduino veilig om te zetten naar 3,3V.

Systeem inschakelen: Schakel de voeding van de schakeling in. De status-LED op de HC-05 zal nu snel knipperen om aan te geven dat de module klaar is om te koppelen.

Applicatie installeren: Download en installeer de app Serial Bluetooth Terminal op uw smartphone.

Bluetooth koppelen: Schakel Bluetooth in op uw smartphone, zoek naar beschikbare apparaten en maak verbinding met HC-05.

Verbinding verifiëren: Open de geïnstalleerde terminal-app en selecteer de gekoppelde HC-05-module om de seriële verbinding te starten. Zodra de verbinding tot stand is gebracht, wijzigt het knipperpatroon van de status-LED op de module naar een trager interval.

Communicatie starten: De verbinding is nu actief. U kunt via de terminal rechtstreeks commando's versturen om de robot aan te sturen en parameters op te vragen of te wijzigen.

#### commando's
debug [on/off]  
start  
stop  
set cycle [µs]  
set power [0..255]  
set diff [0..1]  
set kp [0..]  
set ki [0..]  
set kd [0..]  
calibrate black  
calibrate white  

### kalibratie
uitleg kalibratie  

### settings
De robot rijdt stabiel met volgende parameters:  

### start/stop button
uitleg locatie + werking start/stop button
