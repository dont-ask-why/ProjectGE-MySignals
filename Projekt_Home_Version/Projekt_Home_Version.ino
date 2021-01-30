//necessary libraries for the project, mostly to connect to the screen
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <MCUFRIEND_kbv.h>
#include <stdint.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Wire.h>
#include <SPI.h>

//definition of touchscreen variables and constants
MCUFRIEND_kbv tft;
#define FOREGROUND ILI9341_BLACK
#define FOREGROUND_SELECT ILI9341_GREEN
#define TEXT ILI9341_WHITE
#define BACKGROUND ILI9341_WHITE
volatile int currentWindow = 1;
volatile int newWindow = 1;

//definition of encoder constants (for pins) and variables
#define encoderPinA 19
#define encoderPinB 18
volatile unsigned int encoderPos = 0;
unsigned int lastReportedPos = 1;
boolean A_set = false;
boolean B_set = false;

//variables for the graph
#define ecgPin A10
#define graphic_low_extrem 233
#define graphic_high_extrem 66
#define graphic_left_extrem 6
#define graphic_right_extrem 206
int flickerReduction = 0;
uint16_t graphic_x = graphic_left_extrem;
uint16_t valRead; 
uint16_t graphic_prevRead;
uint16_t prevEcgValue0 = 0;
uint16_t prevEcgValue1 = 0;

/*
 * prints the grapic - method originates from a MySignals example project
 * @param value      value which is to be added to the plot
 * @param delay_time in all example project (and ours as well) set to 0, delay can slow speed of graph
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
    tft.drawLine(graphic_x - 1, graphic_prevRead, graphic_x, value, FOREGROUND_SELECT);
  }
  //Wave refresh (barre pantalla pintando una linea)
  tft.drawLine(graphic_x + 1, graphic_high_extrem, graphic_x + 1, graphic_low_extrem, FOREGROUND);
  graphic_prevRead = value;
  graphic_x++;
  delay(delay_time);
  if (graphic_x == graphic_right_extrem){
    graphic_x = graphic_left_extrem;
  }
  SPI.end();
}

/*
 * @param scale  Chooses the current light on the traffic light. 0 for green, 1 for yellow, 2 for red.
 * @param window Chooses which windows specifications to use. 1 for left, 2 for center.
 */
void showTrafficLight(int scale, int window){
    switch (scale){
      case 0:
        tft.fillCircle(229, 98, 10, 0x4000);
        tft.fillCircle(229, 150, 10, 0x4220);
        tft.fillCircle(229, 202, 10, 0x0FE0);
        break;
      case 1:
        tft.fillCircle(229, 98, 10, 0x4000);
        tft.fillCircle(229, 150, 10, 0xEFE0);
        tft.fillCircle(229, 202, 10, 0x0220);
        break;
      case 2:
        tft.fillCircle(229, 98, 10, 0xF800);
        tft.fillCircle(229, 150, 10, 0x4220);
        tft.fillCircle(229, 202, 10, 0x0220);
        break;
      default:
        break;
    }
    tft.setTextColor(TEXT);
    switch (window){
      case 1:
        //Currently not in use
        drawString("Position", 250, 76, 2);
        drawString("is", 250, 90, 2);
        drawString("critical", 250, 104, 2);
        drawString("Position", 250, 128, 2);
        drawString("is", 250, 142, 2);
        drawString("untypical", 250, 154, 2);
        drawString("Position", 250, 180, 2);
        drawString("is", 250, 194, 2);
        drawString("normal", 250, 208, 2);
        break;
      case 2:
        //Deprecated, would now be used for ECG not EMG
        drawString("Muscle", 250, 80, 2);
        drawString("Activity", 250, 90, 2);
        drawString("is high", 250, 100, 2);
        drawString("Muscle", 250, 132, 2);
        drawString("Activity", 250, 142, 2);
        drawString("is present", 250, 152, 2);
        drawString("Muscle", 250, 184, 2);
        drawString("Activity", 250, 194, 2);
        drawString("is low", 250, 204, 2);
        break;
      default:
        break;
    }
}

//Interrupt procedure for clockwise motion on the encoder, for rest see setup
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

//Interrupt procedure for counterclockwise motion on the encoder, for rest see setup
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
 * Function to draw the "Body/nPosition/nSensor" text in the menu bar. Choose colour of text before calling function.
 */
void drawBPS(){
  drawString("Body", 16, 5, 2);
  drawString("Postion", 16, 20, 2);
  drawString("Sensor", 16, 35, 2);
}

/*
 * Function to draw the "Electro/nCardio/nGraphy" text in the menu bar. Choose colour of text before calling function.
 */
void drawECG(){
  drawString("Electro", 122, 5, 2);
  drawString("Cardio", 122, 20, 2);
  drawString("Graphy", 122, 35, 2);
}

/*
 * Function to draw the "Information/nPage" text in the menu bar. Choose colour of text before calling function.
 */
void drawIP(){
  drawString("Information", 227, 13, 2);
  drawString("Page", 228, 29, 2); 
}

/*
 * Resets the part of the screen for the sensors meaing it draws over any changes done by one of the windows on itself.
 * @param If resetFully is true, the menu bar is also reset.
 * @param Choose which window is beeing reset, possible values are 1 (left), 2 (center) and 3 (right). Other values do not change anything.
 */
void resetGUI(boolean resetFully, int window){
  if(resetFully){
    tft.fillScreen(BACKGROUND);
    tft.fillRect(6, 6, 96, 50, FOREGROUND);
    tft.fillRect(112, 6, 96, 50, FOREGROUND);
    tft.fillRect(218, 6, 96, 50, FOREGROUND);
    tft.setTextColor(TEXT);
    drawBPS();
    drawECG();
    drawIP(); 
  }
  switch(window){
    case 1:
      tft.fillRect(6, 66, 202, 168, FOREGROUND);
      tft.fillRect(218, 66, 96, 168, FOREGROUND);
      break;
    case 2:
      tft.fillRect(6, 66, 202, 168, FOREGROUND);
      tft.fillRect(218, 66, 96, 168, FOREGROUND);
      break;
    case 3:
    tft.fillRect(6, 66, 308, 168, FOREGROUND);
      break;
  }
}

/*
 * Sets the screen up for the first (left) window.
 */
void selectWindow1(){
  resetGUI(true, 1);
  tft.fillRect(6, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  drawBPS();
}

/*
 * Sets the screen up for the second (center) window.
 */
void selectWindow2(){
  resetGUI(true, 2);
  tft.fillRect(112, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  drawECG();
}

/*
 * Sets the screen up for the thrid (right) window.
 */
void selectWindow3(){
  resetGUI(true, 3);
  tft.fillRect(218, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  drawIP();
  resetGUI(false, 3);
  tft.setTextColor(TEXT);
  drawString("Information Page", 17, 71, 4);
  drawString("Created by:", 17, 90, 2);
  drawString("    Till Moehring", 17, 110, 2);
  drawString("    Ivo Opitz", 17, 125, 2);
  drawString("Version Home v0.2", 17, 150, 2);
}

/*
 * Goes one window to the left, but only if the leftmost window has not been reached
 */
void goToWindowToTheLeft(){
  if(currentWindow > 1){
    newWindow--;  
  }  
}

/*
 * Goes one window to the right, but only if the rightmost window has not been reached
 */
void goToWindowToTheRight(){
  if(currentWindow < 3){
    newWindow++;  
  }  
}

/*
 * Draws the specified text.
 * @param text The text which is to be drawn.
 * @param x    x-Position of the cursor from which the text starts.
 * @param y    y-Position of the cursor from which the text starts.
 * @param size Size of the text. Accepted values are 2 for small text and 4 for large text.
 */
void drawString(String text, int x, int y, int size){
  if(size == 2){
    tft.setFont(&FreeSans9pt7b);
    tft.setTextSize(1);
  } else if (size == 4) {
    tft.setFont(&FreeSans12pt7b);
  }
  tft.setCursor(x-8, y+15);
  tft.print(text);
}

/*
 * @deprecated
 * Loop for the body postion sensor in the first window
 */
void loopWindow1(){
  //No loop necessary (yet?), look in rareRefreshWindow1() for what happens in the left window
}


/*
 * Loop for the EMG-Graph in the second window
 */
void loopWindow2(){
  uint16_t ecgRead;
  if((digitalRead(50) == 1) || (digitalRead(51) == 1)){
    printGraphic(map(512, analogRead(A13), analogRead(A14), 232, 66), 0);
  } else {
    prevEcgValue1 = prevEcgValue0;
    prevEcgValue0 = map(analogRead(A10), analogRead(A13), analogRead(A14), 232, 66);
    ecgRead = ((prevEcgValue0 + prevEcgValue1) / 2);
    printGraphic(ecgRead,0);
  }
  delay(analogRead(A15)/10);
}

/*
 * @deprecated 
 * Loop for the information page
 */
void loopWindow3(){
  //No loop necessary, information is plotted at the start and never refreshed. Look in selectWindow3() for the code necessary.
}

//loop for any Strings that have to be refreshed for the body position window in the first window
void rareRefreshWindow1(){
  resetGUI(false, 1);
  tft.setTextColor(TEXT);
  drawString(getBodyPositionText(), 15, 66, 2);
}

//loop for any Strings that have to be refreshed for the body position sensor in the second window
void rareRefreshWindow2(){
  //@TODO - traffic light has to be implemented
}

/*
 * rare loop for the information page
 * @deprecated
 */
void rareRefreshWindow3(){
  //No code necessary
}

/*
 * setup() is an original Arduino method required to run the program.
 * Only called once at the start of the program.
 */
void setup() {
  //setup rotary encoder, defining both pins A and B as pullup - if a connection between either A or B has been made, it's potential is pulled to GND
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(50, INPUT);
  pinMode(60, INPUT);
  //basic interrupt attachments which call their associated procedures, both register a change in signal value as rise and fall can both appear while turning in same direction
  attachInterrupt(digitalPinToInterrupt(18), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(19), doEncoderB, CHANGE);
  Wire.begin();
  tft.begin(tft.readID());
  tft.setRotation(1);
  resetGUI(true, 1);
  selectWindow1();
}

/*
 * loop() is an original method from Arduino required to run the program.
 * Called periodically to apply any changes in the selected window by calling the corresponding loop function.
 */
void loop() {
  if(newWindow != currentWindow){
    flickerReduction = 0;
    graphic_x = graphic_left_extrem;
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
  if(flickerReduction == 199){
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
 * Create traffic light connection for ECG
 * Create global traffic light connection
 * Implement alternative to Body Position Sensor
 */
