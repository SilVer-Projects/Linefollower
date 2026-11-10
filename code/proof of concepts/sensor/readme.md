# Sensoren proof of concept

minimale hard- en software die aantoont dat minimaal 6 sensoren onafhankelijk van elkaar kunnen uitgelezen worden (geen calibratie, normalisatie of interpolatie). Hierbij moet een zo groot mogelijk bereik van de AD converter benut worden (indien van toepassing)
---

### Functionaliteit 

*Een 8-kanaals infraroodsensorarray wordt gebruikt om de positie van een lijn (bijvoorbeeld bij een linefollower) nauwkeurig te detecteren op basis van het verschil in reflectie tussen een zwarte lijn en een witte ondergrond. Door meerdere sensoren te combineren, weet de robot exact waar de lijn zich bevindt ten opzichte van het voertuig. De voedingspin (powerPin) zorgt ervoor dat de sensormodule centraal van stroom kan worden voorzien.
---
### Werking in de code

*De code maakt gebruik van een array (sensorPins) om meerdere sensorpinnen in één handeling te configureren als ingang (INPUT) en vervolgens efficiënt uit te lezen in een for-loop. In de setup wordt de voedingspin permanent op HIGH gezet om de sensoren in te schakelen. In de loop worden alle sensoren opeenvolgend uitgelezen met digitalRead(), waarna de gemeten status per pin (gekoppeld aan de naam uit pinNames) naar de Serial Monitor wordt gestuurd met een pauze van 5 seconden.
