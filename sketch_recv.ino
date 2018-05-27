#include "rgb_lcd.h"

#define ROTARY_ANGLE_SENSOR A0
#define ADC_REF 5
#define GROVE_VCC 5//VCC of the grove interface is normally 5v
#define FULL_ANGLE 300//full value of the rotary angle is 300 degrees

rgb_lcd lcd;

int colorR = 0;
int colorG = 0;
int colorB = 0;

#define MORSE_SYNC      3000
#define MORSE_PAUSE_IS  500
#define MORSE_PAUSE_IC  1500
#define MORSE_LONG      1500
#define MORSE_SHORT     500

volatile byte state = LOW;
volatile bool sync_det = false;
volatile int high_state_t = 0;
volatile int high_state_delta = 0;
volatile int low_state_t = 0;
volatile int low_state_delta = 0;

volatile int i = 0;

volatile bool char_finished = false;
volatile bool trans_finished = false;
volatile char recv_char[6];
String recv_msg;
volatile bool receiving = false;

int received = 0;

int actor_on = 0;




void setup() {

  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), recv_lvl_chg, CHANGE);

  pinMode(4, OUTPUT);
  pinMode(8, OUTPUT);
  
  lcd.begin(16, 2);

  lcd.setRGB(colorR, colorG, colorB);

  // Print a message to the LCD.
  //lcd.print("Hello");

}

void loop() {

  int degrees;
  degrees = getDegree();

  actor_on = map(degrees, 0, FULL_ANGLE, 0, 100);


  lcd.clear();
  if (receiving) {
    lcd.setRGB(colorR, colorG, 255);
    lcd.print("Empfange: ");
    lcd.print(recv_msg);
  } else {
    //lcd.setRGB(colorR, 255, colorB);
    //lcd.print("Ready to receive");
    //lcd.setCursor(0, 1);
    lcd.print("Schwellwert:");
    lcd.print(actor_on);
    lcd.setCursor(0, 1);
    lcd.print("Empfangen: ");
    lcd.print(received);
  }
  lcd.setCursor(0, 1);
  //lcd.print(low_state_delta);
  char temp[6];
  temp[0] = recv_char[0];
  temp[1] = recv_char[1];
  temp[2] = recv_char[2];
  temp[3] = recv_char[3];
  temp[4] = recv_char[4];
  temp[5] = '\0';
  String temp2 = String(temp);
  //if (i > 0) lcd.print(recv_char[i-1]);
  //lcd.print(char_finished);
  //lcd.print(received);
  delay(50);
  Serial.println(temp2);

  if (char_finished) {
    if (temp2.equals(".----")) recv_msg = String(recv_msg + "1");
    if (temp2.equals("..---")) recv_msg = String(recv_msg + "2");
    if (temp2.equals("...--")) recv_msg = String(recv_msg + "3");
    if (temp2.equals("....-")) recv_msg = String(recv_msg + "4");
    if (temp2.equals(".....")) recv_msg = String(recv_msg + "5");
    if (temp2.equals("-....")) recv_msg = String(recv_msg + "6");
    if (temp2.equals("--...")) recv_msg = String(recv_msg + "7");
    if (temp2.equals("---..")) recv_msg = String(recv_msg + "8");
    if (temp2.equals("----.")) recv_msg = String(recv_msg + "9");
    if (temp2.equals("-----")) recv_msg = String(recv_msg + "0");
    char_finished = false;
    i = 0;
  }

  if (trans_finished) {
    trans_finished = false;
    received = recv_msg.toInt();
    recv_msg = String("");
  }

  if (received <= actor_on) {
    digitalWrite(4, HIGH);
    if (!receiving) lcd.setRGB(colorR, 255, colorB);
  } else {
    digitalWrite(4, LOW);
    if (!receiving) lcd.setRGB(255, colorG, colorB);
  }

}

void recv_lvl_chg() {
  state = !state;
  receiving = true;
  if (state == HIGH) {
    digitalWrite(8, HIGH);
    high_state_t = millis();
    if (sync_det) low_state_delta = millis() - low_state_t;
    if (low_state_delta > MORSE_PAUSE_IC - 50 && low_state_delta < MORSE_PAUSE_IC + 50) {
      char_finished = true;
      low_state_delta = 0;
    }
  }
  if (state == LOW) {
    digitalWrite(8, LOW);
    high_state_delta = millis() - high_state_t;
    if (high_state_delta > MORSE_SYNC - 50 && high_state_delta < MORSE_SYNC + 50) {
      if (sync_det) {
        trans_finished = true;
        receiving = false;
      }
      sync_det = !sync_det;
    }
    if (sync_det) low_state_t = millis();
    if (high_state_delta > MORSE_LONG - 50 && high_state_delta < MORSE_LONG + 50) {
      recv_char[i] = '-';
      i++;
    }
    if (high_state_delta > MORSE_SHORT - 50 && high_state_delta < MORSE_SHORT + 50) {
      recv_char[i] = '.';
      i++;
    }

  }

}


/************************************************************************/
/*Function: Get the angle between the mark and the starting position    */
/*Parameter:-void                                                       */
/*Return:   -int,the range of degrees is 0~300                          */
int getDegree()
{
  int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
  float voltage;
  voltage = (float)sensor_value*ADC_REF/1023;
  float degrees = (voltage*FULL_ANGLE)/GROVE_VCC;
  return degrees;
}
