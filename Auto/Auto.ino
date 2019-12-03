#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
//Important pins
#define RIGHT_LIGHT 22
#define LEFT_LIGHT 23
#define TRIGGER_PIN 24
#define ECHO_PIN 25
#define NOISE_ALARM 46
#define SENSOR_PIN 27
#define LIGHTS_PIN 28
#define TANK_LOW 0
#define TANK_SL 1
#define TANK_SH 2
#define TANK_HIGH 3
#define SD_ChipSelectPin 53
//Constants
int lights = 0;
bool state;
bool activated = true;
bool Pause = false;

LiquidCrystal_I2C lcd(0x27,16,2); //Display preparation
TMRpcm Audio; //TMRpcm object

//Keypad preparation 
const byte Rows = 4;
const byte Col = 4;
char keys[Rows][Col] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinRows[Rows] = {6,7,8,9};
byte pinCol[Col] = {10,11,12,13};
Keypad keyP = Keypad(makeKeymap(keys), pinRows, pinCol, Rows,Col);
char key;

//Custom characters
byte part1[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte part2[] = {
  B00100,
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B11111,
  B11111
};
byte part3[] = {
  B00001,
  B00001,
  B00010,
  B00010,
  B00010,
  B00011,
  B00111,
  B00111
};
byte part4[] = {
  B00110,
  B01110,
  B01110,
  B01110,
  B11110,
  B11111,
  B11111,
  B11111
};
byte part5[] = {
  B10000,
  B10000,
  B01000,
  B01000,
  B01000,
  B11000,
  B11100,
  B11100
};
byte part6[] = {
  B01100,
  B01100,
  B01110,
  B01110,
  B01110,
  B11111,
  B11111,
  B11111
};
byte part7[] = {
  B11111,
  B11111,
  B11111,
  B01110,
  B01110,
  B01110,
  B00100,
  B00100
};
byte part8[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};
//End

void setup() {
  //SD communication
  if(!SD.begin(SD_ChipSelectPin)) {
    tone(NOISE_ALARM, 200);
    delay(500);
    noTone(NOISE_ALARM);
  }
  //Audio
  Audio.speakerPin = 46;
  Audio.quality(1);
  Audio.setVolume(5);
  //Fuel Tank
  pinMode(TANK_LOW,INPUT);
  pinMode(TANK_SL,INPUT);
  pinMode(TANK_SH,INPUT);
  pinMode(TANK_HIGH,INPUT);
  //Lights
  pinMode(RIGHT_LIGHT, OUTPUT);
  pinMode(LEFT_LIGHT, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LIGHTS_PIN, OUTPUT);
  //Propximity sensor
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(NOISE_ALARM, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.print("Initing car...");
  Audio.play("engineOn.wav");
  delay(2000);
}

void loop() {
  state = true;
  lcd.clear();
  lcd.display();
  while(state){
    lcd.setCursor(0,0);
    lcd.print("Option:");
    key = keyP.getKey();
    if(key){
      switch (key){
        case 'A':
          flashLight();
          state = false;
        break;
        case 'B':
          crashMode();
          state = false;
        break;
        case 'C':
          musicPlayer();
          state = false;
        break;
        case 'D':
          engine();
        break;
        default:
          lcd.setCursor(0,1);
          lcd.print("Invalid Option");
          delay(700);
          lcd.clear();
      }
    }
    internalLights();
  }
}

void flashLight(){
  bool internal_state = true;
  lcd.clear();
  lcd.print("A)Left");
  lcd.setCursor(0,1);
  lcd.print("B)Right");
  lcd.setCursor(8,0);
  lcd.print("C)Off");
  lcd.setCursor(8,1);
  lcd.print("D)Back");
  while(internal_state){
  key = keyP.getKey();
    if(key){
      switch (key){
        case 'A':
          lcd.clear();
          lcd.print("Left on...");
          delay(1000);
          digitalWrite(LEFT_LIGHT, HIGH);
          internal_state = false;
        break;
        case 'B':
          lcd.clear();
          lcd.print("Right on...");
          delay(1000);
          digitalWrite(RIGHT_LIGHT, HIGH);
          internal_state = false;
        break;
        case 'C':
          lcd.clear();
          lcd.print("Turning off...");
          delay(1000);
          digitalWrite(RIGHT_LIGHT, LOW);
          digitalWrite(LEFT_LIGHT, LOW);
          internal_state = false;
        break;
        case 'D':
          lcd.clear();
          return;
        break;
        default:
          lcd.clear();
          lcd.print("Invalid option");
          lcd.setCursor(0,1);
          lcd.print(key);
          delay(1000);
          lcd.clear();
          lcd.print("A)Left");
          lcd.setCursor(0,1);
          lcd.print("B)Right");
          lcd.setCursor(8,0);
          lcd.print("C)Off");
          lcd.setCursor(8,1);
          lcd.print("D)Back");
      }
   }
 }
 lcd.clear();
}

void crashMode(){
  bool internal_state = true;
  lcd.clear();
  lcd.print("Crash mode!");
  lcd.setCursor(0,1);
  lcd.print("Press A to return");
  char keyReturn;
  int duration, distance;
  while(internal_state){
    digitalWrite(TRIGGER_PIN, HIGH);
    delay(1);
    digitalWrite(TRIGGER_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration/58.2;
    if(distance<8){
      lcd.clear();
      printAlert();
      tone(NOISE_ALARM, 200);
    }else{
      noTone(NOISE_ALARM);
      lcd.clear();
      lcd.print("Crash mode!");
      lcd.setCursor(0,1);
      lcd.print("A)Return");
    }
    delay(100);
    keyReturn = keyP.getKey();
    if(keyReturn=='A'){
      internal_state = false;
    }
  }
  lcd.clear();
  digitalWrite(TRIGGER_PIN, LOW);
}

void printAlert(){
  lcd.home();
  //Creating custom characters
  lcd.createChar(1, part1);
  lcd.createChar(2, part2);
  lcd.createChar(3, part3);
  lcd.createChar(4, part4);
  lcd.createChar(5, part5);
  lcd.createChar(6, part6);
  lcd.createChar(7, part7);
  lcd.createChar(8, part8);
  //End
  lcd.setCursor(5,0);
  lcd.write(1);
  lcd.setCursor(5,1);
  lcd.write(2);
  lcd.setCursor(7,0);
  lcd.write(3);
  lcd.setCursor(7,1);
  lcd.write(4);
  lcd.setCursor(8,0);
  lcd.write(5);
  lcd.setCursor(8,1);
  lcd.write(6);
  lcd.setCursor(10,0);
  lcd.write(7);
  lcd.setCursor(10,1);
  lcd.write(8);
}

void internalLights(){
  if(digitalRead(SENSOR_PIN)==LOW && lights == 0){
    lcd.clear();
    lcd.print("Turning lights");
    lcd.setCursor(0,1);
    lcd.print("on...");
    delay(1000);
    digitalWrite(LIGHTS_PIN, HIGH);
    lights = 1;
    lcd.clear();
  }
  if(digitalRead(SENSOR_PIN)==LOW && lights == 1){
    lcd.clear();
    lcd.print("Turning lights");
    lcd.setCursor(0,1);
    lcd.print("off...");
    delay(1000);
    digitalWrite(LIGHTS_PIN, LOW);
    lights = 0;
    lcd.clear();
  }
}

void engine(){
  if(activated == true){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Bye...");
      delay(1000);
      lcd.noDisplay();
      lcd.noBacklight();
      activated = false;
    } else if (activated == false){
      activated = true;
      lcd.display();
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print("Initing car...");
      Audio.play("engineOn.wav");
      delay(1000);
      lcd.clear();
      lcd.print("Option:");
    }
}

void musicPlayer(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Song:");
  lcd.setCursor(0,1);
  lcd.print("A)Pause B)Back");
  while(1){
    char selectKey = keyP.getKey();
    if (key){
      switch (selectKey){
        case '1':
          lcd.setCursor(0,0);
          lcd.print("Song:Take on me");
          Audio.play("takeOnMe.wav");
        break;
        case '2':
          lcd.setCursor(0,0);
          lcd.print("Song: Welcome to the jungle");
          Audio.play("welcome.wav");
        break;
        case '3':
          lcd.setCursor(0,0);
          lcd.print("Song: Sweet Home Alabama");
          Audio.play("sweet.wav");
        break;
        case 'A':
          Audio.pause();
        break;
        case 'B':
          Audio.disable();
          return;
        break;
      }
    }
    lcd.scrollDisplayLeft();
    delay(400);
  }
}
