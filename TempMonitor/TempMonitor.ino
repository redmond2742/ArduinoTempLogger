/*
-----This code is in the public domain free to use and remix----

TempMonitor arduino code performs the following
-uses a thermisitor to determine the tempeture using voltage divider and Steinhart–Hart equation equation
-Logs date, time and tempeture to serial monitor
-logs minute and seconds and temp to SD card
-Displays first two digits of temp to 7 segment LCD screen

Refrence Sources:
http://www.hacktronics.com/Tutorials/arduino-and-7-segment-led.html
http://www.thecoderscorner.com/electronics/microcontrollers/driving-displays/66-checking-polarity-of-7segment-display
http://computers.tutsplus.com/tutorials/how-to-read-temperatures-with-arduino--mac-53714
http://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation

Fritzing Diagram: http://i.imgur.com/twUzv59.png

*/


#include <SPI.h>
#include <math.h>
#include <SD.h>
#include <Time.h>

//Define the SD file
File myFile;

//Fuction to display the date and time for serial monitor logging
void digitalClockDisplay(){
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print("\t");
   
}

void digitalClockSDWrite(){
  // digital clock display of the time
  myFile.print(" ");
  myFile.print(minute());
  myFile.print(":");
  myFile.print(second());
  myFile.print(" ");
  myFile.print("\t");
   
}

//fuction to display minutes and seconds nicely
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

//Array for 7 segment LED
byte seven_seg_digits[10][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                                           { 0,1,1,0,0,0,0 },  // = 1
                                                           { 1,1,0,1,1,0,1 },  // = 2
                                                           { 1,1,1,1,0,0,1 },  // = 3
                                                           { 0,1,1,0,0,1,1 },  // = 4
                                                           { 1,0,1,1,0,1,1 },  // = 5
                                                           { 1,0,1,1,1,1,1 },  // = 6
                                                           { 1,1,1,0,0,0,0 },  // = 7
                                                           { 1,1,1,1,1,1,1 },  // = 8
                                                           { 1,1,1,0,0,1,1 }   // = 9
                                                           };

void setup() {
  //Make digital pins 2-10 ready for use
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);   
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10, OUTPUT);
 
  //open serial port at a baud rate of 115200
  Serial.begin(115200);
  //set the date on the clock since no external time clock 
  setTime(0,0,0,15,5,2015);
  //initialize SD writing and open text file - using Eithernet Shield with micro SD card port
  Serial.print("Initializing SD card....");
  digitalWrite(10, HIGH);
  delay(10);
   if(!SD.begin(4)){
    Serial.println("initialization failed!");
    return;
   }
    Serial.println("initialization done.");
    myFile = SD.open("tempLog.txt", FILE_WRITE);
    
    if(myFile){
  Serial.print("Writing to tempLog.txt...");
  myFile.println("testing data log");
  //Close the file writer
    myFile.close();
}else{
  Serial.println("error opening text file");
}
}
//fuction for thermister resitance value to temp value in F degrees
double Thermister(int RawADC) {
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1/ (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp))* Temp);
  Temp = Temp - 273.15;
  Temp = (Temp * 9.0)/5.0 +32.0;
  return Temp;
}
//function for getting a single digit to display on 7 segment LCD
int getDigit(long number, int place)
{
    return ((number / long(pow(10, place - 1))) % 10);
}
//Fuction to write number to 7 segment LCD
void sevenSegWrite1(byte digit) {
  byte pin = 2;
  for (byte segCount = 0; segCount < 7; ++segCount) {
    digitalWrite(pin, seven_seg_digits[digit][segCount]);
    ++pin;
  }
}
//Fuction to write dot on 7 segment LCD.
void writeDot(byte dot) {
  digitalWrite(9, dot);
}

void loop() {
  //Turn dot off on 7 segment LCD
  writeDot(0);
  //Var declaration
  int val;
  double temp=0;
  double avgTemp;
  double finalTemp;
  
  //For loop to average samples from analog input on thermisistor
  for (int x = 0; x < 10; x++){ // run through loop 10x
   val=analogRead(4);
   temp=temp+val; // add samples together
   delay (10); // let ADC settle before next sample
}

 avgTemp = temp / 10;
 finalTemp = Thermister(avgTemp);
 
 //serial print clock and temp
  digitalClockDisplay();
  Serial.println(finalTemp);
  
  //display LED - 1 digit to each, decimal dot shown on second digit
  int num1=getDigit(finalTemp,2);
  int num2=getDigit(finalTemp,1);
  sevenSegWrite1(num1);
  delay(500);
  sevenSegWrite1(num2);
  writeDot(1);
  
  //Open the file writer and write min, seconds and temp value
  myFile = SD.open("tempLog.txt", FILE_WRITE);  
  if(myFile){
    digitalClockSDWrite();
    myFile.print(" temp:" );
    myFile.print("\t");
    myFile.print(finalTemp); 
    myFile.close(); 
 }
  delay(1000);
}
