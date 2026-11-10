# start/stop interrupt proof of concept
minimale hard- en software die de correcte werking van een start/stop drukknop aantoont, gebruik makend van een hardware interrupt

Functionaliteit van de knop met interrupt
Een drukknop gekoppeld aan een interrupt reageert onmiddellijk op een fysieke actie. In plaats van dat de microcontroller continu de knop hoeft te controleren (pollen), onderbreekt het elektrische signaal van de knop direct het hoofdprogramma om een actie uit te voeren. Hierdoor mist de microcontroller geen enkele knopdruk en blijft het programma efficiënt.

Werking in de code
De code koppelt de knoppin rechtstreeks aan een interrupt-functie (attachInterrupt) die wordt getriggerd bij een neergaande flank (wanneer de knop wordt ingedrukt). Omdat een knop fysiek kan "denderen" (snel achter elkaar aan/uit springen), wordt binnen de interrupt softwarematige debounce toegepast via een tijdscheck (millis()). Vervolgens wordt een statusvlag omgedraaid (isRunning) en geeft de interrupt een seintje aan de hoofdloop om de statuswijziging (zoals het aansturen van de LED of het tonen van een melding) netjes af te handelen.
