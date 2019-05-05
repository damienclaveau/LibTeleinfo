// **********************************************************************************
// ESP8266 Teleinfo WEB Server routing Include file
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
// Modifié par marc Prieur 2019
//		-intégré le code dans la classe webClient webServer.cpp  webServer.h
//
// Using library ESP8266WebServer version 1.0
//
// **********************************************************************************

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
#include <Arduino.h> 
#include "Wifinfo.h"

// Web response max size
#define RESPONSE_BUFFER_SIZE 4096

const char FP_JSON_START[] PROGMEM = "{\r\n";
const char FP_JSON_END[] PROGMEM = "\r\n}\r\n";
const char FP_QCQ[] PROGMEM = "\":\"";
const char FP_QCNL[] PROGMEM = "\",\r\n\"";
const char FP_RESTART[] PROGMEM = "OK, Redémarrage en cours\r\n";
const char FP_NL[] PROGMEM = "\r\n";

class webServer : public ESP8266WebServer
{
public:
	webServer();
	void initSpiffs(void);
	void initServeur(void);
	void incNb_reinit(void);
private:
	bool handleFileRead(String path);

	void getSysJSONData(String & r);
	void getConfJSONData(String & r);
	void getSpiffsJSONData(String & r);
	void initOptVal(void);
	String getContentType(String filename);
	void formatNumberJSON(String &response, char * value);
	char optval[128];
	String formatSize(size_t bytes);
	int nb_reinit=0;
};
extern webServer WEBSERVER;

#endif