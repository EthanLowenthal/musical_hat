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
delay(1000);
Serial.begin(1200); 
Serial.println("----Volume----");
playFirst();
isPlaying = true;
lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
lcd.setBacklight(HIGH);
lcd.begin (16,2); //  <<----- My LCD was 16x2

lcd.home ();                   // go home
lcd.print("Playing         ");  
lcd.setCursor ( 0, 1 );        // go to the next line
lcd.print ("Playing Album ");  
lcd.setCursor ( 14, 1 );
lcd.print (album); 
execute_CMD(0x4D,0,0);  
}
#define TOLERANCE 1

int oldVal = 0;



void loop () { 

int val = analogRead(A0)/33.5;
delay(15);
int diff = abs(val - oldVal);
if(diff > TOLERANCE)
{
 oldVal = val; 
 setVolume(val);
 Serial.println(val);
 lcd.setCursor ( 0, 1 );
 lcd.print ("Volume set to "); 
 lcd.setCursor ( 14, 1 );  
 lcd.print("  ");
 lcd.setCursor ( 14, 1 );  
 lcd.print(val);
 delay(1000);
 lcd.setCursor ( 0, 1 );
 lcd.print ("                ");
//lcd.setCursor(0,1);
//lcd.print ("Playing Album ");  
//lcd.setCursor ( 14, 1 );
//lcd.print (album);      

}   
 if (digitalRead(buttonPause) == ACTIVATED)
  {
    if(isPlaying)
    {
      pause();
      isPlaying = false;
      lcd.setCursor(0,0);
      lcd.print("Paused          ");
    }else
    {
      isPlaying = true;
      play();
      lcd.setCursor(0,0);
      lcd.print("Playing         ");
    }
  }


 if (digitalRead(buttonNext) == ACTIVATED)
  {
    if(isPlaying)
    {
      playNext();
      lcd.setCursor(0,0);
      lcd.print("Next            ");
      delay(1000);
      lcd.setCursor(0,0);
      lcd.print("Playing         ");
    }
  }

   if (digitalRead(buttonPrevious) == ACTIVATED)
  {
    if(isPlaying)
    {
      playPrevious();
      lcd.setCursor(0,0);
      lcd.print("Previous        ");
      delay(1000);
      lcd.setCursor(0,0);
      lcd.print("Playing         ");
    }
  }
  if (digitalRead(buttonSelect) == ACTIVATED) 
  {
    Serial.println("hi");
    choose_song();
  }
}


void playFirst()
{
  execute_CMD(0x3F, 0, 0);
  delay(500);
//  setVolume(15);
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
    lcd.setCursor ( 0, 0 );  
    lcd.print("Choose Song     ");
    lcd.setCursor ( 0, 1 );
    lcd.print("Play song       ");
    lcd.setCursor ( 10, 1 );
    lcd.print(song);
    delay(250);
    if (digitalRead(buttonPrevious) == ACTIVATED){if (song > 0){
      song --;
      lcd.setCursor ( 0, 1 );
      lcd.print("Play song       ");
      lcd.setCursor ( 10, 1 );
      lcd.print(song);
      delay(250);
      }} 
    if (digitalRead(buttonNext) == ACTIVATED){if (song < 255){
      song ++;
      lcd.setCursor ( 0, 1 );
      lcd.print("Play song       ");
      lcd.setCursor ( 10, 1 );
      lcd.print(song);
      delay(250);
      }}
    if (digitalRead(buttonPause) == ACTIVATED){
      execute_CMD(0x03, 0, song);
      lcd.setCursor ( 0, 0 );  
      lcd.print("Playing         ");
      lcd.setCursor ( 0, 1 );  
      lcd.print("Playing song    ");
      lcd.setCursor ( 13, 1 );
      lcd.print(song);
      delay(1000);
      lcd.setCursor ( 0, 1 );
      lcd.print ("                "); 
      break;
      }
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
mySerial.write( Command_line[k]);
}
}
