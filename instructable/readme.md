# Instructable

Een instructable is een stappenplan - zonder verdere uitleg - hoe je vertrekkend van de bill of materials en gebruik makend van de technische tekeningen de robot kan nabouwen. Ook de nodige stappen om de microcontroller te compileren en te uploaden staan beschreven.  

1. Bestel alle onderdelen uit de Bill of Materials: 1x Bluetooth Transceiver (HC06), 2x Rubberen Wiel (N20 D-Gat 43mm), 2x Microcontroller (Arduino Nano met CH340 USB-driver 16MHz ATMEGA328P), 2x Driver (TB6612 DRV8833 DC Motor Driver Board Module Dual Motor Driver 1A), 1x Protoboards (Groene PCB Dual Panel Board DIY), 2x DC motor (300tpm, 6V), 2x Sensor (8 kanaals TCRT5000 IR Foto-elektrische Schakelaar), 1x Dupont kabels, 1x Batterij (LiitoKala King4000 18650 4000mAh - 2pcs), 1x Batterijhouder (2x 3.7V) en 1x Tuimelschakelaar.
2. Raadpleeg de mechanische tekeningen en print het chassis of maak het zelf uit plaatmateriaal met behoud van de exacte posities voor de motoren, batterijhouder en sensoren zoals aangegeven op de tekening.
3. Zaag, boor of 3D-print het chassis volgens de mechanische tekeningen zodat de bevestigingspunten voor de motoren, batterijhouder en sensoren exact op de voorziene posities liggen.
4. Bevestig de 2x DC-motoren (300tpm, 6V) en de 2x N20 D-Gat 43mm rubberen wielen op de exacte motorpositie van de mechanische tekening.
5. Monteer de 8-kanaals IR-lijnsensor array op de exacte sensorpositie aan de voorzijde van het chassis volgens de mechanische tekening.
6. Bouw de schakeling op een breadboard op volgens het elektrisch schema (Arduino Nano ATmega328P, DRV8833 motordriver, IR-sensor 8-kanaals, Bluetooth-module, drukknop, spanningsdeler met R1=1kΩ en R2=2kΩ, batterijhouder 2x 3,7V en tuimelschakelaar) en controleer alle verbindingen.
7. Open de Arduino IDE en selecteer bij Bord: `Arduino Nano`.
8. Installeer de benodigde bibliotheken in de Arduino IDE en open de Proof of Concept-testcodes om elk onderdeel afzonderlijk te compileren en te uploaden naar de Arduino Nano.
9. Test elk onderdeel op het breadboard met de Proof of Concept-testcodes en controleer de seriële monitor en werking van sensoren, motoren, drukknop en Bluetooth-module.
10. Soldeer de definitieve elektronische schakeling volgens het schema op het protoboard (inclusief R1=1kΩ en R2=2kΩ op RXD/GND) en meet met een multimeter op kortsluiting en continuïteit.
11. Monteer het protoboard, de batterijhouder (2x 3,7V met 2x 18650-batterijen) op de exacte positie van de mechanische tekening, en bevestig de tuimelschakelaar en de Bluetooth-module stevig op het chassis.
12. Sluit alle kabels en pinnen aan volgens het schema: Zie technische tekeningen -> Elektrisch_Schema_2
13. Open het basisprogramma (`basis.ino`) in de Arduino IDE, controleer de pin-definities en bibliotheken, en klik op **Verifieren/Compileren** om te controleren op fouten.
14. Klik in de Arduino IDE op **Uploaden** om de code via de CH340-poort naar de Arduino Nano (ATmega328P) te schrijven en wacht tot het uploaden is voltooid.
15. Plaats de twee 18650-batterijen in de batterijhouder en schakel de tuimelschakelaar in om het systeem van spanning te voorzien.
16. Controleer of de voedings- en statusleds van de Arduino Nano, de motordriver, de IR-sensor en de Bluetooth-module oplichten zonder overmatige opwarming of geur.
17. Open op de smartphone de app *Serial Bluetooth Terminal*, koppel met de Bluetooth-module en maak verbinding met de seriële poort.
18. Verstuur via *Serial Bluetooth Terminal* de kalibratiecommando's om de wit- en zwartwaarden van de IR-sensoren in te lezen en op te slaan.
19. Stel via de seriële verbinding de regeling- en rijparameters (`Kp`, `Ki`, `Kd`, basissnelheid en cyclustijd) in.
20. Plaats de robot op het parcours, start de rit via de drukknop of het terminalcommando "START" en stel `Kp`, `Ki`, `Kd`, snelheid en cyclustijd bij op basis van het rijgedrag.





