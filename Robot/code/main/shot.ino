// ==================================================
// ================= LIBRAIRIES =====================
// ==================================================

#include <Servo.h>
#include <AccelStepper.h>

// ==================================================
// ================= VIS SANS FIN ===================
// ==================================================

//pins moteur vis sans fin (L298N)
#define VIS_IN1 37
#define VIS_IN2 35
#define VIS_IN3 33
#define VIS_IN4 31

//moteur en FULL4WIRE
AccelStepper visSansFin(
  AccelStepper::FULL4WIRE,
  VIS_IN1, VIS_IN2, VIS_IN3, VIS_IN4
);

//distance de déplacement de la vis
const long STEPS_VIS = 1290;

// ==================================================
// ================== RECHARGEUR ====================
// ==================================================
#define HALFSTEP 8
//pins moteur rechargeur (L298N)
#define REC_IN1 49
#define REC_IN2 51
#define REC_IN3 45
#define REC_IN4 46

//moteur rechargeur en FULL4WIRE
AccelStepper rechargeur(
  HALFSTEP,
  REC_IN1, REC_IN3, REC_IN2, REC_IN4
);

//distance de rotation rechargeur
const long STEPS_RECHARGE = 4140;

// ==================================================
// ================= SERVO GACHETTE =================
// ==================================================

Servo FS5115M;

int servoPin = 10;

//positions servo
const int SERVO_BAS = 47;
const int SERVO_HAUT = 25;

// ==================================================
// ================= ETATS GLOBAUX ==================
// ==================================================

//nombre total de tirs effectués
int compteur_tirs = 0;

//4 tirs au total
//1 déjà dans le canon
//3 dans le rechargeur
const int MAX_TIRS = 4;

// ==================================================
// ====================== SETUP =====================
// ==================================================

void setup_tire_canon() {

  // ==================================================
  // ================= VIS SANS FIN ===================
  // ==================================================

  visSansFin.setMaxSpeed(500);
  visSansFin.setAcceleration(300);

  visSansFin.setCurrentPosition(0);

  // ==================================================
  // ================= RECHARGEUR =====================
  // ==================================================

  rechargeur.setMaxSpeed(1600);
  rechargeur.setAcceleration(1000);

  

  // ==================================================
  // ================= SERVO ==========================
  // ==================================================

  FS5115M.attach(servoPin);

  //gachette basse au début
  FS5115M.write(SERVO_BAS);
}

// ==================================================
// ======================= LOOP =====================
// ==================================================

void loop_tire_canon() {

  //si plus de munitions
  if (compteur_tirs >= MAX_TIRS) {
    return;
  }
  if (compteur_tirs ==0){
    rechargeur.move(STEPS_RECHARGE/2);
    while (rechargeur.distanceToGo() != 0) {
      rechargeur.run();
    }
  }
  // ==================================================
  // ================== TIR ===========================
  // ==================================================

  //avance de la vis
  visSansFin.moveTo(-STEPS_VIS);

  while (visSansFin.distanceToGo() != 0) {
    visSansFin.run();
  }

  delay(200);

  //monte la gachette
  FS5115M.write(SERVO_HAUT);

  delay(250);

  //retour de la vis
  visSansFin.moveTo(0);

  while (visSansFin.distanceToGo() != 0) {
    visSansFin.run();
  }

  delay(250);

  //relâche la gachette
  FS5115M.write(SERVO_BAS);

  //tir effectué
  compteur_tirs++;

  // ==================================================
  // ================= RECHARGE =======================
  // ==================================================

  //si il reste encore des tirs
  if (compteur_tirs < MAX_TIRS) {

    rechargeur.move(STEPS_RECHARGE);

    while (rechargeur.distanceToGo() != 0) {
      rechargeur.run();
    }

    delay(300);
  }
}