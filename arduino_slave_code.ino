
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
//#include "pins.h" //wiring volgens pcb ontwerp
#include "pinsShield.h" //wegens andere wiring met voorlopig shield voor demo


//Constants Tim

LiquidCrystal_I2C lcd(0x3f, 20,  4);


// Communication with Main
String value0;
String value1;
const byte numChars = 32;
char receivedChars0[numChars];
char receivedChars1[numChars];
boolean newData0 = false;
boolean newData1 = false;

int sendDelay = 100;

//Constants Hans

// define bytes to send
unsigned int RR = 20;    // Number of breaths per minute setting
unsigned int VT = 600;    // Tidal volume= target to deliver
unsigned int PK = 50;   //Peak pressure
unsigned int TS = 0;    // Breath Trigger Sensitivity = amount the machine should look for
unsigned int IE = 1;           // Inspiration-expiration rate
unsigned int PP = 0;    // PEEP Pressure = Max pressure to deliver --> Manueel instellen op peep valve
//waarde moet wel gekend zijn in het systeem (en moet dus instelbaar zijn)

unsigned int ADPK = 10; // Allowed Deviation Peak pressure
unsigned int ADVT = 10; // Allowed Deviation Tidal Volume
unsigned int ADPP = 5;  // Allowed Deviation PEEP

bool MODE = false; // Mode (false = Pressure)
bool MUTE = false; // Mute alarms (true = mute alarms)
bool ACTIVE = false; // Start/stop (true = start)

bool flag = false;


// constants won't change. They're used here to set pin numbers:
int buttonState;             // the current reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers
uint32_t lastRequestedValueTime = 0;
uint32_t loopMillis;
int k = 0;
int al = 0;
const int numButtons = 20;
unsigned long lastDebounceTime[2][numButtons] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}; // the last time the output pin was toggled & the time the button was pressed the first time


//make an array with all button pins and last button state
int buttonPins[2][numButtons] = { {
    BUTTON_VOLUME_DOWN,
    BUTTON_VOLUME_UP,
    BUTTON_PRESSURE_DOWN,
    BUTTON_PRESSURE_UP,
    BUTTON_TRIG_DOWN,
    BUTTON_TRIG_UP,
    BUTTON_RR_DOWN,
    BUTTON_RR_UP,
    BUTTON_START_STOP,
    BUTTON_MUTE,
    BUTTON_HOLD,
    BUTTON_MODE,
    BUTTON_PRESSURE_ALARM_DOWN,
    BUTTON_PRESSURE_ALARM_UP,
    BUTTON_VOLUME_ALARM_DOWN,
    BUTTON_VOLUME_ALARM_UP,
    BUTTON_PEEP_ALARM_DOWN,
    BUTTON_PEEP_ALARM_UP,
    BUTTON_IE_DOWN,
    BUTTON_IE_UP
  }, {
    HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH
  }
};
int buttons [numButtons] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Constants Loes
#define POS_VOLUME_DOWN     0
#define POS_VOLUME_UP       1
#define POS_PRESSURE_DOWN   2
#define POS_PRESSURE_UP     3
#define POS_TRIG_DOWN       4
#define POS_TRIG_UP         5
#define POS_RR_DOWN        6
#define POS_RR_UP          7
#define POS_START_STOP      8
#define POS_MUTE            9
#define POS_HOLD            10
#define POS_MODE            11
#define POS_PRESSURE_ALARM_DOWN 12
#define POS_PRESSURE_ALARM_UP   13
#define POS_VOLUME_ALARM_DOWN   14
#define POS_VOLUME_ALARM_UP     15
#define POS_PEEP_ALARM_DOWN     16
#define POS_PEEP_ALARM_UP       17 // no worky
#define POS_IE_DOWN             18
#define POS_IE_UP               19


bool test = false;
bool alarm = false;

void setup() {

  Serial1.begin(115200);
  //Serial1.println("Init LCD");
  //init LCD
  lcd.backlight();
  lcd.init();
  //print values void
  Serial1.println("Print letters");
  printLetters();

  Serial1.println("Wacht op OK");
  while (newData1 == 0 ) {
    recvWithEndMarkerSer1(); //keep looking
    if (newData1 == true) {
      //processSerialPort(receivedChars1);
      if (receivedChars1[0] == 'O' && receivedChars1[1] == 'K') {
        //        Serial1.println("Joepie!");
      } else {
        Serial1.print(receivedChars1);
        newData1 = false;
      }

    }
    //dump all data
    for (int i = 0; i < numButtons; i++) {
      buttons[i] = 1;
    }
    serialSend();
  }




  //declare all buttons
  pinMode(BUTTON_VOLUME_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_UP, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_UP, INPUT_PULLUP);
  pinMode(BUTTON_TRIG_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_TRIG_UP, INPUT_PULLUP);
  pinMode(BUTTON_RR_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_RR_UP, INPUT_PULLUP);
  pinMode(BUTTON_START_STOP, INPUT_PULLUP);
  pinMode(BUTTON_MUTE, INPUT_PULLUP);
  pinMode(BUTTON_HOLD, INPUT_PULLUP);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_ALARM_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_ALARM_UP, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_ALARM_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_ALARM_UP, INPUT_PULLUP);
  pinMode(BUTTON_PEEP_ALARM_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PEEP_ALARM_UP, INPUT_PULLUP);
  pinMode(BUTTON_IE_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_IE_UP, INPUT_PULLUP);
}

void loop() {

  loopMillis = millis();
  //read button inputs
  buttonsRead();
  //array with buttons is filled with data


  //edit parameters

  //send parameters
  if (flag == true) {
    serialSend();
    flag = false;
  }
  delay(10);
  //      flag = true;

  //Update screen
  printValues();
  //wacht op PC alive: indien niet: alarm
  while (Serial1.available() > 0) {
    recvWithEndMarkerSer1(); //keep looking
    if (newData0 == true) {
      if (receivedChars0[0] == 'A')
      { //als hij A ontvangt

        newData0 = false;
        if (alarm) { //als het alarm actief was
          alarm = false;
          //Serial.print("Back Alive.");
          al = 0;
        }
        else // als het alarm niet actief was
        {
          //Serial.print("Still Alive.");
          alarm = false;
          al = 0;
        }
      } else { //als hij iets anders ontvangt
        break; //ik denk dat deze else weg mag.
      }

    }

  }




  if (alarm) {
    alarmInitiated();
  }

  delay(100);
  //iedere seconde data dumpen
  k++;
  al++;
  if (al > 100)
  {
    alarm = true;
  }

  /*if (k > 9) {
    //dump all data
    for (int i = 0; i < numButtons; i++) {
      buttons[i] = 1;
    }
    serialSend();
    k = 0;
  }*/ //efkes niet elke seconde data dumpen


}


void buttonsRead() {
  //read out the buttons with debounce
  for (int i = 0; i < numButtons; i++)
  {
    //sample the state of the button - is it pressed or not?
    int  reading = digitalRead(buttonPins[0][i]);
    //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime[0][i]) > debounceDelay) {
      if ( reading == HIGH && buttonPins[1][i] != HIGH) {
        buttonPins[1][i] = reading;
        lastDebounceTime[0][i] = millis(); //set the current time
        lastDebounceTime[1][i] = 0; //reset the first time counter for jumpvalue
      }
      else if ( reading == LOW && buttonPins[1][i] != LOW) {
        buttonPins[1][i] = reading;
        lastDebounceTime[0][i] = millis(); //set the current time

        if (lastDebounceTime[1][i] == 0) //this is for the jumpvalue. if the counter is zero
        {
          lastDebounceTime[1][i] = millis(); //set the current time to the first time counter for jumpvalue
          flag = true;
        }

      }//close if/else
    }//close if(time buffer)
  }

  //Testmodule buttons
  /*
    for (int i = 0; i < numButtons; i++)
    {
    Serial1.print(buttonPins[1][i]);
    }
    Serial1.println();
  */

  //invert the numbers
  for (int i = 0; i < numButtons; i++)
  {
    if (buttonPins[1][i] == 0) {
      buttons[i] = 1;
    }
    else {
      buttons[i] = 0;
    }

  }



  //make the buttons edit the values
  //RR
  RR = RR +  buttons[POS_RR_UP] * jumpValue(POS_RR_UP);
  if (RR > 1) {
    RR = RR -  buttons[POS_RR_DOWN] * jumpValue(POS_RR_DOWN);
  }
  // VT

  VT = VT +  buttons[POS_VOLUME_UP] * jumpValue(POS_VOLUME_UP);
  if (VT > 1) {
    VT = VT -  buttons[POS_VOLUME_DOWN] * jumpValue(POS_VOLUME_DOWN);
  }  // PK
  PK = PK +  buttons[POS_PRESSURE_UP] * jumpValue(POS_PRESSURE_UP);
  if (PK > 1) {
    PK = PK -  buttons[POS_PRESSURE_DOWN] * jumpValue(POS_PRESSURE_DOWN);
  }//TS
  TS = TS +  buttons[POS_TRIG_UP] * jumpValue(POS_TRIG_UP);
  if (TS > 1) {
    TS = TS -  buttons[POS_TRIG_DOWN] * jumpValue(POS_TRIG_DOWN);
  }//PP
  PP = PP +  buttons[POS_IE_UP] * jumpValue(POS_IE_UP);
  if (PP > 1) {
    PP = PP -  buttons[POS_IE_DOWN] * jumpValue(POS_IE_DOWN);
  }  // IE
  if (IE < 3) {
    IE = IE + buttons[POS_HOLD];
  }
  if (IE = 3) {
    IE = 1;
  }
  // ADPK

  ADPK = ADPK +  buttons[POS_PRESSURE_ALARM_UP] * jumpValue(POS_PRESSURE_ALARM_UP);
  if (ADPK > 1) {
    ADPK = ADPK -  buttons[POS_PRESSURE_ALARM_DOWN] * jumpValue(POS_PRESSURE_ALARM_DOWN);
  } // ADVT
  ADVT = ADVT +  buttons[POS_VOLUME_ALARM_UP] * jumpValue(POS_VOLUME_ALARM_UP);
  if (ADVT > 1) {
    ADVT = ADVT -  buttons[POS_VOLUME_ALARM_DOWN] * jumpValue(POS_VOLUME_ALARM_DOWN);
  }// ADPP
  ADPP = ADPP +  buttons[POS_PEEP_ALARM_UP] * jumpValue(POS_PEEP_ALARM_UP);
  if (ADPP > 1) {
    ADPP = ADPP -  buttons[POS_PEEP_ALARM_DOWN] * jumpValue(POS_PEEP_ALARM_DOWN);
  }
  //mode button here
  //if mode is VOLUME and button is pressed: goto PRESSURE
  if (buttons[POS_MODE] == 1)
  {
    MODE = !MODE;
    delay(50);
  }


  if (buttons[POS_MUTE] == 1)
  {
    MUTE = !MUTE;
    delay(50);
  }


  // TODO: hold, mute, start/stop
  if (flag) {
    clearValues();
  }

}


int jumpValue(int i) {

  //  if (loopMillis - lastRequestedValueTime < 200) {
  //    return 0;
  // }
  //  lastRequestedValueTime = loopMillis;
  //Serial1.println(loopMillis);
  //Serial1.println(lastDebounceTime[1][i]);
  // Serial1.println(loopMillis-lastDebounceTime[1][0]);



  if ( loopMillis - lastDebounceTime[1][i] > 4000) {
    return 10;
  }
  if ( loopMillis - lastDebounceTime[1][i] > 2000) {
    return 5;
  }
  if (loopMillis - lastDebounceTime[1][i] > 1000) {
    return 3;
  }
  if (loopMillis - lastDebounceTime[1][i] > 500) {
    return 2;
  }
  return 1;
}

void serialSend()
{
  // RR
  if ((buttons[POS_RR_UP] == 1) || (buttons[POS_RR_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("RR=");
    Serial1.println(RR);
  }
  // VT
  if ((buttons[POS_VOLUME_UP] == 1) || (buttons[POS_VOLUME_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("VT=");
    Serial1.println(VT);
  }
  // PK
  if ((buttons[POS_PRESSURE_UP] == 1) || (buttons[POS_PRESSURE_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("PK=");
    Serial1.println(PK);
  }
  // TS
  if ((buttons[POS_TRIG_UP] == 1) || (buttons[POS_TRIG_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("TS=");
    Serial1.println(TS);
  }
  // IE -->wordt gepruikt voor PP ipv IE, IE wordt veranderd door HOLD.
  if ((buttons[POS_IE_UP] == 1) || (buttons[POS_IE_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("PP=");
    Serial1.println(PP);
  }
  // PP --> kunnen we niet doorsturen, nu dus wel weer :)

  // IE op positie HOLD
  if (buttons[POS_HOLD] == 1) {
    delay(sendDelay);
    Serial1.print("IE=");
    float IEfloat = 1 / IE; //wordt over uart verstuurd als float
    Serial1.println(IEfloat);
  }

  // ADPK
  if ((buttons[POS_PRESSURE_ALARM_UP] == 1) || (buttons[POS_PRESSURE_ALARM_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("ADPK=");
    Serial1.println(ADPK);
  }
  // ADVT
  if ((buttons[POS_VOLUME_ALARM_UP] == 1) || (buttons[POS_VOLUME_ALARM_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("ADVT=");
    Serial1.println(ADVT);
  }
  // ADPP
  if ((buttons[POS_PEEP_ALARM_UP] == 1) || (buttons[POS_PEEP_ALARM_DOWN] == 1)) {
    delay(sendDelay);
    Serial1.print("ADPP=");
    Serial1.println(ADPP);
  }

  // MODE
  if (buttons[POS_MODE] == 1) {
    delay(sendDelay); //delay is nodig voor inlezen, aanvraag van thomas
    Serial1.print("MODE=");
    Serial1.println(MODE);
  }

  // MUTE
  if (buttons[POS_MUTE] == 1) {
    delay(sendDelay); //delay is nodig voor inlezen, aanvraag van thomas
    Serial1.print("MUTE=");
    Serial1.println(MUTE);
  }

  // ACTIVE
  if (buttons[POS_START_STOP] == 1) {
    delay(sendDelay); //delay is nodig voor inlezen, aanvraag van thomas
    Serial1.print("ACTIVE=");
    Serial1.println(ACTIVE);
  }

}

void printValues() {
  lcd.setCursor(12, 2);
  if (MODE == true) lcd.print("VOLUME");
  if (MODE == false) lcd.print("PRESSURE");
  lcd.setCursor(3, 0);
  lcd.print(PK);
  lcd.setCursor(7, 0);
  lcd.print(ADPK);
  lcd.setCursor(3, 1);
  lcd.print(VT);
  lcd.setCursor(7, 1);
  lcd.print(ADVT);
  lcd.setCursor(15, 0);
  lcd.print(RR);
  lcd.setCursor(15, 1);
  lcd.print(TS);
  lcd.setCursor(3, 2);
  lcd.print(PP);
  lcd.setCursor(7, 2);
  lcd.print(ADPP);
  if (MUTE) {
    lcd.setCursor(0, 2);
    lcd.print("MUTE");
  }
  else {
    lcd.setCursor(0, 2);
    lcd.print("    ");
  }


  lcd.setCursor(17, 3);
  lcd.print(IE);
}
void printLetters()
{
  lcd.setCursor(0, 0);
  lcd.print("PK");
  lcd.setCursor(0, 1);
  lcd.print("VT");
  lcd.setCursor(12, 0);
  lcd.print("RR");
  lcd.setCursor(12, 1);
  lcd.print("TS");
  lcd.setCursor(0, 2);
  lcd.print("PP");
  lcd.setCursor(12, 3);
  lcd.print("IE");
  lcd.setCursor(15, 3);
  lcd.print("1/");
}

void clearValues()
{
  lcd.setCursor(12, 2);
  lcd.print("        ");
  lcd.setCursor(3, 0);
  lcd.print("   ");
  lcd.setCursor(7, 0);
  lcd.print("   ");
  lcd.setCursor(3, 1);
  lcd.print("   ");
  lcd.setCursor(7, 1);
  lcd.print("   ");
  lcd.setCursor(15, 0);
  lcd.print("   ");
  lcd.setCursor(15, 1);
  lcd.print("   ");
  lcd.setCursor(3, 2);
  lcd.print("   ");
  lcd.setCursor(7, 2);
  lcd.print("   ");
  lcd.setCursor(17, 3);
  lcd.print(" ");
}

void recvWithEndMarkerSer1() {
  //this waits for confirmation over Serial1 that the data is OK

  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  //if there is still data in the tube
  while (Serial1.available() > 0 && newData1 == false) {
    rc = Serial1.read(); //keep reading
    if (rc != endMarker) { //if there is no endmarker detected
      receivedChars1[ndx] = rc; //put chars in array
      ndx++; //make array larger
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else { //endmarker received
      receivedChars1[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData1 = true;
      Serial1.println(receivedChars1);
    }
  }
}


void recvWithEndMarkerSer0() {
  //this waits for confirmation over Serial1 that the data is OK

  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  //if there is still data in the tube
  while (Serial.available() > 0 && newData0 == false) {
    rc = Serial.read(); //keep reading
    if (rc != endMarker) { //if there is no endmarker detected
      receivedChars0[ndx] = rc; //put chars in array
      ndx++; //make array larger
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else { //endmarker received
      receivedChars0[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData0 = true;
      Serial.println(receivedChars0);
    }
  }
}

void alarmInitiated()
{
  /*
    # 1    (1 << 0) Mechanical failure
    # 2    (1 << 1) Power loss
    # 4    (1 << 2) Watchdog Timout (connection loss)
    # 8    (1 << 3) no pressure
    # 16   (1 << 4) no flow
    # 32   (1 << 5) peak pressure deviation exceeded
    # 64   (1 << 6) peep deviation exceeded
    # 128  (1 << 7) volume deviation exceeded
    # 256  (1 << 8) trigger timeout
  */
  //zolang mute alarm niet wordt gedrukt
  while (!MUTE)
  {
    //lcd flikkert

    Serial1.println("ALARM!");
    lcd.setCursor(1, 3);
    lcd.print("ALARM!");
    //"alarm" komt op LCD
    //type alarm komt op LCD
    buttonsRead();
  }
  Serial1.println("ALARM MUTED");
  lcd.setCursor(1, 3);
  lcd.print("       ");

}
