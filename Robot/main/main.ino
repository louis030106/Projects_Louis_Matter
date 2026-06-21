// ======================================================
// ===================== BIBLIOTHÈQUES ==================
// ======================================================
#include "CytronMotorDriver.h"
#include <Wire.h>
#include <Servo.h>

// ======================================================
// ===================== DRIVERS MOTEURS =================
// ======================================================
CytronMD motorD(PWM_PWM, 9, 8);
CytronMD motorG(PWM_PWM, 5, 4);

// ======================================================
// ===================== SERVO ToF =======================
// ======================================================
Servo myservo;
int pos = 0;
int sens = 1;
unsigned long lastServoMove = 0;
const int servoInterval = 200;
int angle_servo;

// ======================================================
// ===================== ENCODEURS =======================
// ======================================================
#define ENC1_D 20
#define ENC2_D 21
#define ENC1_G 18
#define ENC2_G 19

volatile long countD = 0;
volatile long countG = 0;

// ======================================================
// ===================== PARAMÈTRES PHYSIQUES ============
// ======================================================
const float RAYON_ROUE = 0.0325;
const float ENTRE_AXES = 0.12;
const int TICKS_PAR_TOUR = 970;
int OFFSET = 7;

// ======================================================
// ===================== ODOMÉTRIE =======================
// ======================================================
float x = 0.0;
float y = 0.0;
float theta = PI/2;
long prevCountD = 0;
long prevCountG = 0;

// ======================================================
// ===================== CAPTEUR ToF =====================
// ======================================================
char tofbuffer[10];
byte idx = 0;
int distanceToF = 0;

// ======================================================
// ===================== COULEURS ========================
// ======================================================
bool isBlue_D;
bool isBlue_G;
bool isYellow_D;
bool isYellow_G;
bool blue;
int nonJauneCount = 0;

// ======================================================
// ===================== PONT ============================
// ======================================================
bool surPont = false;
bool Fin_Pont = false;
float distance_mm;
// ======================================================
// ===================== TIMING GÉNÉRAL ==================
// ======================================================
unsigned long top_depart;
unsigned long time_aff = 0;
unsigned long temps_max = 170000;
bool run = true;

// ======================================================
// ===================== CANON ===========================
// ======================================================
int tire_obus = 0;
unsigned long temps_depasse_tir = 20000;
unsigned long temps_att_tir = 0;
bool attente_encours_tir = true;

// ======================================================
// ===================== ÉTATS ROBOT =====================
// ======================================================
enum EtatRobot {
  IDLE,
  DEBUT,
  AVANCE_COURT,
  TOURNE_ROUE,
  TOURNE_VERS,
  VA_VERS,
  RAPPROCHEPONT,
  AJUSTE2CM,
  ORIENTATION1,
  ORIENTATION2,
  ORIENTATION3,
  ORIENTATION4,
  RECULER,
  TOURNER_VERS_PONT,
  AVANCER_VERS_FIN_PONT,
  TOURNER_SUR_PONT,
  AVANCER_SUR_PONTCAPTEUR,
  AVANCER_PONT,
  ORIENTATION_APRES_PONT,
  AVANCER_BOUTEILLE,
  FINI
};

EtatRobot etat = IDLE;

// ======================================================
// ===================== INTERRUPTIONS ===================
// ======================================================
void readEncoderD() {
  if (digitalRead(ENC2_D) == HIGH) countD++;
  else countD--;
}

void readEncoderG() {
  if (digitalRead(ENC2_G) == HIGH) countG++;
  else countG--;
}

// ======================================================
// ===================== ODOMÉTRIE =======================
// ======================================================
void updateOdometrie(long deltaD, long deltaG) {
  float distD = (deltaD * 2.0 * PI * RAYON_ROUE) / TICKS_PAR_TOUR;
  float distG = (deltaG * 2.0 * PI * RAYON_ROUE) / TICKS_PAR_TOUR;

  float dC = (distD + distG) / 2.0;
  float dTheta = (distD - distG) / ENTRE_AXES;

  theta += dTheta;
  while (theta > PI) theta -= 2*PI;
  while (theta < -PI) theta += 2*PI;

  x += dC * cos(theta);
  y += dC * sin(theta);
}

// ======================================================
// ===================== MOTEURS =========================
// ======================================================
void stopMoteurs() {
  motorD.setSpeed(0);
  motorG.setSpeed(0);
}

bool tournerUneRoueNonBloquant(float angleCible, int pwm) {
  long deltaD = countD - prevCountD;
  long deltaG = countG - prevCountG;
  prevCountD = countD;
  prevCountG = countG;
  updateOdometrie(deltaD, deltaG);

  float erreur = angleCible - theta;
  while (erreur > PI) erreur -= 2*PI;
  while (erreur < -PI) erreur += 2*PI;

  if (fabs(erreur) < 0.01) {
    stopMoteurs();
    return true;
  }

  if (erreur > 0) {
    motorD.setSpeed(pwm);
    motorG.setSpeed(0);
  } else {
    motorG.setSpeed(pwm);
    motorD.setSpeed(0);
  }

  return false;
}

bool tournerVersNonBloquant(float angleCible, int pwmMax) {
  float erreur = angleCible - theta;

  while (erreur > PI) erreur -= 2*PI;
  while (erreur < -PI) erreur += 2*PI;

  if (fabs(erreur) < 0.01) {
    stopMoteurs();
    return true;
  }

  if (erreur > 0) {
    motorG.setSpeed(-pwmMax);
    motorD.setSpeed(pwmMax);
  } else {
    motorD.setSpeed(-pwmMax);
    motorG.setSpeed(pwmMax);
  }

  return false;
}

// ======================================================
// ===================== AVANCER / RECULER ===============
// ======================================================
bool avancerActif = false;
long ticksCible = 0;
long startD = 0;
long startG = 0;

bool avancerDroit(float distance_m, int pwm){
  long deltaD = countD - prevCountD;
  long deltaG = countG - prevCountG;
  prevCountD = countD;
  prevCountG = countG;
  updateOdometrie(deltaD, deltaG);

  if (!avancerActif) {
    ticksCible = (distance_m / (2.0 * PI * RAYON_ROUE)) * TICKS_PAR_TOUR;
    startD = countD;
    startG = countG;
    avancerActif = true;
  }

  long dD = countD - startD;
  long dG = countG - startG;

  if (dD >= ticksCible && dG >= ticksCible) {
    stopMoteurs();
    avancerActif = false;
    return true;
  }

  long erreur = dD - dG;
  int correction = erreur * 0.5;

  motorD.setSpeed(pwm - correction);
  motorG.setSpeed(pwm + correction);

  return false;
}

bool reculerDroit(float distance_m, int pwm){
  long deltaD = countD - prevCountD;
  long deltaG = countG - prevCountG;
  prevCountD = countD;
  prevCountG = countG;
  updateOdometrie(deltaD, deltaG);

  if (!avancerActif) {
    ticksCible = (distance_m / (2.0 * PI * RAYON_ROUE)) * TICKS_PAR_TOUR;
    startD = countD;
    startG = countG;
    avancerActif = true;
  }

  long dD = abs(countD - startD);
  long dG = abs(countG - startG);

  if (dD >= ticksCible && dG >= ticksCible) {
    stopMoteurs();
    avancerActif = false;
    return true;
  }

  motorD.setSpeed(-pwm);
  motorG.setSpeed(-pwm);

  return false;
}

// ======================================================
// ===================== SERVO ToF =======================
// ======================================================
void ServoMoteur_rotation() {
  if (millis() - lastServoMove < servoInterval) return;
  lastServoMove = millis();

  pos += sens;

  if (pos >= 25) { pos = 25; sens = -1; }
  if (pos <= 1)  { pos = 1;  sens = 1; }

  angle_servo = pos - 13;
  myservo.write(pos);
}

bool tourner_capteur_jaune(int pwm){
  long deltaD = countD - prevCountD;
  long deltaG = countG - prevCountG;
  prevCountD = countD;
  prevCountG = countG;
  updateOdometrie(deltaD, deltaG);
  if (!isYellow_G){
    motorG.setSpeed(pwm);   // roue gauche avance
    motorD.setSpeed(-pwm+OFFSET);     // roue droite arrêtée
    return false;
  }
  else{
    return true;
  }
}

// ======================================================
// ===================== LECTURE ToF =====================
// ======================================================
void Lecture_ToF() {
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '<') {
      idx = 0;
      tofbuffer[0] = '\0';
    }
    else if (c == '>') {
      tofbuffer[idx] = '\0';
      distanceToF = atoi(tofbuffer);
      idx = 0;
    }
    else if (idx < sizeof(tofbuffer) - 1) {
      tofbuffer[idx++] = c;
    }
  }
}

bool avancer_pont_capteur(int pwm){
  long deltaD = countD - prevCountD;
  long deltaG = countG - prevCountG;
  prevCountD = countD;
  prevCountG = countG;
  updateOdometrie(deltaD, deltaG);

  // Les deux voient jaune → on avance, on reset le compteur
  if (isYellow_D && isYellow_G){
    nonJauneCount = 0;
    motorD.setSpeed(pwm - OFFSET);
    motorG.setSpeed(pwm);
    return false;   // on continue
  }

  // Au moins un ne voit plus jaune → on compte
  nonJauneCount++;

  // Petit trou (1–2 ms) → on tolère, on continue d’avancer
  if (nonJauneCount < 5) {   // ajuste 5 si tu veux plus/moins sensible
    motorD.setSpeed(pwm - OFFSET);
    motorG.setSpeed(pwm);
    return false;
  }

  // Plus vraiment sur le jaune → on s’arrête
  return true;
}

// ======================================================
// ===================== MACHINE D’ÉTAT ==================
// ======================================================
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  myservo.attach(11);
  myservo.write(13);

  pinMode(ENC1_D, INPUT_PULLUP);
  pinMode(ENC2_D, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC1_D), readEncoderD, RISING);

  pinMode(ENC1_G, INPUT_PULLUP);
  pinMode(ENC2_G, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC1_G), readEncoderG, RISING);

  init_setup_couleur();
  init_setup_ultrason();
  setup_tire_canon();

  top_depart = millis();
  etat = DEBUT;
}

void loop() {
  if (!run) { stopMoteurs(); return; }

  loop_lire_couleur();
  Lecture_ToF();

  while (distanceToF <= 150 && !surPont) {
    Lecture_ToF();
    stopMoteurs();
    if (millis() - top_depart >= temps_max) { run = false; break; }
  }

  switch (etat) {

    case DEBUT:
      etat = AVANCE_COURT;
      break;

    case AVANCE_COURT:
      if (avancerDroit(0.12, 80)) etat = TOURNE_VERS;
      break;

    case TOURNE_ROUE:
      if (tournerUneRoueNonBloquant(PI/3, 90)) etat = TOURNE_VERS;
      break;

    case TOURNE_VERS:
      if (tournerVersNonBloquant(-0.17, 110)) etat = VA_VERS;
      break;

    case VA_VERS:
      if (avancerDroit(0.55, 90)) etat = RAPPROCHEPONT;
      break;

    case RAPPROCHEPONT:
      if (avancerDroit(0.3, 70) || blue) {
        stopMoteurs();
        avancerActif = false;
        etat = AJUSTE2CM;
      }
      break;

    case AJUSTE2CM:
      if (avancerDroit(0.01, 70)) {
        stopMoteurs();
        avancerActif = false;
        theta = 0;
        countD = countG = prevCountD = prevCountG = 0;
        etat = ORIENTATION1;
      }
      break;

    case ORIENTATION1:
      if (tournerVersNonBloquant(-PI/15, 100)) {
        if (distanceToF >= 400) loop_tire_canon();
        etat = ORIENTATION2;
      }
      break;

    case ORIENTATION2:
      if (tournerVersNonBloquant(-PI/12, 100)) {
        if (distanceToF >= 400) loop_tire_canon();
        etat = ORIENTATION3;
      }
      break;

    case ORIENTATION3:
      if (tournerVersNonBloquant(-PI/20, 100)) {
        if (distanceToF >= 400) loop_tire_canon();
        etat = ORIENTATION4;
      }
      break;

    case ORIENTATION4:
      if (tournerUneRoueNonBloquant(-PI/25, 100)) {
        if (distanceToF >= 400) loop_tire_canon();
        etat = TOURNER_VERS_PONT;
      }
      break;

    case TOURNER_VERS_PONT:
      if (tournerVersNonBloquant(PI/2+0.42, 100)) etat = AVANCER_VERS_FIN_PONT;
      break;

    case AVANCER_VERS_FIN_PONT:
      if (avancerDroit(0.28, 90) || Fin_Pont) {
        stopMoteurs();
        Fin_Pont = true;
        loop_ultrason();
        if (distance_mm > 350) {
          avancerActif = false;
          etat = TOURNER_SUR_PONT;
        }
      }
      break;

    case TOURNER_SUR_PONT:
      if (tourner_capteur_jaune(110)) {
        nonJauneCount = 0;
        etat = AVANCER_SUR_PONTCAPTEUR;
      }
      break;

    case AVANCER_SUR_PONTCAPTEUR:
      surPont = true;
      if (distanceToF <= 150) etat = RECULER;
      if (avancer_pont_capteur(80)) {
        stopMoteurs();
        avancerActif = false;
        theta = 0;
        countD = countG = prevCountD = prevCountG = 0;
        etat = AVANCER_PONT;
      }
      break;

    case RECULER:
      if (reculerDroit(0.05, 110)) etat = FINI;
      break;

    case AVANCER_PONT:
      surPont = true;
      if (distanceToF <= 150) etat = RECULER;
      if (avancerDroit(0.31, 90)) etat = ORIENTATION_APRES_PONT;
      break;

    case ORIENTATION_APRES_PONT:
      if (tournerVersNonBloquant(-PI/7, 110)) {
        etat = AVANCER_BOUTEILLE;
      }
      break;

    case AVANCER_BOUTEILLE:
      if (avancerDroit(0.65, 100) || millis() - top_depart >= 10000) etat = FINI;
      break;

    case FINI:
      stopMoteurs();
      break;

    default:
      break;
  }

  long deltaD = countD - prevCountD;
  long deltaG = countG - prevCountG;
  prevCountD = countD;
  prevCountG = countG;
  updateOdometrie(deltaD, deltaG);

  if (millis() - top_depart >= temps_max) run = false;
}
