///              MP3 PLAYER PROJECT
/// http://educ8s.tv/arduino-mp3-player/
//////////////////////////////////////////

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x3F // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

int n = 1;

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

#include "SoftwareSerial.h"

SoftwareSerial mySerial(10, 11);
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

# define ACTIVATED LOW
int lastLcdBacklight = millis();
int lcdBacklightMills = millis();
int album = 1;
int song = 1;
int buttonNext = 2;
int buttonPause = 3;
int buttonPrevious = 4;
int buttonSelect = 5;
boolean isPlaying = false;



void setup () {

pinMode(buttonPause, INPUT);
digitalWrite(buttonPause,HIGH);
pinMode(buttonNext, INPUT);
digitalWrite(buttonNext,HIGH);
pinMode(buttonPrevious, INPUT);
digitalWrite(buttonPrevious,HIGH);
pinMode(buttonSelect, INPUT);
digitalWrite(buttonSelect,HIGH);

mySerial.begin (9600);
Serial.begin (4800);
delay(1000);
playFirst();
isPlaying = true;
lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
lcd.setBacklight(HIGH);
lcd.begin (16,2); //  My LCD was 16x2
lcd.home ();
lcd.print("Playing         ");    
}
#define TOLERANCE 1

int oldVal = 0;



void loop () { 
lcdBacklightMills = millis();
if(lcdBacklightMills - lastLcdBacklight > 5000) {
  lcd.setBacklight(LOW);
}

int val = analogRead(A0)/33.5;
delay(15);
int diff = abs(val - oldVal);
if(diff > TOLERANCE)
{
 oldVal = val; 
 setVolume(val);
 displayText("Volume set to ",0,0,1,true,false);
 displayText(String(val),1000,14,1,true,true);   
}   
 if (digitalRead(buttonPause) == ACTIVATED)
  {
    if(isPlaying)
    {
      pause();
      isPlaying = false;
      displayText("Paused",0,0,0,true,false);
    }else
    {
      isPlaying = true;
      play();
      displayText("Playing",0,0,0,true,false);
    }
  }


 if (digitalRead(buttonNext) == ACTIVATED)
  {
    if(isPlaying)
    {
      playNext();
      displayText("Next",1000,0,0,true,false);
      displayText("Playing",0,0,0,true,false);
    }
  }

   if (digitalRead(buttonPrevious) == ACTIVATED)
  {
    if(isPlaying)
    {
      playPrevious();
      displayText("Previous",1000,0,0,true,false);
      displayText("Playing",0,0,0,true,false);
    }
  }
  if (digitalRead(buttonSelect) == ACTIVATED) 
  {
    choose_song();
  }
}


void playFirst()
{
  execute_CMD(0x3F, 0, 0);
  delay(500);
  setVolume(15);
  delay(500);
  execute_CMD(0x11,0,1); 
  delay(500);
}

void pause()
{
  execute_CMD(0x0E,0,0);
  delay(500);
}

void play()
{
  execute_CMD(0x0D,0,1); 
  delay(500);
}

void playNext()
{
  execute_CMD(0x01,0,1);
  delay(500);
}

void playPrevious()
{
  execute_CMD(0x02,0,1);
  delay(500);
}

void choose_song(){
  while (true) {
    displayText("Choose Song",0,0,0,true,false);
    displayText("Play song",0,0,1,true,false);
    displayText(String(song),0,10,1,false,false);
    delay(250);
    if (digitalRead(buttonPrevious) == ACTIVATED){if (song > 0){
      song --;
      displayText("Play song",0,0,1,true,false);
      displayText(String(song),0,10,1,false,false);
      delay(250);
      }} 
    if (digitalRead(buttonNext) == ACTIVATED){if (song < 255){
      song ++;
      displayText("Play song",0,0,1,true,false);
      displayText(String(song),0,10,1,false,false);
      delay(250);
      }}
    if (digitalRead(buttonPause) == ACTIVATED){
      execute_CMD(0x03, 0, song);
      displayText("Playing",0,0,0,true,false);
      displayText("Playing song",0,0,1,true,false);
      displayText(String(song),1000,13,1,true,true);
      break;
      }
    }
}

void displayText(String text, int interval, int line, int row, boolean fillSpace, boolean wipe) {
  lcd.setBacklight(HIGH);
  lastLcdBacklight = millis();
  lcd.setCursor (line,row);
  if (fillSpace) {
    for (int i=text.length(); i < 16; i++){text += " ";}
    lcd.print(text);
   }
  else{lcd.print(text);}
  delay(interval);
  if (wipe){
  lcd.setCursor(0,line);
  lcd.print("                ");
  }
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(250);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
// Calculate the checksum (2 bytes)
word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
// Build the command line
byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
//Send the command line to the module
for (byte k=0; k<10; k++)
{
//Serial.println(Command_line[k]);
//Serial.println("-------");
mySerial.write(Command_line[k]);
}
}
