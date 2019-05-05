// **********************************************************************************
// ESP8266 Teleinfo WEB Client, routine include file
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use, see my blog
// http://hallard.me/category/tinfo
//
// This program works with the Wifinfo board
// see schematic here https://github.com/hallard/teleinfo/tree/master/Wifinfo
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-12-04 - First release
//
// All text above must be included in any redistribution.
//
// Modifié par marc Prieur 2019
//		-V2.0.0:intégré le code dans la classe webClient webClient.cpp  webClient.h
//		-V2.0.2:ajout de TAILLEBUFEMONCMS
// Using library ESP8266HTTPClient version 1.1
//
// **********************************************************************************

#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <Arduino.h> 
#include "Wifinfo.h"
#define TAILLEBUFEMONCMS 400
class webClient
{
public:
	webClient(boolean modeLinkyHistorique);
	boolean emoncmsPost(void);
	boolean jeedomPost(void);
	boolean httpRequest(void);
	//boolean UPD_switch(void);
	String  build_emoncms_json(void);
private:
	boolean httpPost(char * host, uint16_t port, char * url);
	boolean modeLinkyHistorique;
};
extern webClient WEBCLIENT;
#endif
