#include <LiquidCrystal.h>
#include <Stepper.h>
double nombredepas = 2048;
Stepper monmoteur(nombredepas, 8, 10, 9, 7);// dans cet ordre pour que la librairie marche
LiquidCrystal lcd (12, 11, 5,4,3,2);


const float pi = 3.1416;
int diametre = 25;
int choixlongueur = 6;
int ButtonValide = 13;
int mm = 20;
int quantite = 10;
double calcul;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16,2); // 16 caractere et 2 ligne
  pinMode(choixlongueur, INPUT);
  pinMode(ButtonValide, INPUT);
  pinMode(A5, INPUT);
  monmoteur.setSpeed(15); // entre 1 et 15 inclus
}

void loop() {
    if (digitalRead(choixlongueur) == 1 && (mm <100)) {
    mm = mm + 10;
    }
    if (mm ==100){
      mm = 20;
    }
    if (analogRead(A5) > 1000 && (quantite <100)) {
    quantite = quantite + 10;
    }
    if (quantite ==100){
      quantite = 10;
    }
  
    if (digitalRead(ButtonValide) == 1){
      calcul = (nombredepas * mm)/(pi * diametre);
    for (int i = 0 ; i < quantite ; i++){
      monmoteur.step(calcul);
      delay(1000);
  }
  }

  lcd.setCursor(1,0);
  lcd.print("Lange = ");
  lcd.print(mm);
  lcd.setCursor(1,1);
  lcd.print("Menge = ");
  lcd.print(quantite);
  delay(200);

}
