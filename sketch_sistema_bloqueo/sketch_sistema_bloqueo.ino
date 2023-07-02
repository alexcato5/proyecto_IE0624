// Bibliotecas necesarias

#include<Servo.h>

/* Para la implementación se usará una máquina con 3 estados:
    
    - ESTADO_DESBLOQUEADO:

    - ESTADO_BLOQUEADO:

    - ESTADO_PATRON:

*/

// Definición de los estados posibles
enum Estado {
  ESTADO_DESBLOQUEADO,
  ESTADO_BLOQUEADO,
  ESTADO_PATRON,
};

/*****************
*  DEFINICIONES  * 
*****************/


/* Pines para Arduino UNO
*/
const int piezoelectrico  = A0;
const int pulsadorBloqueo = 2;
const int LedAmarillo     = 3;
const int LedVerde        = 4;
const int LedRojo         = 5;
const int pulsadorPatron  = 13;

/* Pines para Arduino Nano BLE 33
const int piezoelectrico  = A0;
const int pulsadorBloqueo = 2;
const int LedAmarillo     = 3;
const int LedVerde        = 4;
const int LedRojo         = 5;
const int pulsadorPatron  = 13;
*/


// Variables globales
Estado estadoActual;
Servo motor;
const int UMBRAL_GOLPE_SUAVE = 100;
const int UMBRAL_GOLPE_FUERTE = 512;
int   ValorGolpe;
int   ValorPulsadorBloqueo;
int   valorPulsadorPatron;
char  golpe;
int   datosGolpe[10];
char  patronDesbloqueo[5] = {'D', 'F', 'D', 'F', 'D'};
char  patronIntento[5];
int   numeroGolpes;
bool  programarPatron;
int  lecturaAnalogica;
bool golpeRecibido;

/**************
*  FUNCIONES  * 
**************/

// Configuración inicial
void setup() {
  
  // Se iniciará la máquina en el estado bloqueado
  estadoActual = ESTADO_BLOQUEADO;

  pinMode(pulsadorBloqueo,  INPUT);
  pinMode(pulsadorPatron,   INPUT);
  pinMode(LedAmarillo,      OUTPUT);
  pinMode(LedRojo,          OUTPUT);
  pinMode(LedVerde,         OUTPUT);
  motor.attach(9);
  Serial.begin(115200);
  valorPulsadorPatron = 0;
  ValorPulsadorBloqueo = 0;

  digitalWrite(LedRojo,HIGH);
  motor.write(90);
  Serial.println("La caja está bloqueada");
  
  numeroGolpes = 0;
} // fin de setup


/* Esta función se encargará de tomar datos al recibir un golpe válido
Se toman múltiples datos debido a que un golpe genera una vibración
que dura varios milisegundos. Se retorna la decisión tomada como un char.
*/
char interpretarGolpe(int primerValorLeido)
{
  int sumaIntensidades = primerValorLeido;
  for(int i=0; i<10; i++)
  {
    sumaIntensidades += analogRead(piezoelectrico);
  }

  return (sumaIntensidades > UMBRAL_GOLPE_FUERTE)? 'F':'D';    
}


// Función para determinar la validez de los golpes
bool golpeValido(int intensidadGolpe){
  return (intensidadGolpe > UMBRAL_GOLPE_SUAVE)? true:false;
} // Fin golpeValido(int)


void loop() {

  // Máquina de estados
  switch (estadoActual) {
    
    case ESTADO_BLOQUEADO:

      // Configuración inicial de bloqueo
      digitalWrite(LedRojo,    HIGH);
      digitalWrite(LedVerde,    LOW);
      digitalWrite(LedAmarillo, LOW);

      lecturaAnalogica = analogRead(piezoelectrico);
      golpeRecibido = golpeValido(lecturaAnalogica);

      if (golpeRecibido && numeroGolpes < 4)
      {
        patronIntento[numeroGolpes] = interpretarGolpe(lecturaAnalogica);
        numeroGolpes++;
        estadoActual = ESTADO_BLOQUEADO;
      } else if(golpeRecibido && numeroGolpes >= 4)
      {
        numeroGolpes = 0;
        patronIntento[4] = interpretarGolpe(lecturaAnalogica);

        Serial.print("El patrón con el que se intenta desbloquear es: ");
        Serial.println(patronIntento[0]);
        Serial.println(patronIntento[1]);
        Serial.println(patronIntento[2]);
        Serial.println(patronIntento[3]);
        Serial.println(patronIntento[4]);
        Serial.print("^ ^ ^");

        Serial.print("El patrón de desbloqueo correcto es: ");
        Serial.println(patronDesbloqueo[0]);
        Serial.println(patronDesbloqueo[1]);
        Serial.println(patronDesbloqueo[2]);
        Serial.println(patronDesbloqueo[3]);
        Serial.println(patronDesbloqueo[4]);
        Serial.print("^ ^ ^");

        // Lógica de seguridad
        bool patronAcertado = true;
        for(int k=0; k < 5; k++)
        {
          patronAcertado = patronAcertado && (patronIntento[k] != patronDesbloqueo[k]);
        }

        if(patronAcertado){
          estadoActual = ESTADO_DESBLOQUEADO;
          digitalWrite(LedRojo,   LOW);
          for(int i=0; i<3; i++){
            digitalWrite(LedAmarillo, HIGH);
            digitalWrite(LedVerde,     LOW);
            delay(666);
            digitalWrite(LedVerde,    HIGH);
            digitalWrite(LedAmarillo,  LOW);
          }
        } else {
          estadoActual = ESTADO_BLOQUEADO;
          digitalWrite(LedVerde, LOW);
          for(int i=0; i<3; i++){
            digitalWrite(LedAmarillo, HIGH);
            digitalWrite(LedRojo,      LOW);
            delay(666);
            digitalWrite(LedAmarillo,  LOW);
            digitalWrite(LedRojo,     HIGH);
          }
        }
      }
      break;
      
    case ESTADO_DESBLOQUEADO:

      // Configuración inicial de desbloqueo
      digitalWrite(LedVerde,   HIGH);
      digitalWrite(LedAmarillo, LOW);
      digitalWrite(LedRojo,     LOW);
      motor.write(0);
      
      // Lectura de los pulsadores para entrar a los distintos modos
      valorPulsadorPatron   = analogRead(pulsadorPatron);
      ValorPulsadorBloqueo  = analogRead(pulsadorBloqueo);

      if(valorPulsadorPatron)       { estadoActual = ESTADO_PATRON; }
      else if(ValorPulsadorBloqueo) { estadoActual = ESTADO_BLOQUEADO; }
      else                          { estadoActual = ESTADO_DESBLOQUEADO; }
      break;
      
    case ESTADO_PATRON:

      // Configuración inicial de programación de patrón
      digitalWrite(LedVerde,    HIGH);
      digitalWrite(LedAmarillo, HIGH);
      digitalWrite(LedRojo,     LOW);
      motor.write(0);

      lecturaAnalogica = analogRead(piezoelectrico);
      golpeRecibido = golpeValido(lecturaAnalogica);

      if (golpeRecibido && numeroGolpes < 4)
      {
        patronDesbloqueo[numeroGolpes] = interpretarGolpe(lecturaAnalogica);
        numeroGolpes++;
        estadoActual = ESTADO_PATRON; 
      } else if(golpeRecibido && numeroGolpes >= 4)
      {
        patronDesbloqueo[4] = interpretarGolpe(lecturaAnalogica);

        Serial.print( "El patrón de desbloqueo nuevo es: ");
        Serial.println(patronDesbloqueo[0]);
        Serial.println(patronDesbloqueo[1]);
        Serial.println(patronDesbloqueo[2]);
        Serial.println(patronDesbloqueo[3]);
        Serial.println(patronDesbloqueo[4]);
        Serial.print( "^ ^ ^");

        // Animación con LEDs para confirmar al usuario
        for(int i=0; i<3; i++){
          digitalWrite(LedAmarillo, HIGH);
          digitalWrite(LedVerde,    HIGH);
          digitalWrite(LedRojo,     HIGH);
          delay(2000);
          digitalWrite(LedRojo,     LOW);
          digitalWrite(LedVerde,    LOW);
          digitalWrite(LedAmarillo, LOW);
        }
        numeroGolpes = 0;
        estadoActual = ESTADO_DESBLOQUEADO;          
      } else {
        estadoActual = ESTADO_PATRON;
      }
      break;
    
    default:
      // En caso de comportamiento inadecuado, se vuelve al estado de bloqueo por seguridad
      estadoActual = ESTADO_BLOQUEADO;
      break;
  }
}