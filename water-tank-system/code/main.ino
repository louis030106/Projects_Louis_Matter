#include <avr/wdt.h>  // bibliothèque watchdog

const int potentiometre = A1;
const int relai = 2;
const int led = 12;
const int lecturcapteur = 7;

int pot_val = 0;
int etatflotteur = 0;
unsigned long attente = 1000;

unsigned long startTime = 0;
bool attenteEnCours = false;

void setup() {
  // Initialisation du watchdog avant tout blocage possible
  wdt_enable(WDTO_4S); // redémarre si bloqué plus de 4 secondes
  
  pinMode(potentiometre, INPUT);
  pinMode(lecturcapteur, INPUT);
  pinMode(relai, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(relai, LOW);
  digitalWrite(led, LOW);
}

void loop() {
  // Réinitialisation du watchdog pour éviter un reset
  wdt_reset();

  pot_val = analogRead(potentiometre);
  etatflotteur = digitalRead(lecturcapteur);

  // Convertir la valeur du potentiomètre en délai entre 1 et 15 minutes
  attente = map(pot_val, 0, 1023, 60000, 900000); // ms

  // Si le flotteur est activé et qu'on n'a pas encore lancé le timer
  if (etatflotteur == 1 && !attenteEnCours) {
    startTime = millis();
    attenteEnCours = true;
  }

  // Si le délai est écoulé
  if (attenteEnCours && millis() - startTime >= attente) {
    etatflotteur = digitalRead(lecturcapteur); // relire le capteur
    if (etatflotteur == 1) {
      digitalWrite(relai, HIGH);
      digitalWrite(led, HIGH);
    } else {
      digitalWrite(relai, LOW);
      digitalWrite(led, LOW);
    }
    attenteEnCours = false;
  }

  // Si le flotteur est désactivé et qu'on n'attend pas
  if (etatflotteur == 0) {
    digitalWrite(relai, LOW);
    digitalWrite(led, LOW);
  }

  delay(100); // petit délai pour éviter de saturer le CPU
  wdt_reset(); // on le met ici aussi par sécurité
}
