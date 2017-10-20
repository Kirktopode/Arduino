#ifndef JKeypad_h_
#define JKeypad_h_

#if defined (ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
  #include <pins_arduino.h>
#endif

#if defined (__AVR__)
	#include <avr/io.h>
	#include <avr/interrupt.h>
#endif

class JKeypad {
  private:
    const char keys[4][4] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    };
    int xpins[4];
    int ypins[4];
    int checkXpins();
    void checkPins(int* pins);
    void processNumInput(const int* pins);
  public:
    JKeypad(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3,
            uint8_t y0, uint8_t y1, uint8_t y2, uint8_t y3);
    char getPress();
};

#endif
