#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <UTouch.h>
#include <MySignals.h>
#include <MySignals_BLE.h>
#include <Wire.h>
#include <SPI.h>

//definition of touchscreen variables and constants
Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(TFT_CS, TFT_DC);
//Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(53, 49);
UTouch  myTouch(TOUCH_CLK,TOUCH_CS,TOUCH_DIN,TOUCH_DOUT,TOUCH_IRQ);
#define FOREGROUND ILI9341_BLACK
#define FOREGROUND_SELECT ILI9341_GREEN
#define TEXT ILI9341_WHITE
#define BACKGROUND ILI9341_WHITE
volatile int currentWindow = 1;
volatile int newWindow = 1;

//definition of encoder constants (for pins) and variables
#define encoderPinA 18
#define encoderPinB 19
volatile unsigned int encoderPos = 0;
unsigned int lastReportedPos = 1;
boolean A_set = false;
boolean B_set = false;

//variables for the graph
#define graphic_low_extrem 234
#define graphic_high_extrem 66
#define graphic_left_extrem 6
#define graphic_right_extrem 208
int flickerReduction = 0;
uint16_t graphic_x = graphic_left_extrem;
uint16_t valRead; 
uint16_t graphic_prevRead;

/*
 * prints the grapic - method originates from a MySignals example project
 */
void printGraphic(uint16_t value, uint8_t delay_time)
{
  if (value < graphic_high_extrem){
    value = graphic_high_extrem;
  }
  if (value > graphic_low_extrem){
    value = graphic_low_extrem;
  }
  //Pinta la linea solo a partir de que ya exista al menos 1 valor
  if (graphic_x > graphic_left_extrem + 1){
    tft.drawLine(graphic_x - 1, graphic_prevRead, graphic_x, value, ILI9341_RED);
  }
  //Wave refresh (barre pantalla pintando una linea)
  tft.drawLine(graphic_x + 1, graphic_high_extrem, graphic_x + 1, graphic_low_extrem, ILI9341_WHITE);
  graphic_prevRead = value;
  graphic_x++;
  delay(delay_time);
  if (graphic_x == graphic_right_extrem){
    graphic_x = graphic_left_extrem;
    flickerReduction = 0;
  }
  SPI.end();
}

//Interrupt method for clockwise motion on the encoder
void doEncoderA() {
  delay(1);
  
  if( digitalRead(encoderPinA) != A_set ) {
    A_set = !A_set;
    if ( A_set && !B_set ){
      encoderPos += 1;
      if (lastReportedPos != encoderPos){
        goToWindowToTheRight();
      }
    }
  }
}

//Interrupt method for counterclockwise motion on the encoder
void doEncoderB() {
  delay(1);
  
  if( digitalRead(encoderPinB) != B_set ){
    B_set = !B_set;
    if( B_set && !A_set ){
      encoderPos -= 1;
      if (lastReportedPos != encoderPos){
        goToWindowToTheLeft();
      }
    }
  }
}

/*
 * Resets the part of the screen for the sensors (draws over any changes by one of the windows itself).
 * @param If resetFully is true, the menu bar is also reset.
 * @param Choose which window is beeing reset, possible values are 1 2 and 3
 */
void resetGUI(boolean resetFully, int window){
  if(resetFully){
    tft.fillScreen(BACKGROUND);
    tft.fillRect(6, 6, 96, 50, FOREGROUND);
    tft.fillRect(112, 6, 96, 50, FOREGROUND);
    tft.fillRect(218, 6, 96, 50, FOREGROUND);
    tft.setTextColor(TEXT);
    tft.drawString("Pulseoxi", 16, 14, 2);
    tft.drawString("Meter", 16, 28, 2);
    tft.drawString("SECOND", 122, 14, 2);
    tft.drawString("WINDOW", 122, 28, 2);
    tft.drawString("THIRD", 228, 14, 2);
    tft.drawString("WINDOW", 228, 28, 2);
  }
  switch(window){
    case 1:
      tft.fillRect(6, 66, 151, 168, FOREGROUND);
      tft.fillRect(163, 66, 151, 168, FOREGROUND);
      break;
    case 2:
      tft.fillRect(6, 66, 202, 168, FOREGROUND);
      tft.fillRect(218, 66, 96, 168, FOREGROUND);
      break;
    case 3:
      break;
  }
}

//sets the screen up for the first (left) window
void selectWindow1(){
  resetGUI(true, 1);
  tft.fillRect(6, 6, 96, 50, FOREGROUND_SELECT);
  tft.drawString("Pulseoxi", 16, 14, 2);
  tft.drawString("Meter", 16, 28, 2);
}

//sets the screen up for the second (center) window
void selectWindow2(){
  resetGUI(true, 2);
  tft.fillRect(112, 6, 96, 50, FOREGROUND_SELECT);
  tft.drawString("SECOND", 122, 14, 2);
  tft.drawString("WINDOW", 122, 28, 2);
}

//sets the screen up for the thrid (right) window
void selectWindow3(){
  resetGUI(true, 3);
  tft.fillRect(218, 6, 96, 50, FOREGROUND_SELECT);
  tft.drawString("THIRD", 228, 14, 2);
  tft.drawString("WINDOW", 228, 28, 2);  
}

//goes one window to the left, but only if the leftmost window has not been reached
void goToWindowToTheLeft(){
  if(currentWindow > 1){
    newWindow--;  
  }  
}

//goes one window to the right, but only if the rightmost window has not been reached
void goToWindowToTheRight(){
  if(currentWindow < 3){
    newWindow++;  
  }  
}

//loop for the pulseoximeter in the first window
void loopWindow1(){
  //currently no loop required
}

//loop for the sensor in the second window
void loopWindow2(){
  //TODO
}

//loop for the third sensor (sensor or information page?)
void loopWindow3(){
  //TODO
}

//loop for any Strings that have to be refreshed for the pulseoximeter in the first window
void rareRefreshWindow1(){
  uint16_t pulseVal = -1;
  uint16_t satVal = -1;
  
  resetGUI(false, 1);
  tft.drawString("BPM:", 16, 76, 4);
  tft.drawString("O2:", 173, 76, 4);
  tft.drawNumber((int)pulseVal, 26, 96, 4);
  tft.drawNumber((int)satVal, 183, 96, 4);
  Serial.print("Heart rate:");
  Serial.print(pulseVal);
  Serial.print("bpm / SpO2:");
  Serial.print(satVal);
  Serial.println("%");
}

//loop for any Strings that have to be refreshed for the sensor in the second window
void rareRefreshWindow2(){
  //TODO
}

//loop for any Strings that have to be refreshed for the sensor in the third window
void rareRefreshWindow3(){
  //TODO
}

//setup for the code, original Arduino method required to run the program
void setup() {
  //setup encoder
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(18), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(19), doEncoderB, CHANGE);
  
  //setup tft (expanderState is initialized with B10000001)
  bitSet(MySignals.expanderState, EXP_TOUCH_CS);
  MySignals.expanderWrite(MySignals.expanderState);
  Wire.begin();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  tft.init();
  tft.setRotation(3);
  resetGUI(true, 1);
  selectWindow1();
}

/*
 * loop, original method from Arduino required to run the program.
 * Checks if the window has to be changed and runs the loop functions for the 3 windows according to which is currently selected
 */
void loop() {
  if(newWindow != currentWindow){
    switch (newWindow){
      case 1:
        currentWindow = 1;
        selectWindow1();
        break;
      case 2:
        currentWindow = 2;
        selectWindow2();
        break;
      case 3:
        currentWindow = 3;
        selectWindow3();
        break;  
      default:
        break;
    }
  }
  switch (currentWindow){
    case 1:
      loopWindow1();
      break;
    case 2:
      loopWindow2();
      break;
    case 3:
      loopWindow3();
      break;  
    default:
      break;
  }
  if(flickerReduction == graphic_right_extrem-graphic_left_extrem){
    flickerReduction = 0;
    switch (currentWindow){
      case 1:
        rareRefreshWindow1();
        break;
      case 2:
        rareRefreshWindow2();
        break;
      case 3:
        rareRefreshWindow3();
        break;  
      default:
        break;
    }
  } else {
    flickerReduction++;  
  }
  delay(1);
}

/*
 * TODO
 * Create the loop function for all three windows.
 * Import the graph function from MySignals.
 * Create link to the sensors and choose which to read (temperature, pulseoximeter, ...)
 */
