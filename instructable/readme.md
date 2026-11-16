# Instructable

Een instructable is een stappenplan - zonder verdere uitleg - hoe je vertrekkend van de bill of materials en gebruik makend van de technische tekeningen de robot kan nabouwen. Ook de nodige stappen om de microcontroller te compileren en te uploaden staan beschreven.  

### Stap 1: Componenten aanschaffen
Bestel alle benodigde onderdelen volgens de opgestelde Bill of Materials (BOM).

### Stap 2: Prototyping & Proof of Concept
Bouw de schakeling tijdelijk op een breadboard en verifieer de basiswerking van elk component met behulp van de Proof of Concept-testcodes.

### Stap 3: Permanente elektronica realiseren
Soldeer de definitieve elektronische schema’s op protoboards en controleer alle verbindingen op kortsluiting en continuïteit.

### Stap 4: Chassis ontwerpen en bouwen
Construeer een stabiel frame voor de robot. Plaats de lijnsensoren hierbij zo dicht mogelijk bij de wielbasis voor een optimale stuurrespons.

### Stap 5: Assemblage en bedrading
Monteer de motoren, sensoren, voeding en elektronica stevig op het frame en sluit alles aan op de Arduino Nano.

### Stap 6: Finale firmware uploaden
Laad de definitieve besturingssoftware (inclusief PID-regeling en Bluetooth-communicatie) op naar de Arduino.

### Stap 7: Voeding en basiscontrole
Schakel de voeding in en controleer of alle modules (Arduino, sensoren, motordriver en HC-05) correct opstarten.

### Stap 8: Draadloze verbinding opzetten
Maak via Bluetooth verbinding tussen de smartphone (*Serial Bluetooth Terminal*) en de HC-05-module.

### Stap 9: Sensorkalibratie en PID-tuning
Kalibreer de zwart- en witwaarden van de sensoren via de terminal en stel de parameters (`Kp`, `Ki`, `Kd`, snelheid en cyclustijd) nauwkeurig af.

### Stap 10: Eindtest op het parcours
Start de robot en test de volledige rijprestaties op het parcours; stuur de parameters indien nodig nog licht bij voor maximale snelheid en stabiliteit.







