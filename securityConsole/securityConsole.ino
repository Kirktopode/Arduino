#include <JKeypad.h>
#include <SoftwareSerial.h>
#include <NewPing.h>
#include <NewTone.h>
#include <string.h>

#define TIMER_ENABLED true

enum State {
  SETPIN, CALIBRATE, DISARMED, ARMED, ALARM
};

int myAbs(int val){
  if(val < 0){
    return -val;
  }
  return val;
}
float myAbs(float val){
  if(val < 0){
    return -val;
  }
  return val;
}
unsigned long myAbs(unsigned long val){
  if(val < 0){
    return -val;
  }
  return val;
}
class SecuritySystem{
  private:
    int calibratedDistance;  //To be used to store the distance that the calibration routine finds.
    int sonarBuffer[50]; //Buffer for the calibration process
    unsigned long firstTrip; //unsigned long to store when the sonar first saw the alarm getting tripped
    char PIN[5];             //User PIN
    char tryPIN[5];          //Buffer to store the user's attempt at guessing the PIN
    JKeypad &keypad;         //JKeypad object reference for the keypad, using Jameson's JKeypad library
    SoftwareSerial &screen;  //SoftwareSerial reference for the LCD
    NewPing &sonar;          //NewPing reference for the ultrasound rangefinder
    State state;             //Stores the current state, an enumerated value defined above
    bool tripped;            //Stores whether the alarm is tripped. Redundant with alarmTripped()?
    bool testing;            //Stores whether the test mode is active
    const int redPin0;
    const int redPin1;
    const int bluePin0;
    const int bluePin1;
    const int buzPin;

    /*
     * clearScreen is a simple method for clearing the LCD screen.
     * It's really just here to reduce redundant code.
     */
    
    void clearScreen(){
      screen.write(254);
      screen.write(128);
      screen.write("                ");
      screen.write("                ");
    }

    /*
     * screenLineOne() receives a C-style string, places a null character at index 17 if it is
     * longer than 17 characters (including the null character), then prints the string to the 
     * first line.
     * 
     * The string is truncated in order to make sure it fits on one line.
     */
    
    void screenLineOne(char* str){
      if(strlen(str) > 17) str[17] = '\0';
      screen.write(254);
      screen.write(128);
      screen.write(str);
    }
    /*
     * screenLineTwo() receives a C-style string, places a null character at index 17 if it is
     * longer than 17 characters (including the null character), then prints the string to the 
     * second line.
     * 
     * The string is truncated in order to make sure it fits on one line.
     */
    void screenLineTwo(char* str){
      if(strlen(str) > 17) str[17] = '\0';
      screen.write(254);
      screen.write(192);
      screen.write(str);
    }
    /*
     * calibrateRoutine() is the routine which the system uses to calibrate the distance from
     * the ultrasound sensor to the plate. It should store a calibrated distance in the
     * calibratedDistance private member and may use the calibrateBuffer to store intermediate
     * data if necessary (it will probably be necessary).
     */
    void calibrateRoutine(){
      
      clearScreen();
      screenLineOne("Calibrating.....");
      int tries = 0;
      calibratedDistance = -1;

      while(calibratedDistance < 0 && tries < 5){
        for(int i = 0; i < 50; i++){
          sonarBuffer[i] = sonar.ping_cm();
          delay(20);
        }
        float avg = 0;
        for(int i = 0; i < 50; i++){
          avg += sonarBuffer[i];
        }
        avg /= 50;

        int i;
        for(i = 0; i < 50; i++){
          if(myAbs(sonarBuffer[i] - avg) > 3){
            break;
          }
        }
        if(i >= 50){
          calibratedDistance = (int)avg;
        }else{
          tries++;
        }
      }

      if(calibratedDistance < 0){
        clearScreen();
        screenLineOne("Calibration err");
        delay(2000);
      }
    
    }
    /*
     * alarmTripped() is a function which checks if the alarm has been tripped. It looks at the
     * data received from the sonar sensor long enough to tell is the alarm is being tripped.
     * Because this method must be run in concert with the input routine, this will need to make
     * use of the milliseconds of its call. Should be doable. Maybe fill a buffer and check the
     * buffer for deviations once it's full, if there are deviations then sound the alarm, if
     * not then refill the buffer.
     */
    bool alarmTripped(){
      /*
       * Whatever routine you're going to use to test if the alarm has been tripped.
       * This will probably need to run a few times on each loop of armed() so that
       * we can balance it with the keypad's input. This sensor is a lot faster than
       * a human finger, so this should be very doable.
       */
      int echo = sonar.ping_median();
      int cm = sonar.convert_cm(echo);
      if(myAbs(cm - calibratedDistance) > 3){
        /*if(firstTrip == 0){
          firstTrip = millis();
          if(firstTrip == 0){
            firstTrip++;
          }
        }else{
          if(millis() - firstTrip > 5){
            firstTrip = 0;
            return true;
          }
        }*/
        return true; //FIXME remove this if this iteration no work
      }/*else{
        firstTrip = 0;
      }*/
      return false;
    }
    /*
     * testAlarm() begins the alarm routine, then goes to the validatePIN() routine so
     * that the user can then shut off the alarm. Testing is set to true so that the
     * system knows not to disarm when the test is complete.
     */
    void testAlarm(){
      testing = true;
      soundAlarm();
      validatePIN();
    }
    /*
     * soundAlarm() is the method called to begin the alarm sequence. Depending upon what we
     * have in mind, it may be a good idea to allow repeated calls to it to cycle through
     * a more interesting alarm sequence than simply "Set tone to x, turn on the lights."
     */
    void soundAlarm(){
      /*
       * Empty so far. It'll just say that the alarm is tripped.
       */
      if(millis() % 10 == 0){
        NewTone(buzPin, millis() % 3000);
        analogWrite(redPin0, (millis() + 128) % 256);
        analogWrite(redPin1, millis() % 256);
        analogWrite(bluePin0, (millis() + 128) % 256);
        analogWrite(bluePin1, millis() % 256);
      }
      tripped = true;
    }
    /*
     * endAlarm() is the method which ends the alarm sequence.
     */
    void endAlarm(){
      /*
       * Whatever commands you need to end the alarm sequence should go here.
       * noNewTone()? I'm not sure what else you might want to use; that one's
       * pretty open-ended.
       */
      noNewTone(buzPin);
      analogWrite(redPin0, 0);
      analogWrite(redPin1, 0);
      analogWrite(bluePin0, 0);
      analogWrite(bluePin1, 0);
      tripped = false;
    }
    /*
     * validatePIN() begins the sequence for the user to enter the PIN. This method is called
     * directly from when the alarm sounds, when the alarm is being tested, and when the 
     * system is being disarmed.
     */
    void validatePIN(){
      char ch;
      bool correct = false;
      int tries = 0;
      clearScreen();
      screenLineOne("Enter PIN:");
      do{
        strcpy(tryPIN, "_   ");
        char* l2;
        if(tripped){
          screen.write(254);
          screen.write(192);
          screen.write("!!!   ");
          screen.write(tryPIN);
          screen.write("   !!!");
        }else{
          screen.write(254);
          screen.write(192);
          screen.write("      ");
          screen.write(tryPIN);
          screen.write("      ");
        }
        for(int i = 0; i < 4; i++){
          do{
            if(!tripped){
              tripped = alarmTripped();
              if(tripped){
                screen.write(254);
                screen.write(192);
                screen.write("!!!");
                screen.write(254);
                screen.write(205);
                screen.write("!!!");
              }
            }
            if(tripped){
              soundAlarm();
            }
            ch = input();
          }while(ch == '\0');
          tryPIN[i] = ch;
          if(i < 3){
            tryPIN[i + 1] = '_';
          }
          screen.write(254);
          screen.write(198);
          screen.write(tryPIN);
        }
        if(strncmp(tryPIN, PIN, 4) == 0){
          correct = true;
        }
        if(!correct){
          tries++;
          if(!tripped && tries >= 3){
            soundAlarm();
          }
          screenLineTwo("!!!   XXXX   !!!");
          delay(300);
        }
      }while(!correct);
      endAlarm();
      screenLineOne("       o o      ");
      screenLineTwo("        U       ");
      delay(2000);
      if(testing){
        state = ARMED;
        testing = false;
      }else{
        state = DISARMED;
      }
    }
  public:
    /*
     * Just a constructor, nothing special here. Oh, the reference variables mean that
     * we can instantiate the components for the SecuritySystem outside and then include
     * those components in the SecuritySystem object.
     */
    SecuritySystem(JKeypad &Lkeypad, SoftwareSerial &Lscreen, NewPing &Lsonar):
    calibratedDistance(-1), keypad(Lkeypad), screen(Lscreen), state(SETPIN), 
    sonar(Lsonar), tripped(false), testing(false), firstTrip(0),
    redPin0(11), redPin1(10), bluePin0(9), bluePin1(6), buzPin(5){
      strncpy(PIN, "    ", 4);
    }
    /*
     * setPIN() begins the sequence for the user to set the PIN. This is called when
     * the system powers up.
     */
    void setPIN(){
      char ch;
      Serial.println("Printing to screen");
      clearScreen();
      screenLineOne("Set PIN");
      strncpy(PIN, "_   ", 4);
      screen.write(254);
      screen.write(192);
      screen.write("      ");
      screen.write(PIN);
      screen.write("      ");
      Serial.println("Grabbing new PIN");
      for(int i = 0; i < 4; i++){
        do{
          ch = input();
        }while(ch == '\0');
        PIN[i] = ch;
        if(i < 3){
          PIN[i+1] = '_';
        }
        screen.write(254);
        screen.write(198);
        screen.write(PIN);
      }
      clearScreen();
      screenLineOne("PIN set to:");
      screen.write(254);
      screen.write(198);
      screen.write(PIN);
      delay(1000);
      if(calibratedDistance < 0){
        state = CALIBRATE;
      }else{
        state = DISARMED;
      }
    }
    /*
     * calibrate() is the beginning of the calibration mode. This allows the user to
     * decide when they're ready to start calibrating, because the calibrate mode
     * is begun immediately after the PIN is first set if the system sees that it
     * has yet to calibrate.
     */
    void calibrate(){
      clearScreen();
      screenLineOne("Press A to start");
      screenLineTwo("calibration");
      char ch;
      do{
        ch = input();
      }while(ch != 'A');
      calibrateRoutine();
      if(calibratedDistance >= 0){
        state = DISARMED; 
      }else{
        state = CALIBRATE;
      }
    }
    /*
     * input() returns whatever input has been received from the keypad. This is
     * a null character ('\0') if there is no input. Because each call to getPress()
     * only runs through each character once, input() is best used inside of a do-while
     * loop so that the system can wait for user input.
     */
    char input(){
      return keypad.getPress();
    }
    /*
     * getState() returns the current state so that the outside world can make decisions
     * based on the current state. Making these methods public may not be the best plan,
     * but it made sense at the time.
     */
    State getState(){
      return state;
    }
    /*
     * alarm() begins the alarm mode, but because this mode is only begun from within the
     * class's methods, we may just remove this method entirely.
     */
    bool alarm(){
      
    }
    /*
     * disarmed() begins the disarmed mode for the system. Really, it's just a menu screen.
     */
    void disarmed(){
      char ch;
      clearScreen();
      screenLineOne("NOT ARMED  A-Arm");
      screenLineTwo("B-set PIN  C-Cal");
      do{
        ch = input();
      }while(ch != 'A' && ch != 'B' && ch != 'C');
      switch(ch){
        case 'A':
          state = ARMED;
          break;
        case 'B':
          state = SETPIN;
          break;
        case 'C':
          state = CALIBRATE;
          break;
        default:
          break;
      }
    }
    /*
     * armed() begins the armed mode for the system. This is a menu screen, but it should
     * also be checking if the alarm is tripped.
     */
    bool armed(){
      char ch;
      bool tripped;
      clearScreen();
      screenLineOne("     ARMED      ");
      screenLineTwo("A-test  D-Disarm");
      do{
        ch = input();
        tripped = alarmTripped();
      }while(ch != 'A' && ch != 'D' && !tripped);
      if(tripped){
        soundAlarm();
      }
      switch(ch){
        case 'A':
          testAlarm();
          break;
        case '\0':
        case 'D':
          validatePIN();
          break;
        default:
          state = ARMED;
          break;
      }
    }
};

JKeypad keypad(A4/*9*/,8,7,A5/*6*/,13,12,A2/*11*/,A3/*10*/);

//11 10 9 6 5
const int trigPin = A0;
const int echoPin = A1;
SoftwareSerial LCD(3, 2);
NewPing sonar(trigPin,echoPin,100);
/*
 * LCD: send 254, then the cursor positions
 * positions:
 * 128-143
 * 192-207
 */
SecuritySystem sys(keypad, LCD, sonar);

void setup() {
  LCD.begin(9600);
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  switch(sys.getState()){
    case SETPIN:
      Serial.println("SETPIN");
      sys.setPIN();
      break;
    case CALIBRATE:
      sys.calibrate();
      break;
    case DISARMED:
      sys.disarmed();
      break;
    case ARMED:
      sys.armed();
      break;
    case ALARM:
      sys.alarm();
      break;
    default:
      break;
  }
}
