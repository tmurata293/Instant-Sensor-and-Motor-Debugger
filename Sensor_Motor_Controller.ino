#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeMono12pt7b.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//sensor declaration
int digitalSensor, analogSensor, motorValue, motorPin, motorPWM;  

//define button
Adafruit_GFX_Button startButton, menuScreenButton[4], scrollButton[4], analogOnOffButton, digitalOnOffButton, digitalInputOutputButton, motorSpeedButton[4];
Adafruit_GFX_Button lockButton[10];

//screen On/ Off indicator
bool startScreenOn = true; 
bool menuScreenOn = false;
bool digitalScreenOn = false;
bool analogScreenOn = false;
bool motorScreenOn = false;
bool analogSensorOn = false;
bool digitalSensorOn = false; 
bool digitalInput = true; 
bool forwardDirection = true;

//pascode
int passwordSet = 401;
int typedPassword = 1; 
int row = 0;
int count = 3;
int digit = 0; 

//display message
String menuLabel[3] = {"DIGITAL SENSOR", "ANALOG SENSOR", ""};

int pageLocationD, currentPageD, pageD, pageLocationA, currentPageA, pageA, pageLocationM, currentPageM, pageM;
String digChar, anaChar, motorChar;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  Serial.println("Sensor Control");
  
  tft.begin();
  if (!ts.begin())
  {
    Serial.println("Can't start touch screen");
    while(1);  
  }
  Serial.println("Touch screen started");
  for (int i=2; i <8; i++)
  {
    pinMode(i, INPUT); 
  }
  tft.setFont(&FreeMono12pt7b); 
  startScreen();  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  TS_Point point; 
  if (!ts.bufferEmpty())
  {
    point = ts.getPoint(); 
  }
  else
  {
    point.x = point.y = point.z = -1; 
  }

  if (point.z != -1)
  {
    point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
    point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());
  }

  /*******Check which button was pressed*********/
  
  if (startButton.contains(point.x, point.y))
  {
     startButton.press(true); 
  }
  else if (analogOnOffButton.contains(point.x, point.y))
  {
     analogOnOffButton.press(true);
  }
  else if (digitalOnOffButton.contains(point.x, point.y))
  {
    digitalOnOffButton.press(true);
  }
  else if (digitalInputOutputButton.contains(point.x, point.y))
  {
    digitalInputOutputButton.press(true);
  }
  else
  {
    startButton.press(false);
    analogOnOffButton.press(false);
    digitalOnOffButton.press(false);
    digitalInputOutputButton.press(false); 
  }


  for (int index=0; index < 4; index++)
  {
    if (menuScreenButton[index].contains(point.x, point.y))
    {
      menuScreenButton[index].press(true);
    }
    else if (scrollButton[index].contains(point.x, point.y))
    {
      scrollButton[index].press(true);
    }
    else if (motorSpeedButton[index].contains(point.x, point.y))
    {
    motorSpeedButton[index].press(true);
    }
    else
    {
      menuScreenButton[index].press(false); 
      scrollButton[index].press(false);
      motorSpeedButton[index].press(false);
    }
  }

   for (int button = 0; button < 10; button++)
  {
    if (lockButton[button].contains(point.x, point.y))
    {
      lockButton[button].press(true); 
    }
    else
    {
       lockButton[button].press(false);
    }
  }

  
 /**********************************************************************************/
 /*****************main code********************************************************/ 
 /**********************************************************************************/

  
  if (startButton.justReleased() and startScreenOn == true)
  {
    Serial.print("startButton : ");
    Serial.println("released");
    lockScreen(); 
    startScreenOn = false;
  }
  else if (menuScreenButton[0].justReleased() and menuScreenOn == true)
  {
    Serial.print("menuScreenButton 0: ");
    Serial.println("released");
    pageLocationD = 0;
    currentPageD = 2;
    pageD = 0;
    digChar = "D"; 
    basePage(pageLocationD, digChar, currentPageD, pageD);  
    menuScreenOn = false;  
  }
  else if (menuScreenButton[1].justReleased() and menuScreenOn == true)
  {
    Serial.print("menuScreenButton 1: ");
    Serial.println("released");
    pageLocationA = 0;
    currentPageA = 0;
    pageA = 0;
    anaChar = "A";
    basePage(pageLocationA, anaChar, currentPageA, pageA); 
    menuScreenOn = false; 
  }
  else if (menuScreenButton[2].justReleased() and menuScreenOn == true)
  {
    Serial.print("menuScreenButton 2: ");
    Serial.println("released");
    pageLocationM = 0;
    currentPageM = 1;
    pageM = 0;
    motorChar = "M";
    basePage(pageLocationM, motorChar, currentPageM, pageM);  
    menuScreenOn = false; 
  }
  else if (scrollButton[2].justReleased() and menuScreenOn == false)
  {
    Serial.print("scrollButton 2: ");
    Serial.println("released");
    digitalScreenOn = false;
    analogScreenOn = false;
    motorScreenOn = false;
    menuScreen(); 
    menuScreenOn = true;
  }


/********************************************/
/**************Passcode Check****************/

  if (count >= 0)
 {
      for (int button=0; button<10; button++)
      {
         if (lockButton[button].justPressed())
         {
             Serial.print("Pushed: "); 
             Serial.println(button); 
             delay(200);  
         }

         if (lockButton[button].justReleased())
         {
            Serial.print("Released: "); 
            Serial.println(button);
            tft.fillCircle(75 + row*30, 60, 5, ILI9341_DARKGREY);
            typedPassword += button * pow(10,count);  
            Serial.print("Typed Password: "); 
            Serial.println(typedPassword);   
            count--;
            row++;
            digit++; 
            Serial.print("Count: "); 
            Serial.println(count);
            Serial.print("Digit: "); 
            Serial.println(digit);  
         } 
      }
  }
 
  if (typedPassword == passwordSet && digit == 4)
  {
        digit ++;
        delay(800); 
        menuScreen(); 
        
  }
  else if (typedPassword != passwordSet && digit == 4)
  { 
      delay(2000); 
      count = 3;
      digit = 0; 
      row = 0;
      typedPassword = 1; 
      lockScreen();  
  }
  
   /********************************************/
   /**************DIGITAL SENSOR****************/

   if (digitalScreenOn == true)
   {
      if (pageD >= 0 and pageD <= 5)
      {
           if (scrollButton[3].justPressed())
           {
            Serial.print("scrollButton 3: ");
            Serial.println("pressed");
            delay(200); 
           }
           else if (scrollButton[0].justPressed())
           {
            Serial.print("scrollButton 0: ");
            Serial.println("pressed");
            delay(200);
           }
           else if (digitalOnOffButton.justPressed())
           {
             Serial.print("digitalOnOffButton : ");
             Serial.println("pressed");
             delay(200); 
           }
           else if (digitalInputOutputButton.justPressed())
           {
             Serial.print("digitlInputOutputButton : ");
             Serial.println("pressed");
             delay(200); 
           }
       }
       
    if (pageD >= 0 and pageD <= 5)
    {
       if (scrollButton[3].justReleased())
       {
            Serial.print("scrollButton 3: ");
            Serial.println("released"); 
            currentPageD++; 
            pageD++;
            if (pageD == 5)
            {
               pageLocationD = 1;
            }
            else
            {
              pageLocationD = 2;
            }
            basePage(pageLocationD, digChar, currentPageD, pageD);
       }
       else if (scrollButton[0].justReleased())
       {
            Serial.print("scrollButton 0: ");
            Serial.println("released");
            currentPageD--; 
            pageD--;
            if (pageD == 0)
            {
               pageLocationD = 0;
            }
            else
            {
              pageLocationD = 2;
            } 
            basePage(pageLocationD, digChar, currentPageD, pageD);
       }
       else if (digitalInputOutputButton.justReleased())
       {
         if (digitalInput == true)
         {
             tft.fillRect(20,60,200,25,ILI9341_LIGHTGREY);
             tft.setCursor(30,80);
             tft.println("Sensor Output");
             pinMode(currentPageD, OUTPUT);
             digitalInput = false; 
             digitalInputOutputButton.drawButton(); 
             tft.setCursor(120,120);
             tft.println("INPUT");
         }
         else if (digitalInput == false)
         {
            tft.fillRect(20,60,200,25,ILI9341_LIGHTGREY);
            tft.setCursor(30,80);
            tft.println("Sensor Input");
            pinMode(currentPageD, INPUT); 
            digitalInput = true;
            digitalInputOutputButton.drawButton(); 
            tft.setCursor(120,120);
            tft.println("OUTPUT");   
         }
       }
       else if (digitalInput == true)
       {
            tft.setCursor(90,170);
            int digitalValue; 
            digitalValue = digitalRead(currentPageD); 
            if (digitalValue == 1)
            {
               tft.println("HIGH");
            }
            else if (digitalValue == 0)
            {
              tft.println("LOW");
            } 
        
       }
       else if (digitalOnOffButton.justReleased() and digitalInput == false)
       {
         if (digitalSensorOn == false)
         {
             digitalSensorOn = true; 
             digitalOnOffButton.drawButton(); 
             tft.setCursor(40,120);
             tft.println("OFF");
             digitalWrite(currentPageD, HIGH); 
             tft.fillRect(50,140,150,50,ILI9341_LIGHTGREY);
             tft.setCursor(90,170);
             tft.println("HIGH");  
             Serial.print("digitalOnOffButton ");
             Serial.println("released");
             Serial.print("digitalSensorOn: ");
             Serial.println("true");
         }
         else if (digitalSensorOn == true)
         {
             digitalSensorOn = false;
             digitalOnOffButton.drawButton(); 
             tft.setCursor(40,120);
             tft.println("ON");
             digitalWrite(currentPageD, LOW); 
             tft.fillRect(50,140,150,50,ILI9341_LIGHTGREY);
             tft.setCursor(90,170); 
             tft.println("LOW");
             Serial.print("digitalOnOffButton ");
             Serial.println("released");
             Serial.print("digitalSensorOn: ");
             Serial.println("false"); 
         }
       }
      
    }
  }

  
  /*******************************************/
  /**************ANALOG SENSOR****************/
  
  else if (analogScreenOn == true)
   {
    
      if (pageA >= 0 and pageA <= 5)
      {
           if (scrollButton[3].justPressed())
           {
            Serial.print("scrollButton 3: ");
            Serial.println("pressed");
            delay(200); 
           }
           else if (scrollButton[0].justPressed())
           {
            Serial.print("scrollButton 0: ");
            Serial.println("pressed");
            delay(200);
           }
           
           else if (analogOnOffButton.justPressed())
           {
             Serial.print("analogOnOffButton : ");
             Serial.println("pressed");
             delay(200); 
           }
           
           
       }
       
       
    if (pageA >= 0 and pageA <= 5)
    {
       if (scrollButton[3].justReleased())
       {
            Serial.print("scrollButton 3: ");
            Serial.println("released"); 
            currentPageA++; 
            pageA++;
            if (pageA == 5)
            {
               pageLocationA = 1;
            }
            else
            {
              pageLocationA = 2;
            }
            basePage(pageLocationA, anaChar, currentPageA, pageA);
       }
       else if (scrollButton[0].justReleased())
       {
            Serial.print("scrollButton 0: ");
            Serial.println("released");
            currentPageA--; 
            pageA--;
            if (pageA == 0)
            {
               pageLocationA = 0;
            }
            else
            {
              pageLocationA = 2;
            } 
            basePage(pageLocationA, anaChar, currentPageA, pageA);
       }
       else if (analogOnOffButton.justReleased())
       {
          if (analogSensorOn == false)
          {
             analogSensorOn = true; 
             analogOnOffButton.drawButton(); 
             tft.setCursor(101,120);
             tft.println("OFF");
             Serial.print("analogOnOffButton ");
             Serial.println("released");
             Serial.print("analogSensorOn: ");
             Serial.println("true"); 
          }
          else if (analogSensorOn == true)
          {
             analogSensorOn = false;
             analogOnOffButton.drawButton(); 
             tft.setCursor(108,120);
             tft.println("ON");
             tft.fillRect(50,140,150,50,ILI9341_LIGHTGREY);
             tft.setCursor(52,170); 
             tft.println("No Values");
             Serial.print("analogOnOffButton ");
             Serial.println("released");
             Serial.print("analogSensorOn: ");
             Serial.println("false"); 
          }
       }
    }

    if (analogSensorOn == true)
    {
          tft.fillRect(50,140,150,50,ILI9341_LIGHTGREY);
          tft.setCursor(100,170); 
          analogSensor = analogRead(currentPageA); 
          tft.println(analogSensor);
          Serial.print ("analoSensor: ");
          Serial.println(analogSensor);
          delay(300); 
    }
  }
  /******************************************/
  /**************MOTOR SENSOR****************/
  
   else if (motorScreenOn == true)
   {
      if (pageM >= 0 and pageM <= 5)
      {
           if (scrollButton[3].justPressed())
           {
            Serial.print("scrollButton 3: ");
            Serial.println("pressed");
            delay(200); 
           }
           else if (scrollButton[0].justPressed())
           {
            Serial.print("scrollButton 0: ");
            Serial.println("pressed");
            delay(200);
           }
           else if (motorSpeedButton[0].justPressed())
           {
            Serial.print("motorSpeedButton 0: ");
            Serial.println("pressed");
            delay(200);
           }
           else if (motorSpeedButton[1].justPressed())
           {
            Serial.print("motorSpeedButton 1: ");
            Serial.println("pressed");
            delay(200);
           }
           else if (motorSpeedButton[2].justPressed())
           {
            Serial.print("motorSpeedButton 2: ");
            Serial.println("pressed");
            delay(200);
           }
           else if (motorSpeedButton[3].justPressed())
           {
            Serial.print("motorSpeedButton 2: ");
            Serial.println("pressed");
            delay(200);
           }
       }
       
    if (pageM >= 0 and pageM <= 5)
    {
       if (scrollButton[3].justReleased())
       {
            Serial.print("scrollButton 3: ");
            Serial.println("released"); 
            currentPageM++; 
            pageM++;
            if (pageM == 5)
            {
               pageLocationM = 1;
            }
            else
            {
              pageLocationM = 2;
            }
            basePage(pageLocationM, motorChar, currentPageM, pageM);
       }
       else if (scrollButton[0].justReleased())
       {
            Serial.print("scrollButton 0: ");
            Serial.println("released");
            currentPageM--; 
            pageM--;
            if (pageM == 0)
            {
               pageLocationM = 0;
            }
            else
            {
              pageLocationM = 2;
            } 
            basePage(pageLocationM, motorChar, currentPageM, pageM);
       }
        else if (motorSpeedButton[1].justReleased())
       {
             if (motorValue < 255)
             {
                  motorValue++;
                  if (forwardDirection == true)
                  {
                      digitalWrite(motorPin, LOW);
                      analogWrite(motorPWM, motorValue);
                  }
                  else if (forwardDirection == false)
                  {
                     digitalWrite(motorPin, HIGH);
                     analogWrite(motorPWM, (255-motorValue));
                  } 
                  tft.fillRect(70,140,100,40,ILI9341_LIGHTGREY);
                  tft.setCursor(100,170); 
                  tft.println(motorValue);
             }
       }
       else if (motorSpeedButton[0].justReleased())
       {
              if (motorValue > 0)
              {
                  motorValue--;
                  if (forwardDirection == true)
                  {
                  digitalWrite(motorPin, LOW);
                  analogWrite(motorPWM, motorValue);
                  }
                  else if (forwardDirection == false)
                  {
                     digitalWrite(motorPin, HIGH);
                     analogWrite(motorPWM, (255-motorValue));
                  } 
                  tft.fillRect(70,140,100,40,ILI9341_LIGHTGREY);
                  tft.setCursor(100,170); 
                  tft.println(motorValue);
              }
       }  
       else if (motorSpeedButton[2].justReleased())
       {
           if (forwardDirection == false)
           {
              digitalWrite(motorPin, LOW);
              analogWrite(motorPWM, motorValue);
              tft.fillRect(70,140,100,40,ILI9341_LIGHTGREY);
              tft.setCursor(100,170); 
              tft.println(motorValue); 
              tft.fillRect(30,180,120,30,ILI9341_LIGHTGREY);
              motorSpeedButton[2].drawButton();
              tft.setCursor(32,203);
              tft.println("Forward");
              forwardDirection = true;
           }
           else if (forwardDirection == true)
           {
              digitalWrite(motorPin, HIGH);
              //digitalWrite(motorPin, LOW);
              analogWrite(motorPWM, 255-motorValue);
              tft.fillRect(70,140,100,40,ILI9341_LIGHTGREY);
              tft.setCursor(100,170); 
              tft.println(motorValue); 
              tft.fillRect(30,180,120,30,ILI9341_LIGHTGREY); 
              motorSpeedButton[2].drawButton(); 
              tft.setCursor(32,203);
              tft.println("Reverse");
              forwardDirection = false;
           }
       }
       else if (motorSpeedButton[3].justReleased())
       {
              motorValue = 0;
              digitalWrite(motorPin, LOW);
              analogWrite(motorPWM, motorValue);
              tft.fillRect(70,140,100,40,ILI9341_LIGHTGREY);
              tft.setCursor(100,170); 
              tft.println(motorValue);   
       }
       
       
    } 
  }

  
}
/*********start up screen**********/
void startScreen()
{ 
  tft.fillScreen(ILI9341_LIGHTGREY);
  tft.setCursor(0,50); 
  tft.setTextColor(ILI9341_DARKGREY); 
  tft.println("Sensor Controller"); 
  delay(500); 
  tft.setCursor(70, 150); 
  tft.println("WELCOME");
  delay(500);   
  
  startButton.initButton(&tft, 120, 250, 100, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1); 
  startButton.drawButton();
  tft.setCursor(85, 255);
  tft.println("START"); 
}


/******** menu screen **********/
void menuScreen()
{
   menuScreenOn = true;
   
   tft.fillScreen(ILI9341_LIGHTGREY);
   tft.setCursor(95,30); 
   tft.setTextColor(ILI9341_DARKGREY); 
   tft.println("MENU"); 
   for (int button=0; button < 3; button++)
    {
         menuScreenButton[button].initButton(&tft, 120, 100+60*button, 220, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
         menuScreenButton[button].drawButton(); 
         tft.setCursor(25, 105 +60*button); 
         tft.println(menuLabel[button]); 
    }
    tft.setCursor(78, 105 +60*2); 
    tft.println("MOTOR");
}


/**********base page ***********/
      //pageLocation 0 = left edge ; 1 = right edge ; 2 = center 
      //type: digital = 'D'    analog = 'A'    motor = 'M'
      
void basePage(int pageLocation, String type, int currentPage, int page)
{
  tft.fillScreen(ILI9341_LIGHTGREY);

  /*****CREATE BUTTON and LABEL*******/
  
  //left = 0, right = 1, bottom = 2
  scrollButton[0].initButton(&tft, 23, 160, 30, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
  scrollButton[3].initButton(&tft, 219, 160, 30, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
  scrollButton[2].initButton(&tft, 122, 280, 40, 40, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
  scrollButton[2].drawButton(); 
  tft.setCursor(115,285); 
  tft.println("O");
 
  if (pageLocation == 2)
  {
    scrollButton[0].drawButton();
    scrollButton[3].drawButton();
    tft.setCursor(9,166); 
    tft.println("<<"); 
    tft.setCursor(205,166); 
    tft.println(">>");
  }
  else if (pageLocation == 0)
  {
    scrollButton[3].drawButton();
    tft.setCursor(205,166); 
    tft.println(">>");
  }
  else if (pageLocation == 1)
  {
    scrollButton[0].drawButton();
    tft.setCursor(9,166); 
    tft.println("<<");
  }

  /*****create page display******/ 
    for (int num=0; num < 6; num++)
  {
    tft.drawCircle(45 + num*30, 230, 5, ILI9341_DARKGREY); 
  }
  tft.fillCircle(45 +  page*30, 230, 5, ILI9341_DARKGREY);
  

  /******* screen on each page *******/ 
  tft.setCursor(0,24);  
  if (type == "D")
  {
     tft.println("DIGITAL");
     digitalOnOffButton.initButton(&tft, 55, 115, 60, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
     digitalOnOffButton.drawButton(); 
     tft.setCursor(40,120);
     tft.println("ON");
     digitalInputOutputButton.initButton(&tft, 163, 115, 100, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
     digitalInputOutputButton.drawButton(); 
     tft.setCursor(120,120);
     tft.println("OUTPUT");
     digitalScreenOn = true;
     tft.setCursor(30,80);
     tft.println("Sensor Input"); 
     Serial.print("digitalScreenOn: ");
     Serial.println("true"); 
  }
  else if (type == "A")
  {
    tft.println("ANALOG");
    analogOnOffButton.initButton(&tft, 123, 115, 60, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
    analogOnOffButton.drawButton(); 
    tft.setCursor(108,120);
    tft.println("ON"); 
    analogScreenOn = true;
    tft.setCursor(38,80);
    tft.println("Sensor Value"); 
    tft.setCursor(52,170); 
    tft.println("No Values");
  }
  else if (type == "M")
  {
    tft.println("MOTOR");
    motorScreenOn = true; 
    tft.setCursor(45,80);
    tft.println("Motor Speed");
    motorSpeedButton[0].initButton(&tft, 80, 115, 40, 40, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
    motorSpeedButton[0].drawButton(); 
    tft.setCursor(72,120);
    tft.println("-"); 
    motorSpeedButton[1].initButton(&tft, 160, 115, 40, 40, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
    motorSpeedButton[1].drawButton(); 
    tft.setCursor(152,120);
    tft.println("+"); 
    motorSpeedButton[2].initButton(&tft, 81, 200, 120, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
    motorSpeedButton[2].drawButton(); 
    tft.setCursor(32,203);
    tft.println("Reverse"); 
    motorSpeedButton[3].initButton(&tft, 181, 200, 60, 30, ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
    motorSpeedButton[3].drawButton(); 
    tft.setCursor(152,203);
    tft.println("Stop"); 

    motorValue = 0; 
    motorPin =  2;
    motorPWM =  3;
    pinMode(motorPin, OUTPUT);
    pinMode(motorPWM, OUTPUT); 
  } 
  tft.setCursor(100,24);
  tft.println(currentPage); 
}



/******************Passcode**********************/
/************************************************/

void lockScreen()
{
  tft.setFont(&FreeMono12pt7b);
  tft.setTextColor(ILI9341_DARKGREY); 
  tft.fillScreen(ILI9341_LIGHTGREY);
  tft.setCursor(65,30);  
  tft.println("PASSCODE"); 
  for (int row=0; row < 4; row++)
  {
    tft.drawCircle(75 + row*30, 60, 5, ILI9341_DARKGREY); 
  }
 
  for (int row=0; row < 3; row++)
  {
    for (int col=0; col< 3; col++)
    {
      lockButton[col+row*3+1].initButton (&tft, 70 + col*50, 120 + row*50, 40, 40,ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1); 
       lockButton[col+row*3+1].drawButton(); 
    }
  }
  lockButton[0].initButton (&tft, 120, 270, 40, 40,ILI9341_DARKGREY, ILI9341_WHITE, ILI9341_DARKGREY, "", 1);
  lockButton[0].drawButton(); 
  tft.setCursor( 114, 274); 
  tft.println("0"); 

  int count = 1; 
  for (int row=0; row < 3; row++)
  {
    for (int col=0; col< 3; col++)
    {
      tft.setCursor( 64 + col*50, 124+ row*50); 
      tft.println(count); 
      count++; 
    }
  }
}






