// ===== CAPTEUR DROITE =====
#define S0_D 26
#define S1_D 28
#define S2_D 30
#define S3_D 32
#define OUT_D 3   // INT1

// ===== CAPTEUR GAUCHE =====
#define S0_G 44
#define S1_G 42
#define S2_G 40
#define S3_G 38
#define OUT_G 2   // INT0

extern bool isBlue_D;
extern bool isBlue_G;
extern bool isYellow_D;
extern bool isYellow_G;
extern bool blue;
// ===== CALIBRATION =====
int redMin_D = 64,   redMax_D = 1952;
int greenMin_D = 88, greenMax_D = 2388;
int blueMin_D = 64,  blueMax_D = 1944;

int redMin_G = 64,   redMax_G = 1952;
int greenMin_G = 88, greenMax_G = 2388;
int blueMin_G = 64,  blueMax_G = 1944;

// ===== VARIABLES =====
volatile unsigned long lastRise_D = 0, pw_D = 0;
volatile unsigned long lastRise_G = 0, pw_G = 0;

int redPW_D, greenPW_D, bluePW_D;
int redPW_G, greenPW_G, bluePW_G;

int redValue_D, greenValue_D, blueValue_D;
int redValue_G, greenValue_G, blueValue_G;

unsigned long timerColor = 0;
byte stateColor = 0; // 0=RED, 1=GREEN, 2=BLUE



// ===== INTERRUPTIONS =====
void isr_D() {
  unsigned long now = micros();
  pw_D = now - lastRise_D;
  lastRise_D = now;
}

void isr_G() {
  unsigned long now = micros();
  pw_G = now - lastRise_G;
  lastRise_G = now;
}

void setFilter(byte state) {
  switch(state) {
    case 0: // RED
      digitalWrite(S2_D, LOW);  digitalWrite(S3_D, LOW);
      digitalWrite(S2_G, LOW);  digitalWrite(S3_G, LOW);
      break;

    case 1: // GREEN
      digitalWrite(S2_D, HIGH); digitalWrite(S3_D, HIGH);
      digitalWrite(S2_G, HIGH); digitalWrite(S3_G, HIGH);
      break;

    case 2: // BLUE
      digitalWrite(S2_D, LOW);  digitalWrite(S3_D, HIGH);
      digitalWrite(S2_G, LOW);  digitalWrite(S3_G, HIGH);
      break;
  }
}
long temps = 0;

void init_setup_couleur(){
  pinMode(S0_D, OUTPUT);
  pinMode(S1_D, OUTPUT);
  pinMode(S2_D, OUTPUT);
  pinMode(S3_D, OUTPUT);
  pinMode(OUT_D, INPUT);

  // GAUCHE
  pinMode(S0_G, OUTPUT);
  pinMode(S1_G, OUTPUT);
  pinMode(S2_G, OUTPUT);
  pinMode(S3_G, OUTPUT);
  pinMode(OUT_G, INPUT);

  // ===== MODE 2% =====
  digitalWrite(S0_D, LOW); digitalWrite(S1_D, HIGH);
  digitalWrite(S0_G, LOW); digitalWrite(S1_G, HIGH);

  // ===== INTERRUPTIONS =====
  attachInterrupt(digitalPinToInterrupt(OUT_D), isr_D, RISING);
  attachInterrupt(digitalPinToInterrupt(OUT_G), isr_G, RISING);
}

void loop_lire_couleur(){
      // ===== MACHINE D'ÉTAT COULEUR =====
    if (millis() - timerColor > 3) {  // 3 ms par couleur
      timerColor = millis();

      if (stateColor == 0) { // RED
        redPW_D = pw_D;
        redPW_G = pw_G;
      }
      else if (stateColor == 1) { // GREEN
        greenPW_D = pw_D;
        greenPW_G = pw_G;
      }
      else if (stateColor == 2) { // BLUE
        bluePW_D = pw_D;
        bluePW_G = pw_G;

        // ===== MAPPING =====
        redValue_D   = constrain(map(redPW_D,   redMin_D,   redMax_D,   255, 0), 0, 255);
        greenValue_D = constrain(map(greenPW_D, greenMin_D, greenMax_D, 255, 0), 0, 255);
        blueValue_D  = constrain(map(bluePW_D,  blueMin_D,  blueMax_D,  255, 0), 0, 255);

        redValue_G   = constrain(map(redPW_G,   redMin_G,   redMax_G,   255, 0), 0, 255);
        greenValue_G = constrain(map(greenPW_G, greenMin_G, greenMax_G, 255, 0), 0, 255);
        blueValue_G  = constrain(map(bluePW_G,  blueMin_G,  blueMax_G,  255, 0), 0, 255);

        // ===== RATIOS =====
        float sumD = redValue_D + greenValue_D + blueValue_D;
        float rD = redValue_D / sumD;
        float gD = greenValue_D / sumD;
        float bD = blueValue_D / sumD;

        float sumG = redValue_G + greenValue_G + blueValue_G;
        float rG = redValue_G / sumG;
        float gG = greenValue_G / sumG;
        float bG = blueValue_G / sumG;

        // ===== DÉTECTION BLEU =====
        isBlue_D = (bD >= 0.36 && gD <= 0.38);
        isBlue_G = (bG >= 0.36   && gG <= 0.38);

        // ===== DÉTECTION JAUNE =====
        isYellow_D = (gD >= 0.33 && bD <= 0.32);
        isYellow_G = (gG >= 0.33 && bG <= 0.32);
        if (isBlue_D || isBlue_G){
          blue = true;
        }
    
        
        
        
      }

      // Passe à l’état suivant
      stateColor = (stateColor + 1) % 3;
      setFilter(stateColor);
    }
    }
