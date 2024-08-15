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

OLED display (21,22);
ezButton button(BUTTON_PIN); // Asignar BUTTON_PIN
Servo myservo;               // Crear objeto myservo
// VARIABLES//
uint8_t angle = 180; // Angulo inicial del servo
char Str[5] = {' ', ' ', ' ', ' ','\0'}; //pass que varia
int character = 0;     // Variable de orden del dígito
int activated = 0;     // Estado de la cerradura -> 2=abierto, 0=cerrado
char pass[5] = {'1','2','3','4','\0'}; // pass fija
unsigned long lightStartTime = 0; // tiempo donde se encendio el led

//fuentes---------------------------------
extern uint8_t SmallFont[];
// -------Configuración del Keypad---------
const uint8_t ROWS = 4; // define numero de filas
const uint8_t COLS = 4; // define numero de filas

// define la distribucion de teclas
const char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

uint8_t colPins[COLS] = {2, 15, 26, 12}; // pines correspondientes a las filas
uint8_t rowPins[ROWS] = {19, 18, 5, 4};  // pines correspondientes a las columnas

// crea objeto con los prametros creados previamente
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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
  display.drawBitmap(0,0,logoFausti,128,64);
  //display.print("Hola Electronico", CENTER, 0);
  delay(200);
  display.update();
  delay(2000);
  display.clrScr();
  myservo.write(angle); // Asigna el ángulo inicial a myservo
}

void loop()
{
  if (activated == 0)
  {
    display.drawBitmap(0,0,interfazCerrada,128,8);
    display.drawLine(0,10,128,10);
    display.update();
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
      display.clrScr();
      display.print("bienvenido",CENTER,0);
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
        Str[i] = ' ';
      }

      encender_rojo();
      lightStartTime = millis();
      display.print("Cerrando...",0,56);
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
      angle = 180;
      //display.setCursor(0, 50);
      if (activated == 0)
      {
      display.print("limpiando...",0,56);
      }
      else if (activated == 2)
      {
      display.print("Cerrando...",0,56);
      }
      encender_rojo();
      lightStartTime = millis();
      display.update();
      delay(500);
      display.clrScr();
      myservo.write(angle);
      activated = 0;
      character = 0;
      for (int i = 0; i < 4; i++)
      {
        Str[i] = ' ';
      }
    }
    // Cambiar pass al presionar "C"
    else if (customKey == 'C' && character == 4)
    {
      // Contraseña correcta
      if (Str[0] == pass[0] && Str[1] == pass[1] && Str[2] == pass[2] && Str[3] == pass[3])
      {
        char nuevopass[5] = {' ',' ',' ',' ','\0'}; // Añadido tamaño 5 para incluir el carácter nulo '\0'
        int i = 0;
        display.clrScr();
        while (i < 5)
        {
          char customKey = customKeypad.getKey();
          display.print("Ingresa la nueva contrasena",0,0);
          display.update();
          if (customKey)
          {
            if (customKey == 'B')//B para volver
            {
              character = 0;
              display.clrScr();
              for (int i = 0; i < 4; i++)
              {
                Str[i] = ' ';
              }
              encender_verde();
              lightStartTime = millis();
              break;
            }
            // asignar los valores de nuevopass a pass con tecla A
            else if (customKey == 'A' && i == 4)
            {
              for (int j = 0; j < 4; j++)//pasar la pass nueva a pass fija
              {
                pass[j] = nuevopass[j];
              }
              display.clrScr();
              display.print("actualizada",CENTER,0);
              display.update();
              character = 0;
              for (int k = 0; k < 4; k++)//borrar array de pass de teclado
              {
                Str[k] = ' ';
              }
              encender_verde();
              lightStartTime = millis();
              delay(1000);
              display.clrScr();
              break;
            }
            // asignando valores al array del nuevo pass
            else if (i < 4)
            {
              nuevopass[i] = customKey;
              i++;
              display.print(nuevopass,0,8);
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
          Str[i] = ' ';
        }
        activated = 0;
        display.clrScr();
        display.print("vuelve intentar",CENTER,0);
        display.update();
        encender_rojo();
        lightStartTime = millis();
        delay(1000);
        display.clrScr();
      }
    }
    // Abrir puerta con tecla "A"
    else if (customKey == 'A' && character == 4 && activated==0)
    {
      // Iniciar la verificación al presionar A con 4 dígitos ingresados
      if (Str[0] == pass[0] && Str[1] == pass[1] && Str[2] == pass[2] &&
       Str[3] == pass[3])
      {
        // Contraseña correcta
        angle = 0;
        myservo.write(angle);
        activated = 2;
        display.clrScr();
        display.print("bienvenido",CENTER,0);
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
          Str[i] = ' ';
        }
        activated = 0;
        display.print("contrasena incorrecta",CENTER,40);
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
      Str[character] = customKey;
      display.print(Str,CENTER,24);
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