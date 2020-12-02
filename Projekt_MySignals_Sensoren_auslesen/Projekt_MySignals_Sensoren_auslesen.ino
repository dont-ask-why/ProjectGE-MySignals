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
#define graphic_low_extrem 233
#define graphic_high_extrem 66
#define graphic_left_extrem 6
#define graphic_right_extrem 206
int flickerReduction = 0;
uint16_t graphic_x = graphic_left_extrem;
uint16_t valRead; 
uint16_t graphic_prevRead;

char* getBodyPositionText(){
  uint8_t position = MySignals.getBodyPosition(); 
  switch (position){
    case 0:
      return "Test";
      break;
    case 1:
      return "Auf dem Bauch liegend";
      break;
    case 5:
      return "Stehend";
      break;
    case 2:
      return "Auf der linken Seite";
      break;
    case 3:
      return "Auf der rechten Seite";
      break;
    case 4:
      return "Auf dem Ruecken liegend";
      break;
    case 6:
      return "Unbekannte Position";
      break;
  }
}

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
    tft.drawString("Body Postion", 16, 14, 2);
    tft.drawString("Sensor", 16, 28, 2);
    tft.drawString("Electromyo-", 122, 14, 2);
    tft.drawString("graphy", 122, 28, 2);
    tft.drawString("Information", 228, 14, 2);
    tft.drawString("Page", 228, 28, 2);  
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

//sets the screen up for the first (left) window
void selectWindow1(){
  resetGUI(true, 1);
  tft.fillRect(6, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  tft.drawString("Body Postion", 16, 14, 2);
  tft.drawString("Sensor", 16, 28, 2);
}

//sets the screen up for the second (center) window
void selectWindow2(){
  resetGUI(true, 2);
  tft.fillRect(112, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  tft.drawString("Electromyo-", 122, 14, 2);
  tft.drawString("graphy", 122, 28, 2);
}

//sets the screen up for the thrid (right) window
void selectWindow3(){
  resetGUI(true, 3);
  tft.fillRect(218, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  tft.drawString("Information", 228, 14, 2);
  tft.drawString("Page", 228, 28, 2);  
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

//loop for the body postion sensor in the first window
void loopWindow1(){
  //No loop necessary, look in rareRefreshWindow1()
}

//loop for the EMG-Graph in the second window
uint16_t emgRead;
void loopWindow2(){
  emgRead = (uint16_t)MySignals.getEMG(DATA);
  emgRead = map(emgRead, 100, 600, 234, 66);
  printGraphic(emgRead,0);
  //TODO
}

//loop for the information page
void loopWindow3(){
  //No loop necessary, information is plotted at the start and never refreshed
}

//loop for any Strings that have to be refreshed for the body position window in the first window
void rareRefreshWindow1(){
  resetGUI(false, 1);
  tft.setTextColor(TEXT);
  tft.drawString(getBodyPositionText(), 8, 66, 2);
}

//loop for any Strings that have to be refreshed for the sensor in the second window
void rareRefreshWindow2(){
  //TODO
}

//rare loop for the information page
void rareRefreshWindow3(){
  //No code necessary
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
 * Create the loop function for all three windows.
 * Import the graph function from MySignals.
 * Create link to the sensors and choose which to read (temperature, pulseoximeter, ...)
 */
