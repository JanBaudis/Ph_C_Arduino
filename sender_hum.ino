#include <FreqCounter.h>
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

#define MORSE_SYNC      3000
#define MORSE_PAUSE_IS  500
#define MORSE_PAUSE_IC  1500
#define MORSE_LONG      1500
#define MORSE_SHORT     500

const int colorR = 0;
const int colorG = 0;
const int colorB = 0;
int frq_mapped;

long int frq;

volatile byte state = LOW;

bool init_frq = false;

long int frq_dry=0;
long int frq_wet=0;

void setup(){

    // initialize the pushbutton pin as an input:
    pinMode(2, INPUT);
    attachInterrupt(digitalPinToInterrupt(2), button, RISING);

    Serial.begin(57600);
    lcd.begin(16, 2);
    
    lcd.setRGB(colorR, 255, colorB);
    

    // initialize digital pin as an output.
    pinMode(12, OUTPUT);

    
    // 2 Dummy Measurements since otherwise i got bad first results
    FreqCounter::f_comp= 8;
    FreqCounter::start(1000);
    while (FreqCounter::f_ready == 0)         // wait until counter ready

    frq=FreqCounter::f_freq;            // read result

    FreqCounter::f_comp= 8;
    FreqCounter::start(1000);
    while (FreqCounter::f_ready == 0)         // wait until counter ready

    frq=FreqCounter::f_freq;            // read result

    delay(100);
}


void loop()
{

    FreqCounter::f_comp= 8;
    FreqCounter::start(1000);
    while (FreqCounter::f_ready == 0)         // wait until counter ready
    frq=FreqCounter::f_freq;            // read result
    
    frq_mapped=map(frq, frq_wet, frq_dry, 100, 0);
    if (frq_mapped < 0) {
      frq_mapped = 0;
    }
    if (frq_mapped > 100) {
      frq_mapped = 100;
    }
    
    lcd.clear();
    //lcd.print("Frequency:");
    //lcd.print(frq);
    //lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("Feuchtigkeit:");
    lcd.print(frq_mapped);
    delay(20);                      // wait for a second
    /*if (frq_mapped >= 70) {
      lcd.setRGB(0, 255, 0);
    } else {
      lcd.setRGB(255, 0, 0);
    }*/

    if (!init_frq) {
      while (state == LOW){
        FreqCounter::f_comp= 8;
        FreqCounter::start(1000);
        while (FreqCounter::f_ready == 0)         // wait until counter ready
        frq=FreqCounter::f_freq;            // read result
        lcd.clear();
        lcd.print("Kalibr. trocken");
        lcd.setCursor(0, 1);
        lcd.print(frq);
        delay(100);
      }
      state = LOW;
      frq_dry=frq;
      while (state == LOW){
        FreqCounter::f_comp= 8;
        FreqCounter::start(1000);
        while (FreqCounter::f_ready == 0)         // wait until counter ready
        frq=FreqCounter::f_freq;            // read result
        lcd.clear();
        lcd.print("Kalibr. nass");
        lcd.setCursor(0, 1);
        lcd.print(frq);
        delay(100);
      }
      state = LOW;
      frq_wet=frq;
      init_frq=true;
    }

    if (state == HIGH) {
      lcd.clear();
      lcd.setRGB(0, 0, 255);
      lcd.print("Sende...");
      lcd.setCursor(0, 1);
      // print the number of seconds since reset:
      lcd.print(frq_mapped);
      
      digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(MORSE_SYNC);                       // wait for a second
      digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
      delay(MORSE_PAUSE_IS);                       // wait for a second
      int temp;
      temp = frq_mapped % 100;
      if (temp == 0 && frq_mapped != 0) {
        send_char(1);
        delay(MORSE_PAUSE_IC - MORSE_PAUSE_IS);
        send_char(0);
        delay(MORSE_PAUSE_IC - MORSE_PAUSE_IS);
        send_char(0);
        delay(MORSE_PAUSE_IC - MORSE_PAUSE_IS);
      } else {
        if ((int)temp/10 != 0) {
          send_char((int)temp/10);
          delay(MORSE_PAUSE_IC - MORSE_PAUSE_IS);
        }
        send_char((int)(temp%10));
        delay(MORSE_PAUSE_IC - MORSE_PAUSE_IS);
      }
      digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(MORSE_SYNC);                       // wait for a second
      digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
      delay(MORSE_PAUSE_IS);                       // wait for a second
      
    } 
    state = LOW;

    lcd.setRGB(colorR, 255, colorB);
    
    delay(400);
}

void button() {
  state = HIGH;
}

void send_char(int send) { // currently only 0-9
  switch (send) {
    case 0:
      morse_long();
      morse_long();
      morse_long();
      morse_long();
      morse_long();
      break;
    case 1:
      morse_short();
      morse_long();
      morse_long();
      morse_long();
      morse_long();
      break;
    case 2:
      morse_short();
      morse_short();
      morse_long();
      morse_long();
      morse_long();
      break;
    case 3:
      morse_short();
      morse_short();
      morse_short();
      morse_long();
      morse_long();
      break;
    case 4:
      morse_short();
      morse_short();
      morse_short();
      morse_short();
      morse_long();
      break;
    case 5:
      morse_short();
      morse_short();
      morse_short();
      morse_short();
      morse_short();
      break;
    case 6:
      morse_long();
      morse_short();
      morse_short();
      morse_short();
      morse_short();
      break;
    case 7:
      morse_long();
      morse_long();
      morse_short();
      morse_short();
      morse_short();
      break;
    case 8:
      morse_long();
      morse_long();
      morse_long();
      morse_short();
      morse_short();
      break;
    case 9:
      morse_long();
      morse_long();
      morse_long();
      morse_long();
      morse_short();
      break;
    default:
      // statements
      break;
  }
}

void morse_short() {
  digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(MORSE_SHORT);                       // wait for a second
  digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
  delay(MORSE_PAUSE_IS);                       // wait for a second
}

void morse_long() {
  digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(MORSE_LONG);                       // wait for a second
  digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
  delay(MORSE_PAUSE_IS);                       // wait for a second
}

