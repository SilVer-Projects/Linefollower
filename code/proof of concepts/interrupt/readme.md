##start/stop interrupt proof of concept##
minimale hard- en software die de correcte werking van een start/stop drukknop aantoont, gebruik makend van een hardware interrupt

---

##Functionaliteit van de knop met interrupt##
Een drukknop op pin D2 (INPUT_PULLUP) gekoppeld aan een hardware interrupt reageert onmiddellijk op een fysieke actie. In plaats van dat de microcontroller continu de knop hoeft te controleren (pollen), onderbreekt de neergaande flank (FALLING) bij het indrukken van de knop direct het hoofdprogramma om een seintje te geven. Hierdoor mist de microcontroller geen enkele knopdruk, blijft de interrupt zelf kort en veilig, en handelt de hoofdloop de start/stop-actie en motorstop netjes af.

---

##Werking in de code##
De code koppelt pin D2 (INPUT_PULLUP) via attachInterrupt aan de interrupt-functie buttonISR() die wordt getriggerd bij een neergaande flank (FALLING, wanneer de knop wordt ingedrukt). Om de interrupt zelf kort en veilig te houden, zet buttonISR() enkel een volatile vlag (buttonInterruptFlag) omhoog. In de hoofdloop wordt deze vlag veilig gereset en wordt softwarematige debounce toegepast via een tijdscheck (millis()) en een controle op LOW om fysiek denderen of ruis op te vangen. Vervolgens wordt de status (robotRunning) omgedraaid, de PID gereset, de motoren bij uitschakelen meteen stilgezet en de nieuwe status via Bluetooth teruggestuurd.
