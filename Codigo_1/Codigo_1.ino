#include<Servo.h>
Servo miServo;

const int zumbador = A0;
const int PinPulsador = 2;
const int LedAmarillo = 3;
const int LedVerde = 4;
const int LedRojo = 5;

int ValorGolpe;
int ValorPulsador;

const int GolpesSuaves = 200;
const int GolpesFuertes = 1023;

boolean bloqueado = false;
int NumeroGolpes = 0;



void setup(){
  miServo.attach(9);
  pinMode(LedAmarillo,OUTPUT);
  pinMode(LedRojo,OUTPUT);
  pinMode(LedVerde,OUTPUT);
  pinMode(PinPulsador,INPUT);
  Serial.begin(9600);

  digitalWrite(LedVerde,HIGH);
  miServo.write(0);
  Serial.println("La caja está bloqueada!");
}//fin del setup



void loop(){
  if(bloqueado == false){
    ValorPulsador = digitalRead(PinPulsador);
      if(ValorPulsador == HIGH){
        bloqueado = true;
        digitalWrite(LedVerde,LOW);
        digitalWrite(LedRojo,HIGH);
        miServo.write(90);
        Serial.println("La caja esta bloqueada!");
        delay(1000);
      }//fin if valorPulsador

    // PRUEBA
    Serial.print("Golpe = ");
    Serial.println(ValorGolpe);
    Serial.println(ValorPulsador);
    delay(500);
    // FIN DE PRUEBA

  }//fin if desbloqueado

  if(bloqueado == true){
    ValorGolpe = analogRead(zumbador);
      if(NumeroGolpes < 3 && ValorGolpe > 0){
          if(VerificarGolpes(ValorGolpe)==true){
            NumeroGolpes++;
          }//fin if VerificarGolpes
        Serial.print(3-NumeroGolpes);
        Serial.println("golpes para abrir");
      }//fin if NumeroGolpes < 3
      if(NumeroGolpes >= 3){
        bloqueado = false;
        miServo.write(0);
        delay(20);
        digitalWrite(LedVerde,HIGH);
        digitalWrite(LedRojo,LOW);
        Serial.println("La caja está bloqueada!");
        NumeroGolpes = 0;
      }//fin if NumeroGolpes >= 3
    delay(500);
  }//if bloqueado

}//fin del loop



boolean VerificarGolpes(int valor){
  if(valor>GolpesSuaves && valor < GolpesFuertes){
    digitalWrite(LedAmarillo,HIGH);
    delay(50);
    digitalWrite(LedAmarillo,LOW);
    Serial.print("Golpe valido de valor = ");
    Serial.println(valor);
    return true;
  }//fin if golpes suaves y fuertes
  else{
    Serial.print("El valor del golpe no es valido = ");
    Serial.println(valor);
    return false;
  }//fin del else
}//fin boolean VerificarGolpes