#include "mylib.h"
#include <Arduino.h>

void encender_rojo()
{
  digitalWrite(14, LOW); // Enciende la luz roja
  digitalWrite(27, HIGH);
  digitalWrite(33, HIGH); // Enciende el buzzer
}
void encender_verde()
{
  digitalWrite(14, HIGH);
  digitalWrite(27, LOW); // Enciende la luz verde
  digitalWrite(33, HIGH);    // Enciende el buzzer 
}

