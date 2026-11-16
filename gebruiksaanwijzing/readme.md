# Gebruiksaanwijzing

### opladen / vervangen batterijen
De batterijen aangekocht uit de BOM zijn batterijen die oplaadbaar zijn en met grote capaciteit, je kan er lang mee doen voor ze in deze toepassing opgeladen moeten worden, wel aangeraden in het geval dat ze leeg zijn om oplader voor deze batterijen ook aan te kopen tenzij je deze al in huis hebt.

### draadloze communicatie
#### verbinding maken
Verbinden maken is mogelijk door de HC05, dit gebeurt via bluetooth en doe je best via je smartphone:
1. Zorg dat de code met HC05 functies opgeladen is naar de arduino nano
2. Sluit de HC05 correct aan op de arduino nano + spanningsdeler
3. Bij het aanzetten van de schakeling zal een lampje beginnen knipperen op de HC05
4. Download de app Serial Bluetooth Terminal op je smartphone
5. Zet je bluetooth aan, zoek in devices HC05 en koppel er mee
6. Het HC05 lampje begint nu aan een ander interval te knipper
7. Je bent nu verbonden en kunt Commando's versturen en parameters ontvangen

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
