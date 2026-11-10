# H-Bridge proof of concept

minimale hard- & software + stappenplan dat aantoont dat 2 motoren onafhankelijk van elkaar kunnen draaien, en (traploos) regelbaar zijn in snelheid en draairichting.

---

## **Functionaliteit van de driver (DRV8833)**
* De driver stuurt twee motoren aan via een H-brug per motor (twee pinnen per motor). Door een PWM-signaal (Pulse-Width Modulation) te combineren met een `LOW`-signaal, kun je nauwkeurig het toerental regelen en bepalen of de motor vooruit of achteruit draait.

---

## **Werking in de code**
* De code stelt vier pinnen in als uitgang voor de driver. Om **verschillende toerentallen** aan te tonen, verander je de PWM-waarde (de getallen in het signaal) zodat de ene motor sneller draait dan de andere. Om **twee richtingen** aan te tonen, wissel je in de code welke pin het PWM-signaal krijgt en welke pin `LOW` is, waardoor de stroomrichting en dus de draairichting omkeert.
