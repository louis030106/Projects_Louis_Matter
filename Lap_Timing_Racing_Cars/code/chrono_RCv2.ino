//10M58 circuit longueur

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// Remplacement des objets LCD
hd44780_I2Cexp lcd1;
hd44780_I2Cexp lcd2;

// ===================== CONFIGURATION =====================
const int trigPinA = 12;
const int echoPinA = 11;
const int trigPinB = 9;
const int echoPinB = 10;
const float longueur_circuit = 10.58;

const int distance_obj_max = 60; // cm
const unsigned long minChrono = 300; // ms

// ===================== VARIABLES =====================
long durationA, distanceA;
long durationB, distanceB;

unsigned long time_max;

enum EtatChrono { ATTENTE, EN_COURS };

// Chrono A
EtatChrono etatA = ATTENTE;
unsigned long startTimeA = 0;
float timeA = 0.0f;
float best_timeA = 0.0f;
float best_vitesseA = 0.0f;
float best_vitesseB = 0.0f;
bool detectedA_prev = false;

// Chrono B
EtatChrono etatB = ATTENTE;
unsigned long startTimeB = 0;
float timeB = 0.0f;
float best_timeB = 0.0f;
float vitesseA = 0.0F;
float vitesseB = 0.0F;
bool detectedB_prev = false;

// ===================== FONCTIONS =====================
inline void ping(int trigPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}

// ===================== SETUP =====================
void setup() {
  Wire.begin();
  Wire.setClock(50000);

  lcd2.begin(16, 2, 0x22); // lcd1 prend l’adresse de l’ancien lcd
  lcd2.backlight();
  delay(50);              // IMPORTANT

  lcd1.begin(16, 2, 0x27);
  lcd1.backlight();
  delay(50);  

  lcd1.clear();
  lcd1.setCursor(0,0);
  lcd1.print("Ecran 1 actif");

  delay(10);              // laisse respirer le bus

  lcd2.clear();
  lcd2.setCursor(0,0);
  lcd2.print("Ecran 2 actif");

  pinMode(trigPinA, OUTPUT);
  pinMode(echoPinA, INPUT);

  pinMode(trigPinB, OUTPUT);
  pinMode(echoPinB, INPUT);

  time_max = (unsigned long)(2.0 * distance_obj_max / 0.0343);
}

// ===================== LOOP =====================
void loop() {

  // ===================== CAPTEUR A =====================
  ping(trigPinA);
  durationA = pulseIn(echoPinA, HIGH, time_max);
  distanceA = durationA / 58.2;

  bool detectedA = (distanceA >= 1 && distanceA <= 5);

  if (detectedA && !detectedA_prev) { // FRONT DE PASSAGE
    unsigned long now = millis();

    switch (etatA) {

      case ATTENTE:
        startTimeA = now;
        etatA = EN_COURS;
        lcd1.clear();
        lcd1.print("[A] TOP depart ");
        break;

      case EN_COURS: {
        unsigned long chrono = now - startTimeA;

        if (chrono >= minChrono) {
          timeA = chrono * 0.001f;

          if (best_timeA == 0.0f || timeA < best_timeA) {
            best_timeA = timeA;
          }

          lcd1.setCursor(0,1);
          lcd1.print("LAP:");
          lcd1.setCursor(4,1);
          lcd1.print(timeA,3);

          lcd1.setCursor(11,1);
          lcd1.print(best_timeA,3);
          lcd1.setCursor(15,1);
          lcd1.print(" ");

          startTimeA = now;
          vitesseA = (longueur_circuit/timeA) * 3.6;
          if (best_vitesseA == 0.0f || vitesseA > best_vitesseA) {
            best_vitesseA = vitesseA;
          }

          lcd1.setCursor(0,0);
          lcd1.print("speed:          ");
          lcd1.setCursor(6,0);
          lcd1.print(vitesseA, 1);
          lcd1.setCursor(11,0);
          lcd1.print(best_vitesseA, 2);
        }
        break;
      }
    }
  }

  detectedA_prev = detectedA;

  delay(10);

  // ===================== CAPTEUR B =====================
  ping(trigPinB);
  durationB = pulseIn(echoPinB, HIGH, time_max);
  distanceB = durationB / 58.2;

  bool detectedB = (distanceB >= 1 && distanceB <= 5);

  if (detectedB && !detectedB_prev) { // FRONT DE PASSAGE
    unsigned long now = millis();

    switch (etatB) {

      case ATTENTE:
        startTimeB = now;
        etatB = EN_COURS;
        lcd2.clear();
        lcd2.print("[B] TOP depart ");
        break;

      case EN_COURS: {
        unsigned long chrono = now - startTimeB;

        if (chrono >= minChrono) {
          timeB = chrono * 0.001f;

          if (best_timeB == 0.0f || timeB < best_timeB) {
            best_timeB = timeB;
          }

          lcd2.setCursor(0,1);
          lcd2.print("LAP:");
          lcd2.setCursor(4,1);
          lcd2.print(timeB,3);

          lcd2.setCursor(11,1);
          lcd2.print(best_timeB,3);
          lcd2.setCursor(15,1);
          lcd2.print(" ");

          startTimeB = now;
          vitesseB = (longueur_circuit/timeB) * 3.6;
          if (best_vitesseB == 0.0f || vitesseB > best_vitesseB) {
            best_vitesseB = vitesseB;
          }

          lcd2.setCursor(0,0);
          lcd2.print("speed:          ");
          lcd2.setCursor(6,0);
          lcd2.print(vitesseB, 1);
          lcd2.setCursor(11,0);
          lcd2.print(best_vitesseB, 2);
        }
        break;
      }
    }
  }

  detectedB_prev = detectedB;

  delay(10);
}
