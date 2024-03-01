#include "EncoderStepCounter.h"
#include <SPI.h>
#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*****  Variables      **********************/

const int safetyButtonPIN = 13; // Safety Trigger disables all buttons and encoder when not pressed
int safetyState = 0;            // Sets safety sw to off on boot

/*************************************  BOOT IMAGES  *****************************************************************/

/*************************************  bootScreen() *****************************************************************/

Adafruit_SSD1306 display(-1);
void bootScreen()
{
  /*****  Initialize OLED and Show Bootscreen logo  *****/
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Set IC2 Address

  display.clearDisplay();
  // display.drawBitmap(0, 0, epd_bitmap_tomlogo, 128, 64, WHITE);
  // display.display();
  // delay(2500);

  // display.clearDisplay();
  // display.drawBitmap(0, 0, epd_bitmap_Arduino, 128, 64, WHITE);
  // display.display();
  // delay(2500);
}

/*
        Keypad assignements

X         Y                 Z             XY

0.01      0.1               1             10

F10       F50               F100          F240

HOME      ZERO              Motor         Read
XYZ       Abs               Off           SD
G28       G92 X0 Y0 Z0      M84           M22 followed by
                                          M21
*/

String moveAxis = "Z"; // Set for default axis on power //#2 - Z axis
String moveAxis2 = ""; // Set for default axis on power //#2 - Z axis
float moveRes = 0.01;  // Set for default axis on power //#2 - 1mm resolution
float feedRate = 10.0;
String gsMoveRes = "0.01";
String gsFeedRate = "10.0";

String currentDir = "";

char commandBuffer[64] = "\0";
char userInfoBuffer[64] = "\0";

void oledUpdate()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(moveAxis);
  if (moveAxis2 != "")
  {
    // display.print(" ");
    display.print(moveAxis2);
  }
  display.print(" ");
  display.print(moveRes);
  display.print("mm");
  display.print(" ");
  display.print("F");
  display.print(feedRate);
  // display.setTextSize(1);
  // display.println();

  // display.println(VersionMessage);
  display.display();
}

void displayCommandBuffer()
{
  display.clearDisplay();

  oledUpdate();

  // display.setCursor(0, 16);
  // display.println("                           ");
  display.setCursor(0, 16);
  display.println(commandBuffer);
  display.display();
}

bool canSendCommand()
{
  // ---- COMENT LINE BELOW AFTER WIRING THE SAFETY BUTTON
  return true; // <---------------------------------------

  // ---- read safe buttons state
  safetyState = digitalRead(safetyButtonPIN); // read the state of the safetyButton value
  if (safetyState == LOW)
    return true;
  else
    return true;
}

void dispatchCommand(bool bRequiresAuth = true, bool bM117 = true)
{
  if (bRequiresAuth)
    if (!canSendCommand())
      return;

  if (bM117)
  {
    sprintf(userInfoBuffer, "M117 %s", commandBuffer);
    Serial.println(userInfoBuffer);
    userInfoBuffer[0] = '\0';
  }

  Serial.println(commandBuffer);

  displayCommandBuffer();

  // --- clear last dispatched command
  commandBuffer[0] = '\0';
}

long int g91Dispatched = -1;
void dispatchCommand_MoveByEncoder()
{
  if (!canSendCommand())
    return;

  // dispatchCommand(true, false);
  // dtostrf(moveRes, 6, 2, moveresBuff); // 8 char min total width, 6 after decimal

  // String sMoveRes = String(moveRes);
  // String sFeedRate = String(feedRate);

  sprintf(commandBuffer, "G1 %s%s%s %s%s%s F%s\0", moveAxis.c_str(), currentDir.c_str(), gsMoveRes.c_str(),
          moveAxis2 != "" ? moveAxis2.c_str() : "", moveAxis2 != "" ? currentDir.c_str() : "", moveAxis2 != "" ? gsMoveRes.c_str() : "",
          gsFeedRate.c_str());

  if (g91Dispatched == -1)
  {
    Serial.println("G91");
    g91Dispatched = millis();
  }
  // else
  // {
  //   g91Dispatched = millis();
  // }

  Serial.println(commandBuffer);

  // --- only send if G91 was sent
  if (g91Dispatched != -1)
  {
    Serial.println("G90");
    g91Dispatched = -1;
  }

  displayCommandBuffer();

  commandBuffer[0] = '\0';

  // if there is a second axis selected (2ndAxis name != ' ')
}

/*****  Define KEYPAD  **********************/
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns

char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {12, 11, 10, 9}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6, 5};    // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void sendM117_variables()
{
  Serial.print("M117 ");
  Serial.print(moveAxis);
  if (moveAxis2 != "")
  {
    // Serial.print("");
    Serial.print(moveAxis2);
  }
  Serial.print(" ");
  Serial.print(moveRes);
  Serial.print("mm");

  Serial.print(" ");
  Serial.print("F");
  Serial.println(feedRate);
}

void setActiveAxis(KeypadEvent ekey)
{
  switch (ekey)
  {
  case '1':
    moveAxis = "X";
    moveAxis2 = "";
    break;
  case '2':
    moveAxis = "Y";
    moveAxis2 = "";
    break;
  case '3':
    moveAxis = "Z";
    moveAxis2 = "";
    break;
  case 'A':
    moveAxis = "X";
    moveAxis2 = "Y";
    break;
  }

  oledUpdate();
  sendM117_variables();
}

void setRes(KeypadEvent ekey)
{
  switch (ekey)
  {
  case '4':
    moveRes = 0.01;
    break;
  case '5':
    moveRes = 0.1;
    break;
  case '6':
    moveRes = 1.0;
    break;
  case 'B':
    moveRes = 10.0;
    break;
  }

  gsMoveRes = String(moveRes);

  oledUpdate();
  sendM117_variables();
}

void setFeed(KeypadEvent ekey)
{
  switch (ekey)
  {
  case '7':
    feedRate = 10.0;
    break;
  case '8':
    feedRate = 50.0;
    break;
  case '9':
    feedRate = 100.0;
    break;
  case 'C':
    feedRate = 240.0;
    break;
  }

  gsFeedRate = String(feedRate);

  oledUpdate();
  sendM117_variables();
}

void reinitSD(bool requiresAuth = false)
{
  sprintf(commandBuffer, "M22");
  dispatchCommand(requiresAuth, true);
  delay(2000);
  sprintf(commandBuffer, "M21");
  dispatchCommand(requiresAuth, true);
}

bool bStopSent = false;

void keypadEvent(KeypadEvent ekey)
{

  if (keypad.getState() == PRESSED)
  {
    switch (ekey)
    {
    case '1':
    case '2':
    case '3':
    case 'A':
      setActiveAxis(ekey);
      break;

    case '4':
    case '5':
    case '6':
    case 'B':
      setRes(ekey);
      break;

    case '7':
    case '8':
    case '9':
    case 'C':
      setFeed(ekey);
      break;

    case '*':
      sprintf(commandBuffer, "G28");
      dispatchCommand(true, true);
      break;
    case '0':
      sprintf(commandBuffer, "G92 X0 Y0 Z0");
      dispatchCommand(false, true);
      break;
    case '#':
      sprintf(commandBuffer, "M84");
      dispatchCommand(false, true);
      bStopSent = !bStopSent;
      break;
    case 'D':
      reinitSD(false);
      break;
    }
  }
}

/*****  Define ENCODER pins **********************/
#define ENCODER_PIN1 2
#define ENCODER_INT1 digitalPinToInterrupt(ENCODER_PIN1) // A
#define ENCODER_PIN2 3
#define ENCODER_INT2 digitalPinToInterrupt(ENCODER_PIN2) // B

// Create instance for one full step encoder
EncoderStepCounter encoder(ENCODER_PIN1, ENCODER_PIN2);
// Use the following for half step encoders
// EncoderStepCounter encoder(ENCODER_PIN1, ENCODER_PIN2, HALF_STEP);

// This is an example on how to change a "long" variable
// with the library. With every loop the value is added
// and then cleared in the encoder library
signed long position = 0;
signed long prevPosition = 0;

bool bEncIntEnabled = true; // normal mode in loop or interrupt
signed char lastpos = 0;

// Call tick on every change interrupt
void interrupt()
{
  encoder.tick();
  // Serial.println("click");
}

void setup()
{
  Serial.begin(250000);
  while (!Serial)
    ;

  Serial.println("<setup()>");

  bootScreen();

  oledUpdate();

  // Initialize encoder
  encoder.begin();

  // Initialize interrupts
  if (bEncIntEnabled)
  {
    attachInterrupt(ENCODER_INT1, interrupt, CHANGE);
    attachInterrupt(ENCODER_INT2, interrupt, CHANGE);
  }

  lastpos = encoder.getPosition();
  Serial.print("lastPos:");
  Serial.println(lastpos);

  /***** SAFETY PINS *****/
  pinMode(safetyButtonPIN, INPUT_PULLUP); // Setup safetyButton pin

  /*****  Start keypad listener  *****/
  keypad.addEventListener(keypadEvent);
  keypad.setDebounceTime(20);
  keypad.setHoldTime(0);

  Serial.println("</setup()>");
}

void loop()
{
  if (!bEncIntEnabled)
  {
    // Call tick on every loop
    encoder.tick();
  }

  signed char pos = encoder.getPosition();

  if (bEncIntEnabled)
  {
    if (pos != 0)
    {
      position += pos;

      if (prevPosition > position)
        currentDir = "-";
      if (prevPosition < position)
        currentDir = "";

      // Serial.println(position);

      encoder.reset();

      prevPosition = position;

      dispatchCommand_MoveByEncoder();
    }
  }
  else
  {
    if (pos != lastpos)
    {

      if (lastpos > pos)
        currentDir = "-";
      if (lastpos < pos)
        currentDir = "";

      // Serial.print(pos);
      // Serial.print("   ");
      // Serial.println(lastpos);

      lastpos = pos;

      dispatchCommand_MoveByEncoder();
    }
  }

  keypad.getKey(); // read keypad
  // delay(1);        // delay to help debounce

  // if (g91Dispatched != -1)
  // {
  //   long int now = millis();
  //   if (now - g91Dispatched >= 2000)
  //   {
  //     Serial.println("G90");
  //     g91Dispatched = -1;
  //   }
  // }
}