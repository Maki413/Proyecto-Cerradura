#include <Arduino.h>
#include <OLED_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <ezButton.h>
#include <Adafruit_GFX.h>
#include "mylib.h"

#define BUTTON_PIN 25       // PIN DEL BOTON
#define SERVO_PIN 23        // PIN DEL SERVO
#define LED_RED_PIN 14      // PIN del LED rojo
#define LED_GREEN_PIN 27    // PIN del LED verde
#define BUZZER_PIN 33       // PIN del buzzer
#define LIGHT_DURATION 1000 // Duración de la luz y sonido en milisegundos (1 segundos)

OLED display (21,22);        // objeto displayOLED
ezButton button(BUTTON_PIN); // objeto Boton
Servo myservo;               // objeto myservo

// -----------------VARIABLES-----------------
uint8_t angle = 180; // Angulo inicial del servo
char passVar[5] = {' ', ' ', ' ', ' ','\0'}; //pass activa
int8_t character = 0;     // Variable de orden del dígito
int8_t activated = 0;     // Estado de la cerradura -> 2=abierto, 0=cerrado
char passSave[5] = {'1','2','3','4','\0'}; // pass guardada
unsigned long lightStartTime = 0; // tiempo donde se encendio el led

//-------------------fuentes---------------
extern uint8_t SmallFont[];
extern uint8_t TinyFont[];

// -------Configuración del Keypad---------
const uint8_t FILAS = 4; // define numero de filas
const uint8_t COLUMNAS = 4; // define numero de columnas

// define la dipassVaribucion de teclas
const char keys[FILAS][COLUMNAS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
  };

uint8_t colPins[COLUMNAS] = {2, 15, 26, 12}; // pines correspondientes a las filas
uint8_t filPins[FILAS] = {19, 18, 5, 4};  // pines correspondientes a las columnas

// crea objeto con los prametros creados previamente
Keypad customKeypad = Keypad(makeKeymap(keys), filPins, colPins, FILAS, COLUMNAS);

void setup()
{
  Serial.begin(115200);
  button.setDebounceTime(0); // Delay antirebote, asignar un delay mayor genera errores
  myservo.attach(SERVO_PIN); // Enlaza myservo con el pin del servo
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.begin();
  display.invert(false);
  display.setBrightness(207);
  display.clrScr();
  display.setFont(SmallFont);
  display.drawBitmap(2,5,logoFausti,128,64);
  delay(200);
  display.update();
  delay(2000);
  display.clrScr();
  myservo.write(angle); // Asigna el ángulo inicial a myservo
}

void loop()
{
  display.setFont(TinyFont);
  if (activated == 0)
  {
    dibujarInterfazCerrada();
  }
  button.loop(); // Bucle inicio del botón
  if (button.isPressed())
  {
    // Cambiando ángulo del servo
    switch (activated)
    {
    case 0: //0 = cerrado
      angle = 0;
      activated = 2;
      character = 4;
      display.setFont(SmallFont);
      display.print("BIENVENIDO",CENTER,23);
      display.drawBitmap(0,0,candado,10,8);
      display.update();
      encender_verde();
      lightStartTime = millis();
      break;

    case 2: //2 = abierto
      angle = 180;
      activated = 0;
      character = 0;
      for (int i = 0; i < 4; i++)
      {
        passVar[i] = ' ';
      }

      encender_rojo();
      lightStartTime = millis();
      display.print("Cerrando...",0,40);
      display.update();
      delay(500);
      display.clrScr();
      display.clrScr();
      break;

    default:
      break;
    }
  }
  myservo.write(angle); // imprimir angulo constantemente en cda cambio
  // PEDIR CONTRASEÑA-KEYPAD OPEN/CLOSE
  char customKey = customKeypad.getKey(); // asignar tecla presionada a la variable customkey
  if (customKey)
  {
    // cerrar o limpiar con tecla "B"
    if (customKey == 'B' && character != 0)
    {
      display.invertText(true);
      display.print("     cerrar     ",65,51);
      display.update();
      delay(100);
      display.invertText(false);
      display.print("     cerrar     ",65,51);
      display.update();
      angle = 180;
      display.setFont(SmallFont);
      if (activated == 0)
      {
      display.print("Limpiando...",0,40);
      display.update();
      }
      else if (activated == 2)
      {
      display.print("Cerrando...",0,40);
      display.update();
      }
      encender_rojo();
      lightStartTime = millis();
      delay(500);
      display.clrScr();
      myservo.write(angle);
      activated = 0;
      character = 0;
      for (int i = 0; i < 4; i++)
      {
        passVar[i] = ' ';
      }
    }
    // Cambiar password al presionar "C"
    else if (customKey == 'C' && character == 4 && activated==0)
    {
      display.invertText(true);
      display.print("    cambiar     ",0,58);
      display.update();
      delay(100);
      display.invertText(false);
      display.print("    cambiar     ",0,58);
      display.update();
      // Contraseña correcta
      if (passVar[0] == passSave[0] && passVar[1] == passSave[1] && passVar[2] == passSave[2] && passVar[3] == passSave[3])
      {
        char nuevopass[5] = {' ',' ',' ',' ','\0'}; // Añadido tamaño 5 para incluir el carácter nulo '\0'
        int i = 0;
        display.clrScr();
        while (i < 5)
        {
          char customKey = customKeypad.getKey();
          display.setFont(TinyFont);
          dibujarInterfazCerrada();
          display.print("Ingresa la nueva contrasena",0,13);
          display.update();
          if (customKey)
          {
            if (customKey == 'B')//B para volver
            {
              display.invertText(true);
              display.print("     cerrar     ",65,51);
              display.update();
              delay(100);
              display.invertText(false);
              display.print("     cerrar     ",65,51);
              display.update();
              character = 0;
              for (int i = 0; i < 4; i++)
              {
                passVar[i] = ' ';
              }
              display.setFont(SmallFont);
              display.print("Cerrando...",0,40);
              display.update();
              encender_verde();
              lightStartTime = millis();
              delay(500);
              display.clrScr();
              break;
            }
            // asignar los valores de nuevopass a passSave con tecla A
            else if (customKey == 'A' && i == 4)
            {
              display.invertText(true);
              display.print("     abrir      ",0,51);
              display.update();
              delay(100);
              display.invertText(false);
              display.print("     abrir      ",0,51);
              display.update();
              for (int j = 0; j < 4; j++)//pasar la pass nueva a passSave
              {
                passSave[j] = nuevopass[j];
              }
              display.print("contrasena actualizada",0,40);
              display.update();
              character = 0;
              for (int k = 0; k < 4; k++)//borrar array de pass de teclado
              {
                passVar[k] = ' ';
              }
              encender_verde();
              lightStartTime = millis();
              delay(500);
              display.clrScr();
              break;
            }
            // asignando valores al array del nuevo pass
            else if (i < 4 && customKey != 'A' && customKey != 'B' && customKey != 'C' && 
            customKey != 'D' && customKey != '*' && customKey != '#')
            {
              nuevopass[i] = customKey;
              i++;
              display.setFont(SmallFont);
              display.print(nuevopass,CENTER,24);
              display.update();
            }
          }
        }
      }
      // fallo al cambiar contraseña
      else
      {
        character = 0;
        for (int i = 0; i < 4; i++)
        {
          passVar[i] = ' ';
        }
        activated = 0;
        display.setFont(SmallFont);
        display.print("vuelve intentar",0,40);
        display.update();
        encender_rojo();
        lightStartTime = millis();
        delay(500);
        display.clrScr();
      }
    }
    // Abrir puerta con tecla "A"
    else if (customKey == 'A' && character == 4 && activated==0)
    {
      display.invertText(true);
      display.print("     abrir      ",0,51);
      display.update();
      delay(100);
      display.invertText(false);
      display.print("     abrir      ",0,51);
      display.update();
      // Iniciar la verificación al presionar A con 4 dígitos ingresados
      if (passVar[0] == passSave[0] && passVar[1] == passSave[1] && passVar[2] == passSave[2] &&
       passVar[3] == passSave[3])
      {
        // Contraseña correcta
        angle = 0;
        myservo.write(angle);
        activated = 2;
        display.setFont(SmallFont);
        display.print("BIENVENIDO",CENTER,23);
        display.drawBitmap(0,0,candado,10,8);
        display.update();        
        encender_verde();
        lightStartTime = millis();
      }
      else
      {
        // Contraseña incorrecta
        character = 0;
        for (int i = 0; i < 4; i++)
        {
          passVar[i] = ' ';
        }
        activated = 0;
        display.setFont(SmallFont);
        display.print("contrasena incorrecta",0,40);
        display.update();
        encender_rojo();
        lightStartTime = millis();
        delay(1000);
        display.clrScr();
      }
    }
    // ingresar los numeros a la lista
    else if (character < 4 && customKey != 'A' && customKey != 'B' && 
    customKey != 'C' && customKey != 'D' && customKey != '*' && customKey != '#')
    {
      passVar[character] = customKey;
      display.setFont(SmallFont);
      display.print(passVar,CENTER,24);
      display.update();
      character++;
    }
  }
  // Apagar la luz y el buzzer después de 1 segundos
  if (millis() - lightStartTime >= LIGHT_DURATION)
  {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
  }
}