/*
Dette programmet benytter en Arduino UNO for å automatisk overvåke fuktigheten i plantejord ved bruk av en fuktighetssensor. 
Et potensiometer setter et fuktighetsnivå som fuktigheten i jorden skal holdes over. 
Om fuktighethen går under det satte nivået vil en piezo buzzer aktiveres slik at brukeren så får vite at planten skal vannes. 
Systemet overvåker planten hele døgnet, men buzzeren aktiveres ikke mellom 00.00-08.00.

Utstyr:
Arduino UNO R3
DS1302 rtc modul
DC buzzer
10k ohm variabel motstand
20 4 LCD skjerm med I2C adapter
DFROBOT capacitive soil moisture sensor V1.0
250 ohm resistor
Arduino USB cable
*/

//include libraries
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h> //rtc by makuna library

//lcd
LiquidCrystal_I2C lcd(0x27,20,4);  // LCD adress: 0x27, størrelse 20 characthers x 4 rows

//rtc modul
ThreeWire myWire(7,6,8); //dat, clk, rst
RtcDS1302<ThreeWire> Rtc(myWire);

//pins
const int potentiometer_pin = A3;
const int moisture_sensor_pin = A2;
const int buzzer_pin = 2; 

//constants
int potentiometer_value = 0;
int moisture_threshold = 0; 
int moisture_level = 0; 
int sleep_start = 0; 
int sleep_end = 8;

//functions
int sensorval_to_percent(int sensor_input){
  //Denne funksjonen omgjør sensormåligene til fuktighetsprosent.
  return -0.398*sensor_input + 199;
}

void setup() {
  //start serial communication 
  Serial.begin(9600);
  //pin setup
  pinMode(buzzer_pin, OUTPUT);
  pinMode(moisture_sensor_pin, INPUT);

  //lcd setup
  lcd.init();
  lcd.clear();
  lcd.backlight(); //skru på backlight

  //rtc setup
  Rtc.Begin();
  //set rtc time to the time of the computer
  RtcDateTime currentTime = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(currentTime);
}

void loop() {
  //read pot
  potentiometer_value = analogRead(potentiometer_pin);
  moisture_threshold = map(potentiometer_value, 0, 1023, 0, 100);
  //read current moisture level
  moisture_level = sensorval_to_percent(analogRead(moisture_sensor_pin));
  //read time
  RtcDateTime now = Rtc.GetDateTime();

  //print pot level to lcd
  lcd.setCursor(0,0);
  lcd.print("Threshold: ");
  if(moisture_threshold < 100){
    lcd.print("0");
  } 
  if(moisture_threshold < 10){
    lcd.print("0");
  } 
  lcd.print(moisture_threshold);
  lcd.print("%");

  //print current moisture level:
  lcd.setCursor(0,1);
  lcd.print("Moisture level: ");
  lcd.print(moisture_level);
  lcd.print("%");
  
  //print time to lcd
  lcd.setCursor(15,3);
  if (now.Minute() < 10) {
    lcd.print(now.Hour());
    lcd.print(":");
    lcd.print("0");
    lcd.print(now.Minute());
  } else {
    lcd.print(now.Hour());
    lcd.print(":");
    lcd.print(now.Minute());
  }

  //buzzer if moisture level < moisture_threshold and time not between sleep_start and sleep_end 
  if ((moisture_level <= moisture_threshold)){
    if (now.Hour() >= sleep_start && now.Hour() < sleep_end){
      //Serial.println("zzzzzzzz");
    } else{
      digitalWrite(buzzer_pin, HIGH);
    }
  } else {
    digitalWrite(buzzer_pin, LOW);
  }
  delay(100);
}