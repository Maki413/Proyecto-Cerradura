#include <Arduino.h>

#include <Keypad.h>
#include <ESP32Servo.h>
#include <ezButton.h>
#include <Adafruit_SSD1306.h>
#include "mylib.h"

#define BUTTON_PIN 25      // PIN DEL BOTON
#define SERVO_PIN 23       // PIN DEL SERVO
#define LED_RED_PIN 14      // PIN del LED rojo
#define LED_GREEN_PIN 27    // PIN del LED verde
#define BUZZER_PIN 33      // PIN del buzzer
#define LIGHT_DURATION 1000 // Duración de la luz y sonido en milisegundos (1 segundos)
#define SCREEN_WIDTH 128 // OLED display width, en pixels

#define SCREEN_HEIGHT 64 // OLED display height, en pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ezButton button(BUTTON_PIN); // Asignar BUTTON_PIN
Servo myservo;               // Crear objeto myservo
// VARIABLES//
int angle = 180; // Angulo inicial del servo
char Str[4] = {' ', ' ', ' ', ' '};
int character = 0; // Variable de orden del dígito
int activated = 0; // Estado de la cerradura -> 2=abierto, 0=cerrado
char pass[5] = "1234"; // Añadido tamaño 5 para incluir el carácter nulo '\0'
const char* ssid = "LUIS";
unsigned long lightStartTime = 0;//tiempo donde se encendio el led

// -------Configuración del Keypad---------
const uint8_t ROWS = 4; // define numero de filas
const uint8_t COLS = 4; // define numero de filas

// define la distribucion de teclas
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

uint8_t colPins[COLS] = {2, 15, 26, 12};    // pines correspondientes a las filas
uint8_t rowPins[ROWS] = {19, 18, 5, 4}; // pines correspondientes a las columnas

// crea objeto con los prametros creados previamente
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
  button.setDebounceTime(0);    // Delay antirebote, asignar un delay mayor genera errores
  myservo.attach(SERVO_PIN);     // Enlaza myservo con el pin del servo
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  //bucle inicio del display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  Serial.println(F("SSD1306 allocation failed"));
  for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);           
  display.setTextColor(SSD1306_WHITE);      
  display.setCursor(0,0); 
  display.println("Hola electronico");
  delay(200);
  display.display();
  delay(2000);
  display.clearDisplay();
  myservo.write(angle); // Asigna el ángulo inicial a myservo
}

void loop() {
  if(activated==0){
  display.setCursor(0,0); 
  display.println("Ingresa la contrasena");
  display.display();
  }
  button.loop(); // Bucle del botón
  if (button.isPressed())
  {
    // Cambiando ángulo del servo
    switch (activated)
    {
    case 0:
      angle = 0;
      activated = 2;
      character=4;
      Serial.print("Puerta abierta \n");
      display.clearDisplay();
      display.println("bienvenido");
      display.display();
      encender_verde();
      lightStartTime = millis();
        break;

    case 2:
      angle = 180;
      activated = 0;
      character = 0;
      for (int i = 0; i < 4; i++)
      {
        Str[i] = ' ';
      }
      Serial.print("Puerta cerrada \n");
      encender_rojo();
      lightStartTime = millis();
      display.clearDisplay();    
        break;

    default:
      break;
    }
  }
  myservo.write(angle);//imprimir angulo constantemente en cda cambio
  // PEDIR CONTRASEÑA-KEYPAD OPEN/CLOSE
  char customKey = customKeypad.getKey(); // Variable de la contraseña
  if (customKey){
  //cerrar y reiniciar con tecla "B"
    if (customKey == 'B')
    {
      angle = 180;
      display.setCursor(0,50); 
      display.println("Cerrando...");
      display.display();
      delay(500);
      display.clearDisplay();
      myservo.write(angle);
      activated = 0;
      character = 0;
      for (int i = 0; i < 4; i++)
      {
        Str[i] = ' ';
      }
      Serial.print("borrar y cerrar \n");
      encender_rojo();
      lightStartTime = millis();
    }
    //Cambiar pass al presionar "C"
    else if (customKey == 'C' && character == 4)
    {
      // Contraseña correcta
      if (Str[0] == pass[0] && Str[1] == pass[1] && Str[2] == pass[2] && Str[3] == pass[3])
      {
        Serial.print("insertar nueva contraseña\n");
        char nuevopass[5] = "    "; // Añadido tamaño 5 para incluir el carácter nulo '\0'
        int i=0;
        display.clearDisplay();
        while (i<5)
        {
          char customKey = customKeypad.getKey();
          display.setCursor(0,0); 
          display.println("Ingresa la nueva");
          display.println("contrasena");
          display.display();
          if (customKey)
          {
            if(customKey=='B')
            {
              character = 0;
              display.clearDisplay();
              for (int i = 0; i < 4; i++)
              {
                Str[i] = ' ';
              }
              encender_verde();
              lightStartTime = millis();
                break;
            }
            //asignar los valores de nuevopass a pass
            else if(customKey=='A' && i==4)
            {          
              for (int j = 0; j < 4; j++)
              {
                pass[j] = nuevopass[j];
              }
              pass[4] = '\0'; // Agregar el carácter nulo al final
              Serial.print("contraseña actualizada\n");
              display.clearDisplay();
              display.println("contrasena actualizada");
              display.display();
              character = 0;
              for (int i = 0; i < 4; i++)
              {
                Str[i] = ' ';
              }
              encender_verde();
              lightStartTime = millis();
              delay(1000);
              display.clearDisplay();
                break;
            }
            //asignando valores del nuevo pass
            else if(i<4)
            {
              nuevopass[i]=customKey;
              i++;
              display.println(nuevopass);
              display.display();
              Serial.print(customKey);
            }
          }
        }
      }
      //fallo al cambiar contraseña
      else
      {
        character = 0;
        for (int i = 0; i < 4; i++)
        {
          Str[i] = ' ';
        }
        activated = 0;
        Serial.print("contraseña incorrecta, vuelve intentar \n");
        display.clearDisplay();
        display.println("contraseña incorrecta, vuelve intentar");
        display.display();                
        encender_rojo();
        lightStartTime = millis();
        delay(1000);
        display.clearDisplay();
      }
    }
    //Abrir puerta con tecla "A"
    else if (customKey == 'A' && character == 4)
    {
      // Iniciar la verificación al presionar A con 4 dígitos ingresados
      if (Str[0] == pass[0] && Str[1] == pass[1] && Str[2] == pass[2] && Str[3] == pass[3])
      {
        // Contraseña correcta
        angle = 0;
        myservo.write(angle);
        activated = 2;
        Serial.print("contraseña correcta, abierto \n");
        display.clearDisplay();
        display.println("contraseña correcta, bienvenido");
        display.display();
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
        Serial.print("contraseña incorrecta \n");
        display.clearDisplay();                
        display.println("contraseña incorrecta");
        display.display();
        encender_rojo();
        lightStartTime = millis();
        delay(1000);
        display.clearDisplay();
      }
    }
    //ingresar los numeros a la lista
    else if (character < 4)
    {
      Str[character] = customKey;
      Serial.print(customKey);
      display.println(Str);
      display.display();
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