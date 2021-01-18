//imnport of any necessary libraries
#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <UTouch.h>
#include <MySignals.h>
#include <MySignals_BLE.h>
#include <Wire.h>
#include <SPI.h>

//definition of screen variables and constants
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

//variables for the graph in the second window
#define graphic_low_extrem 233
#define graphic_high_extrem 66
#define graphic_left_extrem 6
#define graphic_right_extrem 206
int flickerReduction = 0;
uint16_t graphic_x = graphic_left_extrem;
uint16_t valRead; 
uint16_t graphic_prevRead;

/*
 * Function to get the text which is to be printed
 * in the window for the body position sensor.
 * @returns char array for immediate use in the tft.printString function
 */
char* getBodyPositionText(){
  uint8_t position = MySignals.getBodyPosition(); 
  switch (position){
    case 0:
      return "Test";
      break;
    case 1:
      showTrafficLight(1,1);
      return "Auf dem Bauch liegend";
      break;
    case 5:
      showTrafficLight(2,1);
      return "Stehend";
      break;
    case 2:
      showTrafficLight(1,1);
      return "Auf der linken Seite";
      break;
    case 3:
      showTrafficLight(1,1);
      return "Auf der rechten Seite";
      break;
    case 4:
      showTrafficLight(0,1);
      return "Auf dem Ruecken liegend";
      break;
    case 6:
      showTrafficLight(2,1);
      return "Unbekannte Position (WTF?)";
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

/*
 * @param scale  Chooses the current light on the traffic light. 0 for green, 1 for yellow, 2 for red.
 * @param window Chooses which windows specifications to use. 1 for left, 2 for center.
 */
void showTrafficLight(int scale, int window){
    switch (scale){
      case 0:
        tft.fillCircle(233, 98, 10, 0x4000);
        tft.fillCircle(233, 150, 10, 0x4220);
        tft.fillCircle(233, 202, 10, 0x0FE0);
        break;
      case 1:
        tft.fillCircle(233, 98, 10, 0x4000);
        tft.fillCircle(233, 150, 10, 0xEFE0);
        tft.fillCircle(233, 202, 10, 0x0220);
        break;
      case 2:
        tft.fillCircle(233, 98, 10, 0xF800);
        tft.fillCircle(233, 150, 10, 0x4220);
        tft.fillCircle(233, 202, 10, 0x0220);
        break;
      default:
        break;
    }
    tft.setTextColor(TEXT);
    switch (window){
      case 1:
        tft.drawString("Position", 250, 80, 2);
        tft.drawString("is", 250, 90, 2);
        tft.drawString("critical", 250, 100, 2);
        tft.drawString("Position", 250, 132, 2);
        tft.drawString("is", 250, 142, 2);
        tft.drawString("untypical", 250, 152, 2);
        tft.drawString("Position", 250, 184, 2);
        tft.drawString("is", 250, 194, 2);
        tft.drawString("normal", 250, 204, 2);
        break;
      case 2:
        tft.drawString("Muscle", 250, 80, 2);
        tft.drawString("Activity", 250, 90, 2);
        tft.drawString("is high", 250, 100, 2);
        tft.drawString("Muscle", 250, 132, 2);
        tft.drawString("Activity", 250, 142, 2);
        tft.drawString("is present", 250, 152, 2);
        tft.drawString("Muscle", 250, 184, 2);
        tft.drawString("Activity", 250, 194, 2);
        tft.drawString("is low", 250, 204, 2);
        break;
      default:
        break;
    }
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

/*
 * Sets the screen up for the first (left) window.
 */
void selectWindow1(){
  resetGUI(true, 1);
  tft.fillRect(6, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  tft.drawString("Body Postion", 16, 14, 2);
  tft.drawString("Sensor", 16, 28, 2);
}

/*
 * Sets the screen up for the second (center) window.
 */
void selectWindow2(){
  resetGUI(true, 2);
  tft.fillRect(112, 6, 96, 50, FOREGROUND_SELECT);
  tft.setTextColor(FOREGROUND);
  tft.drawString("Electromyo-", 122, 14, 2);
  tft.drawString("graphy", 122, 28, 2);
}

/*
 * Sets the screen up for the thrid (right) window.
 */
void selectWindow3(){
  resetGUI(true, 3);
  tft.setTextColor(TEXT);
  tft.drawString("Information Page", 13, 71, 4);
  tft.drawString("Created by:", 15, 90, 2);
  tft.drawString("    Till Moehring", 15, 105, 2);
  tft.drawString("    Ivo Opitz", 15, 120, 2);
  tft.drawString("Version Beta 4.592.1.02.12", 15, 150, 2);
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
 * @deprecated
 * Loop for the body postion sensor in the first window
 */
void loopWindow1(){
  //No loop necessary, look in rareRefreshWindow1() - can be removed
}

/*
 * Loop for the EMG-Graph in the second window
 */
void loopWindow2(){
  uint16_t emgRead;
  emgRead = (uint16_t)MySignals.getEMG(DATA);
  emgRead = map(emgRead, 100, 600, 234, 66);
  printGraphic(emgRead,0);
  //@TODO logic for the traffic light of the muscular activity
}

/*
 * @deprecated 
 * Loop for the information page
 */
void loopWindow3(){
  //No loop necessary, information is plotted at the start and never refreshed
}

/*
 * Loop for any Strings that have to be refreshed for the body position window in the first window.
 */
void rareRefreshWindow1(){
  resetGUI(false, 1);
  tft.setTextColor(TEXT);
  tft.drawString(getBodyPositionText(), 8, 66, 2);
}

//loop for any Strings that have to be refreshed for the sensor in the second window
void rareRefreshWindow2(){
  //@TODO - traffic light has to be implemented
}

/*
 * @deprecated
 */
void rareRefreshWindow3(){
  //No code necessary
}

/*
 * setup for the code, original Arduino method required to run the program
 */
void setup() {
  //setup rotary encoder
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
 * Loop, original method from Arduino required to run the program.
 * Checks if the window has to be changed and runs the loop and rareRefresh functions for the 3 windows according to which is currently selected.
 */
void loop() {
  // Logic to change the window if necessary
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
  // Executes the loop function corresponding to the selected window 
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
  /*
   * Run relevant rareRefresh function but only if the graph from the second window is at the end of its panel
   * This exakt value is not necessary for the first and thrid window but works well to stop flickering in all three windows.
   */
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
 * Create traffic light connection for EMG
 * Create global traffic light connection
 * Currently on pause due to corona pandemic circumstances: look in the Home_Version for new developments
 */
