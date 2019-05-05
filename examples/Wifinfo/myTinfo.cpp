// **********************************************************************************
// ESP8266 Traitement teleinfo
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// This program works with the Wifinfo board
// see schematic here https://github.com/hallard/teleinfo/tree/master/Wifinfo
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// Modifié par Dominique DAMBRAIN 2017-07-10 (http://www.dambrain.fr)
//
// Modifié par marc PRIEUR 2019-05-01 ()
//		-intégré le code dans la classe myTinfo myTinfo.cpp
//
//
//********************************************************************************

#include <Arduino.h>
#include "Wifinfo.h"
#include "LibTeleinfo.h"
#include "config.h"
#include "leds.h"
#include "mySyslog.h"

#include "myTinfo.h"


/* ======================================================================
Function: Task_emoncms
Purpose : callback of emoncms ticker
Input   :
Output  : -
Comments: Like an Interrupt, need to be short, we set flag for main loop
====================================================================== */
volatile boolean task_emoncms = false;
void Task_emoncms()
{
	task_emoncms = true;
}

/* ======================================================================
Function: Task_jeedom
Purpose : callback of jeedom ticker
Input   :
Output  : -
Comments: Like an Interrupt, need to be short, we set flag for main loop
====================================================================== */
volatile boolean task_jeedom = false;
void Task_jeedom()
{
	task_jeedom = true;
}

/* ======================================================================
Function: Task_httpRequest
Purpose : callback of http request ticker
Input   :
Output  : -
Comments: Like an Interrupt, need to be short, we set flag for main loop
====================================================================== */
volatile boolean task_httpRequest = false;
void Task_httpRequest()
{
	task_httpRequest = true;
}

/* ======================================================================
Function: ADPSCallback
Purpose : called by library when we detected a ADPS on any phased
Input   : phase number
			0 for ADPS (monophase)
			1 for ADIR1 triphase
			2 for ADIR2 triphase
			3 for ADIR3 triphase
Output  : -
Comments: should have been initialised in the main sketch with a
		  tinfo.attachADPSCallback(ADPSCallback())
====================================================================== */
void ADPSCallback(uint8_t phase)
{
	// Monophasé
	if (phase == 0) {
		Debugln(F("ADPS"));
	}
	else {
		Debug(F("ADPS Phase "));
		Debugln('0' + phase);
	}
}

/* ======================================================================
Function: NewFrame
Purpose : callback when we received a complete teleinfo frame
Input   : linked list pointer on the concerned data
Output  : -
Comments: it's called only if one data in the frame is different than
		  the previous frame
====================================================================== */
void UpdatedFrame(ValueList * me)
{
	DebuglnF("UpdatedFrame received");
}

/* ======================================================================
Function: NewFrame
Purpose : callback when we received a complete teleinfo frame
Input   : linked list pointer on the concerned data
Output  : -
Comments: -
====================================================================== */
//volatile boolean nouvelleTrame = false;
void NewFrame(ValueList * me)
{
	// Light the RGB LED 
	if (CONFIGURATION.config.config & CFG_RGB_LED) {
		LESLEDS.LedRGBON(COLOR_GREEN);
		LESLEDS.tempoLedOff();
		// led off after delay
		//rgb_ticker.once_ms((uint32_t)BLINK_LED_MS, LedOff, (int)RGB_LED_PIN);
	}
	DebuglnF("NewFrame received");
}

/* ======================================================================
Function: DataCallback
Purpose : callback when we detected new or modified data received
Input   : linked list pointer on the concerned data
		  value current state being TINFO_VALUE_ADDED/TINFO_VALUE_UPDATED
Output  : -
Comments: -
====================================================================== */
//void DataCallback(ValueList * me, uint8_t flags)
//{

	// This is for simulating ADPS during my tests
	// ===========================================
	/*
	static uint8_t test = 0;
	// Each new/updated values
	if (++test >= 20) {
	  test=0;
	  uint8_t anotherflag = TINFO_FLAGS_NONE;
	  ValueList * anotherme = tinfo.addCustomValue("ADPS", "46", &anotherflag);

	  // Do our job (mainly debug)
	  DataCallback(anotherme, anotherflag);
	}
	Debugf("%02d:",test);
	*/
	// ===========================================

  /*
	// Do whatever you want there
	Debug(me->name);
	Debug('=');
	Debug(me->value);

	if ( flags & TINFO_FLAGS_NOTHING ) Debug(F(" Nothing"));
	if ( flags & TINFO_FLAGS_ADDED )   Debug(F(" Added"));
	if ( flags & TINFO_FLAGS_UPDATED ) Debug(F(" Updated"));
	if ( flags & TINFO_FLAGS_EXIST )   Debug(F(" Exist"));
	if ( flags & TINFO_FLAGS_ALERT )   Debug(F(" Alert"));

	Debugln();
  */
  //}

myTinfo::myTinfo()
{
}
void myTinfo::init(boolean modeLinkyHistorique)
{
	TINFO.init(modeLinkyHistorique);
	TINFO.attachADPS(ADPSCallback);
	TINFO.attachNewFrame(NewFrame);
	TINFO.attachUpdatedFrame(UpdatedFrame);


	// Emoncms Update if needed
	if (CONFIGURATION.config.emoncms.freq)
		Tick_emoncms.attach(CONFIGURATION.config.emoncms.freq, Task_emoncms);

	// Jeedom Update if needed
	if (CONFIGURATION.config.jeedom.freq)
		Tick_jeedom.attach(CONFIGURATION.config.jeedom.freq, Task_jeedom);

	// HTTP Request Update if needed
	if (CONFIGURATION.config.httpReq.freq)
		Tick_httpRequest.attach(CONFIGURATION.config.httpReq.freq, Task_httpRequest);


}

bool myTinfo::getTask_emoncms(void)
{
	if (task_emoncms)
	{
		task_emoncms = false;
		return true;
	}
	return false;
}

bool myTinfo::getTask_jeedom(void)
{
	if (task_jeedom)
	{
		task_jeedom = false;
		return true;
	}
	return false;
}
bool myTinfo::getTask_httpRequest(void)
{
	if (task_httpRequest)
	{
		task_httpRequest = false;
		return true;
	}
	return false;
}

