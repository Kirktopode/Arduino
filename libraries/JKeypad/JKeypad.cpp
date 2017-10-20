#include "JKeypad.h"

/*
*
* Jameson Richard's keypad library. Ain't no copyrights here.
*
*
*/

JKeypad::JKeypad(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3,
                 uint8_t y0, uint8_t y1, uint8_t y2, uint8_t y3){
  xpins[0] = x0;
  xpins[1] = x1;
  xpins[2] = x2;
  xpins[3] = x3;
  ypins[0] = y0;
  ypins[1] = y1;
  ypins[2] = y2;
  ypins[3] = y3;
  for(int i = 0; i < 4; i++){
    pinMode(xpins[i],INPUT);
    pinMode(ypins[i],OUTPUT);
  }
}

/* checkXpins() cycles through the input xpins
   to check for the first Xpin reading high during
   the cycle in checkPins(), returning the index
   of the Xpin which reads high.
   If none read high, 0 is returned.
   If multiple read high, the lower index is returned
*/

int JKeypad::checkXpins(){
  for (int i = 0; i < 4; i++) {
    if (digitalRead(xpins[i]) ) {
      while (digitalRead(xpins[i])){ /* Do not allow additional processing until button is released */
        delay(20);
      }
      return i;
    }
  }
  return -1;
}
void JKeypad::checkPins(int* pins){
  int Xpin;
  for (int i = 0; i < 4; i++) {
    digitalWrite(ypins[i], LOW);
  }
  for (int i = 0; i < 4; i++) {
    digitalWrite(ypins[i], HIGH);
    delay(5);
    if ( (Xpin = checkXpins()) > -1) {
      pins[0] = i;
      pins[1] = Xpin;
      return;
    }
    digitalWrite(ypins[i], LOW);
    delay(5);
  }
  pins[0] = -1;
  pins[1] = -1;
}

char JKeypad::getPress(){
  int pins[2] = {-1,-1};
  checkPins(pins);
  if(pins[0] > -1 && pins[1] > -1){
    return keys[pins[0]][pins[1]];
  }
  else{
    return '\0';
  }
}
