///              MP3 PLAYER PROJECT
/// http://educ8s.tv/arduino-mp3-player/
//////////////////////////////////////////

#include <fix_fft.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define LCHAN 3
#define RCHAN 1
const int channels = 1;
const int xres = 16;
const int yres = 8;
const int gain = 2;
//int decayrate = 2; // larger number is longer decay
int decaytest = 1;
char im[64], data[64];
char Rim[64], Rdata[64];
char data_avgs[32];
float peaks[32];
int i = 0,val,Rval;
int x = 0, y=0, z=0;

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

byte custom_0[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F};
byte custom_1[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x1F};
byte custom_2[] = {0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F};
byte custom_3[] = {0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F};
byte custom_4[] = {0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F};
byte custom_5[] = {0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
byte custom_6[] = {0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
byte custom_7[] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};

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
lcd.createChar(1, custom_0);
lcd.createChar(2, custom_1);
lcd.createChar(3, custom_2);
lcd.createChar(4, custom_3);
lcd.createChar(5, custom_4);
lcd.createChar(6, custom_5);
lcd.createChar(7, custom_6);
lcd.createChar(8, custom_7);
//lcd.home ();
//lcd.print("Playing         ");  
lcd.home();
//lcd.write((byte)0);  
//lcd.setCursor (1,0);
//lcd.write((byte)1);  
//lcd.setCursor (2,0);
//lcd.write((byte)2);  
//lcd.setCursor (3,0);
//lcd.write((byte)3);  
//lcd.setCursor (4,0);
//lcd.write((byte)4);  
//lcd.setCursor (5,0);
//lcd.write((byte)5);  
//lcd.setCursor (6,0);
//lcd.write((byte)6);  
//lcd.setCursor (7,0);
//lcd.write((byte)7);  
}
#define TOLERANCE 1

int oldVal = 0;

void loop () { 

  for (i=0; i < 64; i++){    
    val = ((analogRead(LCHAN) / 3 ) - 128);  // chose how to interpret the data from analog in
    if (oldVal<=10){val=val*4.5;}
    if (16<=oldVal<10){val=val*3;}
    if (20<oldVal<16){val=val*2;}
    if (oldVal>=20){val=val*0.8;}
    
    data[i] = val;                                      
    im[i] = 0;   
    if (channels ==2){
      Rval = ((analogRead(RCHAN) / 4 ) - 128);  // chose how to interpret the data from analog in
      Rdata[i] = Rval;                                      
      Rim[i] = 0;   
    }
  };

  fix_fft(data,im,6,0); // Send the data through fft
  if (channels == 2){
    fix_fft(Rdata,Rim,6,0); // Send the data through fft
  }

  // get the absolute value of the values in the array, so we're only dealing with positive numbers
  for (i=0; i< 32 ;i++){  
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
  }
  if (channels ==2){
    for (i=16; i< 32 ;i++){  
      data[i] = sqrt(Rdata[i-16] * Rdata[i-16] + Rim[i-16] * Rim[i-16]); 
    }
  }

  // todo: average as many or as little dynamically based on yres
  for (i=0; i<32; i++) {
    data_avgs[i] = (data[i]);// + data[i*2+1]);// + data[i*3 + 2]);// + data[i*4 + 3]);  // add 3 samples to be averaged, use 4 when yres < 16
    data_avgs[i] = constrain(data_avgs[i],0,9-gain);  //data samples * range (0-9) = 9
    data_avgs[i] = map(data_avgs[i], 0, 9-gain, 0, yres);        // remap averaged values
  }



  decay(1);
  mono();


  
//lcdBacklightMills = millis();
//if(lcdBacklightMills - lastLcdBacklight > 5000) {
//  lcd.setBacklight(LOW);
//}

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

mySerial.write(Command_line[k]);
}
}




void decay(int decayrate){
  //// reduce the values of the last peaks by 1 
  if (decaytest == decayrate){
    for (x=0; x < 32; x++) {
      peaks[x] = peaks[x] - 2;  // subtract 1 from each column peaks
      decaytest = 0;
      
    }
  }
  decaytest++;

}

void mono(){ 
  for (x=0; x < xres; x++) {  // repeat for each column of the display horizontal resolution
    y = data_avgs[x];  // get current column value 
        z= peaks[x];
    if (y > z){
      peaks[x]=y;
    }
    y= peaks[x]*2; 

    if (y <= 8){            
      lcd.setCursor(x-1,0); // clear first row
      lcd.print(" ");
      lcd.setCursor(x-1,1); // draw second row
      if (y == 0){
        lcd.print(" "); // save a glyph
      }
      else {
        lcd.write(y);
      }
    }
    else{
      lcd.setCursor(x-1,0);  // draw first row
      if (y == 9){
        lcd.write(" ");  
      }
      else {
        lcd.write(y-8);  // same chars 1-8 as 9-16
      }
      lcd.setCursor(x-1,1);
      lcd.write(8);  
    } // end display
  }  // end xres
}







