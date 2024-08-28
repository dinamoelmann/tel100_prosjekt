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
Generic pushbutton
x2 250 ohm resistor
Arduino USB cable

Når programmet initialiseres starter en kallibreringssekvens der den relative fuktigheten i luft og vann settes til 
en fast maximum- og minimumsverdi beregnet utfra gjennomsnittet av 100 målinger. 
Når kallibreringen er utført starter overvåkingen av planten med basis i de registrerte maksimums- og minimumsverdiene.
*/

//include libraries
#include <LiquidCrystal_I2C.h> //bibliotek som støtter LCD operasjoner gjennom I2C
#include <ThreeWire.h>         //makuna library
#include <RtcDS1302.h>         //rtc by makuna library

//lcd
LiquidCrystal_I2C lcd(0x27,20,4);  //LCD adress: 0x27, størrelse 20 characthers x 4 rows

//rtc modul
ThreeWire myWire(7,6,8);           //dat, clk, rst
RtcDS1302<ThreeWire> Rtc(myWire);

//pins
const int potentiometer_pin = A3;
const int moisture_sensor_pin = A2;
const int buzzer_pin = 2; 
const int button_pin = 3;

//konstanter
int potentiometer_value = 0;
int moisture_threshold = 0; 
int moisture_level = 0; 
int sleep_start = 0; 
int sleep_end = 8;

//kallibreringskonstanter
float luft_kallibrering = 0;    //kallibreringsvariabel luft
float vann_kallibrering = 0;    //kallibreringsvariabel vann
volatile byte pressed = false;  //pushbutton state

//sensorverdi til prosentverdi som tar i bruk maks og min verdier målt i kallibreringen
float humidity_to_presentage(float meas, float luft_kallibrering, float vann_kallibrering){
  float a = 0;
  float b = 0;
  a = (-100)/(luft_kallibrering-vann_kallibrering);
  b = -(a*luft_kallibrering);
  return a*meas + b;
}

//funksjon som returnerer gjennomsnittlig kallibrert verdi
float calibrate () {
  float temp1 = 0;
  float temp2 = 0;
  int n = 100;
  delay(2000);
  while(pressed == false){}
  while (pressed == true) {
    for (int i = 0; i < n; i++){ 
      delay(10);
      temp1 += analogRead(moisture_sensor_pin);
      delay(10);
    }
    temp2 = temp1/n;
    delay(10);
    pressed = false;
  }
  return temp2;
}

//interrupt funksjon
void int_pressed() {
  pressed = !pressed;
  //Serial.println("Pressed:");
}

void setup() {
  Serial.begin(9600);
  //pin setup
  pinMode(buzzer_pin, OUTPUT);
  pinMode(moisture_sensor_pin, INPUT);
  pinMode(button_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(button_pin), int_pressed, RISING);

  //lcd setup
  lcd.init();
  lcd.clear();
  lcd.backlight(); //skru på backlight

  //kallibrering luft
  lcd.setCursor(0,0);
  lcd.print("Registering luft:");
  luft_kallibrering = calibrate();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Luft: ");
  lcd.print(luft_kallibrering);
  delay(3000);

  //kallibrering vann
  lcd.setCursor(0,0);
  lcd.print("Registering vann:");
  vann_kallibrering = calibrate();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Vann: ");
  lcd.print(vann_kallibrering);
  delay(3000);

  lcd.clear();

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
  moisture_level = humidity_to_presentage(analogRead(moisture_sensor_pin), luft_kallibrering, vann_kallibrering);
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
  lcd.print("Moisture: ");
  lcd.print(" ");
  if(moisture_level < 100){
    lcd.print("0");
  } 
  if(moisture_level < 10){
    lcd.print("0");
  } 
  if(moisture_level < 0){
    lcd.print("0");
  }
  if(moisture_level > 0 || moisture_level == 0){
  lcd.print(moisture_level);
  }
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



