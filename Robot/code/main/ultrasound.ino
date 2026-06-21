// ====== ULTRASON======
const int trigPin = 48;
const int echoPin = 50;
long duration;
extern float distance_mm;

void init_setup_ultrason(){
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop_ultrason(){
  if (millis() - top_depart >= temps_max){
        run = false;
        
    }

  // Envoi de l'impulsion ultrason
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Lecture du signal retour
  duration = pulseIn(echoPin, HIGH, 20000); // timeout 20 ms


  // Conversion en mm
  // vitesse du son ≈ 0.343 mm/µs
  distance_mm = (duration * 0.343) / 2;

}