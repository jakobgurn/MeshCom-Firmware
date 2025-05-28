/**
 * FunctionalButton.ino - Example for the OneButtonLibrary library.
 * This is a sample sketch to show how to use OneClick library functionally on ESP32,ESP8266... 
 * 
 */
#include <Arduino.h>
#include <OneButton.h>

#include "configuration.h"
#include "loop_functions_extern.h"
#include "loop_functions.h"
#include "command_functions.h"

#include "onebutton_functions.h"

#if defined(BOARD_T_DECK) || defined(BOARD_T_DECK_PLUS)
#include <lvgl.h>
#include <t-deck/tdeck_main.h>
#include <t-deck/lv_obj_functions.h>
#endif 

class Button
{
private:
  OneButton onebutton;
  int value;
public:
  explicit Button(uint8_t pin):onebutton(pin)
  {
    onebutton.attachClick([](void *scope) { ((Button *) scope)->Clicked();}, this);
    onebutton.attachDoubleClick([](void *scope) { ((Button *) scope)->DoubleClicked();}, this);
    onebutton.attachMultiClick([](void *scope) { ((Button *) scope)->MultiClicked();}, this);
    onebutton.attachLongPressStart([](void *scope) { ((Button *) scope)->LongPressed();}, this);
  }

  void Clicked()
  {
    if(bDisplayCont)
    {
      Serial.println("Click then value++");
    }

    value++;

    // no oneclick on TRACK=on
    if(!bDisplayTrack)
    {
      if(bDisplayCont)
          Serial.printf("BUTTON singel press last:%i pointer:%i lines:%i\n", pageLastPointer, pagePointer, pageLastLineAnz[pagePointer]);

      if(pagePointer == 5)
        pagePointer = pageLastPointer-1;
        
      if(pageLastLineAnz[pagePointer] == 0)
        pagePointer = pageLastPointer-1;

      bDisplayIsOff=false;

      pageLineAnz = pageLastLineAnz[pagePointer];
      for(int its=0;its<pageLineAnz;its++)
      {
          // Save last Text (init)
          pageLine[its][0] = pageLastLine[pagePointer][its][0];
          pageLine[its][1] = pageLastLine[pagePointer][its][1];
          pageLine[its][2] = pageLastLine[pagePointer][its][2];
          memcpy(pageText[its], pageLastText[pagePointer][its], 25);
          if(its == 0)
          {
              for(int iss=0; iss < 20; iss++)
              {
                  if(pageText[its][iss] == 0x00)
                      pageText[its][iss] = 0x20;
              }
              pageText[its][19] = pagePointer | 0x30;
              pageText[its][20] = 0x00;
          }
      }

      #ifdef BOARD_E290
          iDisplayType=9;
      #else
          iDisplayType=0;
      #endif

      strcpy(pageTextLong1, pageLastTextLong1[pagePointer]);
      strcpy(pageTextLong2, pageLastTextLong2[pagePointer]);

      sendDisplay1306(false, true, 0, 0, (char*)"#N");

      pagePointer--;
      if(pagePointer < 0)
          pagePointer=PAGE_MAX-1;

      pageHold=5;
    }
  }

  void DoubleClicked()
  {

    if(bDisplayCont)
      Serial.println("DoubleClick");

    if(bDisplayTrack)
        commandAction((char*)"--sendtrack", false);
    else
        commandAction((char*)"--sendpos", false);
  }

  void MultiClicked()
  {
    if(bDisplayCont)
      Serial.println("MultiClick (3)");

    bDisplayTrack=!bDisplayTrack;

    bDisplayIsOff=false;

    if(bDisplayTrack)
        commandAction((char*)"--track on", false);
    else
        commandAction((char*)"--track off", false);

    sendDisplayHead(false);
  }

  void LongPressed()
  {
    if(bDisplayCont)
    {
      Serial.print("LongPress and the value is ");
      Serial.println(value);
    }

    bShowHead=false;

    #ifdef BOARD_E290
        sendDisplayMainline();
        E290DisplayUpdate();
    #else
        pageHold=0;
        bDisplayOff=!bDisplayOff;

        if(bDisplayOff)
        {
            commandAction((char*)"--display off", isPhoneReady, false);
        }
        else
        {
            commandAction((char*)"--display on", isPhoneReady, false);
        }

    #endif
  }

  void handle()
  {
    onebutton.tick();
  }
};

#if defined(BUTTON_PIN)
  
Button onebutton(BUTTON_PIN); //iButtonPin);

#endif

void init_onebutton()
{
  if(bButtonCheck)
  {
    // none
  }

}

void loop_onebutton()
{
    #if defined(BOARD_T_DECK) || defined(BOARD_T_DECK_PLUS)
    
    button.check();
    lv_task_handler();

    #elif defined(BUTTON_PIN)
  
    if(bButtonCheck)
      onebutton.handle();

    #else
    // none
    #endif
}