#ifndef MYLIB_H  // Si MYLIB_H no está definido
#define MYLIB_H  // Define MYLIB_H
#include <stdint.h>
#include <Arduino.h>

extern uint8_t interfazCerrada PROGMEM[];
extern uint8_t logoFausti PROGMEM[];
extern uint8_t candado PROGMEM[];
void encender_rojo();
void encender_verde();
void dibujarInterfazCerrada();
#endif