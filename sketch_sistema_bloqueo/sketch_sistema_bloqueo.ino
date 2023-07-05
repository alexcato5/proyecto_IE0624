// Bibliotecas necesarias

#include<Servo.h>

/* Para la implementación se usará una máquina con 3 estados:
    
    - ESTADO_BLOQUEADO: En este estado, el sistema cierra la 
                        cerradura y lee patrones de golpes,
                        esperando a la combinación correcta
                        para pasar al estado de desbloqueo.

    - ESTADO_DESBLOQUEADO: En este estado, el sistema abre 
                          la cerradura y espera a que se 
                          presione uno de los pulsadores
                          (bloqueo o definición del patrón).

    - ESTADO_PATRON: Se entra en este estado para definir 
                    un nuevo patrón de golpes, únicamente
                    al presionar el pulsador respectivo
                    mientras se encuentra en el estado de
                    desbloqueo.
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
const int piezoelectrico  = A0;
const int pulsadorBloqueo = 2;
const int LedBlanco       = 3;
const int LedAzul         = 4;
const int LedRojo         = 5;
const int pulsadorPatron  = 13;
*/

/* Pines para Arduino Nano BLE 33
*/
const int piezoelectrico  = A3;
const int pulsadorBloqueo = D2;
const int pulsadorPatron  = D3;
const int LedBlanco       = D4;
const int LedAzul         = D5;
const int LedRojo         = D6;


// Variables globales
Estado estadoActual;
Servo motor;
const int UMBRAL_GOLPE_SUAVE = 100;
const int UMBRAL_GOLPE_FUERTE = 10000;
int   ValorGolpe;
int   ValorPulsadorBloqueo;
int   valorPulsadorPatron;
char  golpe;
int   datosGolpe[10];
char  patronDesbloqueo[5] = {'D', 'D', 'D', 'D', 'D'};
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
  pinMode(LedBlanco,       OUTPUT);
  pinMode(LedRojo,         OUTPUT);
  pinMode(LedAzul,         OUTPUT);
  motor.attach(D7);
  Serial.begin(115200);
  valorPulsadorPatron = 0;
  ValorPulsadorBloqueo = 0;

  digitalWrite(LedRojo, HIGH);
  motor.write(0);
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
      motor.write(0);
      digitalWrite(LedRojo,    HIGH);
      digitalWrite(LedAzul,    LOW);
      digitalWrite(LedBlanco, LOW);

      lecturaAnalogica = analogRead(piezoelectrico);
      //Serial.println("Valor leido:");
      //Serial.println(lecturaAnalogica);
      
      golpeRecibido = golpeValido(lecturaAnalogica);
      
      
      if (golpeRecibido && numeroGolpes < 4)
      {
        Serial.println("Se recibió golpe");
        
        patronIntento[numeroGolpes] = interpretarGolpe(lecturaAnalogica);
        Serial.println(patronIntento[numeroGolpes]);
        numeroGolpes++;
        estadoActual = ESTADO_BLOQUEADO;
      } else if(golpeRecibido && numeroGolpes >= 4)
      {
        numeroGolpes = 0;
        patronIntento[4] = interpretarGolpe(lecturaAnalogica);

        Serial.println("El patrón con el que se intenta desbloquear es: ");
        Serial.println(patronIntento[0]);
        Serial.println(patronIntento[1]);
        Serial.println(patronIntento[2]);
        Serial.println(patronIntento[3]);
        Serial.println(patronIntento[4]);
        Serial.println("^ ^ ^");

        Serial.println("El patrón de desbloqueo correcto es: ");
        Serial.println(patronDesbloqueo[0]);
        Serial.println(patronDesbloqueo[1]);
        Serial.println(patronDesbloqueo[2]);
        Serial.println(patronDesbloqueo[3]);
        Serial.println(patronDesbloqueo[4]);
        Serial.println("^ ^ ^");

        // Lógica de seguridad
        bool patronAcertado = true;
        for(int k=0; k < 5; k++)
        {
          patronAcertado = patronAcertado && (patronIntento[k] == patronDesbloqueo[k]);
          Serial.println(patronAcertado);  
        }

        if(patronAcertado){
          estadoActual = ESTADO_DESBLOQUEADO;
          digitalWrite(LedRojo,   LOW);
          for(int i=0; i<3; i++){
            digitalWrite(LedBlanco, HIGH);
            digitalWrite(LedAzul,     LOW);
            delay(666);
            digitalWrite(LedAzul,    HIGH);
            digitalWrite(LedBlanco,  LOW);
            delay(666);
          }
        } else {
          estadoActual = ESTADO_BLOQUEADO;
          digitalWrite(LedAzul, LOW);
          for(int i=0; i<3; i++){
            digitalWrite(LedBlanco, HIGH);
            digitalWrite(LedRojo,      LOW);
            delay(666);
            digitalWrite(LedBlanco,  LOW);
            digitalWrite(LedRojo,     HIGH);
          }
        }
      }
      break;
      
    case ESTADO_DESBLOQUEADO:

      // Configuración inicial de desbloqueo
      digitalWrite(LedAzul,   HIGH);
      digitalWrite(LedBlanco, LOW);
      digitalWrite(LedRojo,     LOW);
      motor.write(90);
      
      // Lectura de los pulsadores para entrar a los distintos modos
      valorPulsadorPatron   = digitalRead(pulsadorPatron);
      ValorPulsadorBloqueo  = digitalRead(pulsadorBloqueo);

      if(ValorPulsadorBloqueo) { estadoActual = ESTADO_BLOQUEADO; Serial.println("Se pasará al modo de bloqueo"); }
      else if(valorPulsadorPatron)       { estadoActual = ESTADO_PATRON; Serial.println("Modo de configuración de patrón activado");}
      else                          { estadoActual = ESTADO_DESBLOQUEADO; }
      break;
      
    case ESTADO_PATRON:

      // Configuración inicial de programación de patrón
      digitalWrite(LedAzul,    HIGH);
      digitalWrite(LedBlanco, HIGH);
      digitalWrite(LedRojo,     LOW);
      motor.write(90);

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
          digitalWrite(LedBlanco, HIGH);
          digitalWrite(LedAzul,    HIGH);
          digitalWrite(LedRojo,     HIGH);
          delay(2000);
          digitalWrite(LedRojo,     LOW);
          digitalWrite(LedAzul,    LOW);
          digitalWrite(LedBlanco, LOW);
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