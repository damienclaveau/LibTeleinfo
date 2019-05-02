// **********************************************************************************
// ESP8266 Teleinfo WEB Server, route web function
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
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include "Wifinfo.h"
#include "mySyslog.h"
#include "myNTP.h"
#include "mywifi.h"
#include "myOTA.h"
#include "webclient.h"
#include "LibTeleinfo.h"
#include "leds.h"
#include "config.h"
#include "webServer.h"

//https://projetsdiy.fr/esp8266-web-serveur-partie2-interaction-arduino-interface-html/

webServer::webServer()
{
	//******************************sysJSONTable****************************************
	this->on("/system.json", [&]() {
		DebuglnF("Serving /system.json page...");
		String response = "";

		ESP.wdtFeed();  //Force software watchdog to restart from 0
		this->getSysJSONData(response);

		// Just to debug where we are
		//Debug(F("Serving /system page..."));
		this->send(200, "text/json", response);
		yield();  //Let a chance to other threads to work
	});
	//*******************************handleRoot***************************************
	this->on("/", [&]() {
		LedBluON();
		this->handleFileRead("/");
		LedBluOFF();
	});

	//*******************************handleFormConfig***************************************
	this->on("/config_form.json", [&]() {
		String response = "";
		int ret;

		LedBluON();

		// We validated config ?
		if (this->hasArg("save"))
		{
			int itemp=0;

			DebuglnF("===== Posted configuration");

			// WifInfo
			strncpy(CONFIGURATION.config.ssid, this->arg("ssid").c_str(), CFG_SSID_SIZE);
			strncpy(CONFIGURATION.config.psk, this->arg("psk").c_str(), CFG_PSK_SIZE);
			strncpy(CONFIGURATION.config.host, this->arg("host").c_str(), CFG_HOSTNAME_SIZE);
			strncpy(CONFIGURATION.config.ap_psk, this->arg("ap_psk").c_str(), CFG_PSK_SIZE);
			strncpy(CONFIGURATION.config.ota_auth, this->arg("ota_auth").c_str(), CFG_PSK_SIZE);
			itemp = this->arg("ota_port").toInt();
			CONFIGURATION.config.ota_port = (itemp >= 0 && itemp <= 65535) ? itemp : DEFAULT_OTA_PORT;

			strncpy(CONFIGURATION.config.syslog_host, this->arg("syslog_host").c_str(), 64);

			itemp = this->arg("syslog_port").toInt();
			CONFIGURATION.config.syslog_port = (itemp >= 0 && itemp <= 65535) ? itemp : DEFAULT_SYSLOG_PORT;

			// Emoncms
			strncpy(CONFIGURATION.config.emoncms.host, this->arg("emon_host").c_str(), CFG_EMON_HOST_SIZE);
			strncpy(CONFIGURATION.config.emoncms.url, this->arg("emon_url").c_str(), CFG_EMON_URL_SIZE);
			strncpy(CONFIGURATION.config.emoncms.apikey, this->arg("emon_apikey").c_str(), CFG_EMON_APIKEY_SIZE);
			itemp = this->arg("emon_node").toInt();
			CONFIGURATION.config.emoncms.node = (itemp >= 0 && itemp <= 255) ? itemp : 0;
			itemp = this->arg("emon_port").toInt();
			CONFIGURATION.config.emoncms.port = (itemp >= 0 && itemp <= 65535) ? itemp : CFG_EMON_DEFAULT_PORT;
			itemp = this->arg("emon_freq").toInt();
			if (itemp > 0 && itemp <= 86400) {
				// Emoncms Update if needed
				Tick_emoncms.detach();
				Tick_emoncms.attach(itemp, Task_emoncms);
			}
			else {
				itemp = 0;
			}
			CONFIGURATION.config.emoncms.freq = itemp;

			// jeedom
			strncpy(CONFIGURATION.config.jeedom.host, this->arg("jdom_host").c_str(), CFG_JDOM_HOST_SIZE);
			strncpy(CONFIGURATION.config.jeedom.url, this->arg("jdom_url").c_str(), CFG_JDOM_URL_SIZE);
			strncpy(CONFIGURATION.config.jeedom.apikey, this->arg("jdom_apikey").c_str(), CFG_JDOM_APIKEY_SIZE);
			strncpy(CONFIGURATION.config.jeedom.adco, this->arg("jdom_adco").c_str(), CFG_JDOM_ADCO_SIZE);
			itemp = this->arg("jdom_port").toInt();
			CONFIGURATION.config.jeedom.port = (itemp >= 0 && itemp <= 65535) ? itemp : CFG_JDOM_DEFAULT_PORT;
			itemp = this->arg("jdom_freq").toInt();
			if (itemp > 0 && itemp <= 86400) {
				// Emoncms Update if needed
				Tick_jeedom.detach();
				Tick_jeedom.attach(itemp, Task_jeedom);
			}
			else {
				itemp = 0;
			}
			CONFIGURATION.config.jeedom.freq = itemp;

			// HTTP Request
			strncpy(CONFIGURATION.config.httpReq.host, this->arg("httpreq_host").c_str(), CFG_HTTPREQ_HOST_SIZE);
			strncpy(CONFIGURATION.config.httpReq.path, this->arg("httpreq_path").c_str(), CFG_HTTPREQ_PATH_SIZE);
			itemp = this->arg("httpreq_port").toInt();
			CONFIGURATION.config.httpReq.port = (itemp >= 0 && itemp <= 65535) ? itemp : CFG_HTTPREQ_DEFAULT_PORT;

			itemp = this->arg("httpreq_freq").toInt();
			if (itemp > 0 && itemp <= 86400)
			{
				Tick_httpRequest.detach();
				Tick_httpRequest.attach(itemp, Task_httpRequest);
			}
			else {
				itemp = 0;
			}
			CONFIGURATION.config.httpReq.freq = itemp;

			itemp = this->arg("httpreq_swidx").toInt();
			if (itemp > 0 && itemp <= 65535)
				CONFIGURATION.config.httpReq.swidx = itemp;
			else
				CONFIGURATION.config.httpReq.swidx = 0;

			if (CONFIGURATION.saveConfig()) {
				ret = 200;
				response = "OK";
			}
			else {
				ret = 412;
				response = "Unable to save configuration";
			}

			CONFIGURATION.showConfig();
		}
		else
		{
			ret = 400;
			response = "Missing Form Field";
		}

		DebugF("Sending response ");Debug(ret);DebugF(":");Debugln(response);
		this->send(ret, "text/plain", response);

		LedBluOFF();
	});
	//****************************sendJSON******************************************
	this->on("/json", [&]() {
		boolean first_item = true;
		ValueList * me = TINFO.getList();
		String response = "";

		ESP.wdtFeed();  //Force software watchdog to restart from 0

		DebuglnF("Serving /json page...");
		// Got at least one ?
		if (me) {
			// Json start
			response += FPSTR(FP_JSON_START);
			response += F("\"_UPTIME\":");
			response += NTP.getSeconds1970();

			// Loop thru the node
			while (me->next) {
				if (!first_item)
					// go to next node
					me = me->next;

				if (!me->free) {
					if (first_item)
						first_item = false;

					if (WEBCLIENT.validate_value_name(me->name)) {
						//It's a known name : process the entry
						response += F(",\"");
						response += me->name;
						response += F("\":");
						formatNumberJSON(response, me->value);
					}
					else {
						Debugln("setReinit sendJSON");
						TINFO.setReinit();
					} // name validity
				} //free entry
			} //while
		   // Json end
			response += FPSTR(FP_JSON_END);
		}
		else {
			this->send(404, "text/plain", "No data");
		}
		this->send(200, "text/json", response);
		yield();  //Let a chance to other threads to work
	});
	//******************************tinfoJSONTable****************************************
	this->on("/tinfo.json", [&]() {
		// we're there
		ESP.wdtFeed();  //Force software wadchog to restart from 0

		ValueList * me = TINFO.getList();
		String response = "";

		// Just to debug where we are
		Debugln(F("Serving /tinfo page...\r\n")); 

		if (!me) //&& first_info_call) 
		{
			//Let tinfo such time to build a list....
			DebugF("No list yet...\r\n");
			//first_info_call=false;
			unsigned long topdebut = millis();
			bool expired = false;
			while (!expired) {
				if ((millis() - topdebut) >= 3000) {
					expired = true;   // 3 seconds delay expired
				}
				else {
					yield();  //Let CPU to other threads
				}
			}
			// continue, hoping list values is now ready
			me = TINFO.getList();
		}
		//TINFO.valuesDump(); 
		// Got at least one ?
		if (me) {
			uint8_t index = 0;
			DebuglnF("tinfoJSONTable me...\r\n");
			//first_info_call=false;
			boolean first_item = true;
			// Json start
			response += F("[\r\n");

			// Loop thru the node
			while (me->next) {
				index++;

				if (!first_item)
					// go to next node
					me = me->next;
				else
					if (me->free) {
						//1st item is free : empty list !
						Debugln("Teleinfo list is empty !");
						break;
					}

				if (!me->free) {
					// First item do not add , separator
					if (first_item)
						first_item = false;
					else
						response += F(",\r\n");
					if (WEBCLIENT.validate_value_name(me->name)) {
						//It's a known name : process the entry  
						response += F("{\"na\":\"");
						response += me->name;
						response += F("\", \"va\":\"");
						response += me->value;
						response += F("\", \"ck\":\"");
						if (me->checksum == '"' || me->checksum == '\\' || me->checksum == '/')
							response += '\\';
						response += (char)me->checksum;
						response += F("\", \"fl\":");
						response += me->flags;
						response += '}';
					}
					else {
						//Don't put this line in table : name is corrupted !
						Debugln("setReinit tinfoJSONTable");
						TINFO.setReinit();
					}
				}
			}
			// Json end
			response += F("\r\n]");
		}
		else {
			Debugln(F("sending 404..."));
			this->send(404, "text/plain", "No data");
		}
		//Debug(F("sending..."));
		this->send(200, "text/json", response);
		//Debugln(response);
		//Debugln(F("OK!"));
		yield();  //Let a chance to other threads to work
	});
	//******************************emoncmsJSONTable****************************************
	this->on("/emoncms.json", [&]() {
		DebuglnF("Serving /emoncms.json page...");
		String response = WEBCLIENT.build_emoncms_json();

		this->send(200, "text/json", response);
		//Debugln(response);
		//Debugln(F("Ok!"));
		yield();  //Let a chance to other threads to work
	});

	//*******************************confJSONTable***************************************
	this->on("/config.json", [&]() {
		String response = "";
		//ESP.wdtFeed();  //Force software watchdog to restart from 0
		this->getConfJSONData(response);
		// Just to debug where we are
		DebuglnF("Serving /config page...");
		this->send(200, "text/json", response);
		Debugln(F("getConfJSONData Ok!"));
		yield();  //Let a chance to other threads to work
	});
	//*******************************spiffsJSONTable***************************************
	this->on("/spiffs.json", [&]() {
		String response = "";
		//ESP.wdtFeed();  //Force software watchdog to restart from 0
		this->getSpiffsJSONData(response);
		this->send(200, "text/json", response);
		yield();  //Let a chance to other threads to work
	});
	//*********************************wifiScanJSON*************************************
	this->on("/wifiscan.json", [&]() {
		String response = "";
		bool first = true;

		// Just to debug where we are
		DebuglnF("Serving /wifiscan page...");

		int n = WiFi.scanNetworks();

		// Json start
		response += F("[\r\n");

		for (uint8_t i = 0; i < n; ++i)
		{
			int8_t rssi = WiFi.RSSI(i);

			//uint8_t percent = 0;

			//// dBm to Quality
			//if (rssi <= -100)
			//	percent = 0;
			//else if (rssi >= -50)
			//	percent = 100;
			//else
			//	percent = 2 * (rssi + 100);

			if (first)
				first = false;
			else
				response += F(",");

			response += F("{\"ssid\":\"");
			response += WiFi.SSID(i);
			response += F("\",\"rssi\":");
			response += rssi;
			response += FPSTR(FP_JSON_END);
		}

		// Json end
		response += FPSTR("]\r\n");

		DebugF("sending...");
		this->send(200, "text/json", response);
		DebuglnF("Ok!");
		yield();  //Let a chance to other threads to work
	});
	//*****************************handleFactoryReset*****************************************
	this->on("/factory_reset", [&]() {
		// Just to debug where we are
		DebuglnF("Serving /factory_reset page...");
		CONFIGURATION.ResetConfig();
		ESP.eraseConfig();
		DebuglnF("sending...");
		this->send(200, "text/plain", FPSTR(FP_RESTART));
		DebuglnF("ResetConfig Ok!");
		delay(1000);
		ESP.restart();
		while (true)
			delay(1);
	});
	//****************************handleReset******************************************
	this->on("/reset", [&]() {
		// Just to debug where we are
		DebuglnF("Serving /reset page...");
		DebuglnF("sending...");
		this->send(200, "text/plain", FPSTR(FP_RESTART));
		DebuglnF("Ok!");
		delay(1000);
		ESP.restart();
		while (true)
			delay(1);
	});
	//****************************heartbeat******************************************
	this->on("/hb.htm", HTTP_GET, [&]() {
		this->sendHeader("Connection", "close");
		this->sendHeader("Access-Control-Allow-Origin", "*");
		this->send(200, "text/html", R"(OK)");
	});
	//******************************update****************************************
	this->on("/update", HTTP_POST,
		// handler once file upload finishes
		[&]() {
		this->sendHeader("Connection", "close");
		this->sendHeader("Access-Control-Allow-Origin", "*");
		this->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
		ESP.restart();
	},
		// handler for upload, get's the sketch bytes, 
		// and writes them through the Update object
		[&]() {
		HTTPUpload& upload = this->upload();
		char  buffer[132];
		if (upload.status == UPLOAD_FILE_START) {
			uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
			DebugF("ESP.getFreeSketchSpace() = ");Debugln(ESP.getFreeSketchSpace());
			DebugF("maxSketchSpace = ");Debugln(maxSketchSpace);

			WiFiUDP::stopAll();
			
			sprintf(buffer, "Update: %s\n", upload.filename.c_str());
			Debug(buffer);



			MYOTA.setOta_blink();

			//ajout pour spiffs
			int command = U_FLASH;
			if (strstr(upload.filename.c_str(), "spiffs"))
			{
				command = U_SPIFFS;
				Debugln("command = U_SPIFFS");
			}
			else
				Debugln("command = U_FLASH");



			if (!Update.begin(maxSketchSpace, command))
			{
				Update.printError(Serial);			//Serial1
				DebugF("Erreur maxSketchSpace = ");	Debugln(maxSketchSpace);
			}

		}
		else if (upload.status == UPLOAD_FILE_WRITE) {

			if (MYOTA.getOta_blink()) {
				LESLEDS.LedRGBON(COLOR_MAGENTA);
			}
			else {
				LESLEDS.LedRGBOFF();
			}
			MYOTA.notOta_blink();
			//ota_blink = !ota_blink;

			DebugF(".");
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
				Update.printError(Serial);			//Serial1

		}
		else if (upload.status == UPLOAD_FILE_END) {
			//true to set the size to the current progress
			if (Update.end(true)) {
				sprintf(buffer, "Update Success: %u\nRebooting...\n", upload.totalSize);
				Debug(buffer);
			}
			else
				Update.printError(Serial);			//Serial1

			LESLEDS.LedRGBOFF();

		}
		else if (upload.status == UPLOAD_FILE_ABORTED) {
			DebuglnF("avant Update.end()");
			Update.end();

			LESLEDS.LedRGBOFF();

			DebuglnF("Update was aborted");
		}
		delay(0);
	});
	//****************************handleNotFound******************************************
	this->onNotFound([&]() {
		String response = "";
		boolean found = false;
		char  buffer[132];

		LedBluON();

		// try to return SPIFFS file
		found = handleFileRead(this->uri());

		// Try Teleinfo ETIQUETTE
		if (!found) {
			// We check for an known label
			ValueList * me = TINFO.getList();
			const char * uri;
			// convert uri to char * for compare
			uri = this->uri().c_str();

			sprintf(buffer, "handleNotFound(%s)\r\n", uri);
			Debug(buffer);

			// Got at least one and consistent URI ?
			if (me && uri && *uri == '/' && *++uri) {

				// Loop thru the linked list of values
				while (me->next && !found) {

					// go to next node
					me = me->next;

					//Debugf("compare to '%s' ", me->name);
					// Do we have this one ?
					if (strcmp(me->name, uri) == 0)
					{
						// no need to continue
						found = true;

						// Add to respone
						response += F("{\"");
						response += me->name;
						response += F("\":");
						formatNumberJSON(response, me->value);
						response += F("}\r\n");
					}
				}
			}

			// Got it, send json
			if (found)
				this->send(200, "text/json", response);
		}

		// All trys failed
		if (!found) {
			// send error message in plain text
			String message = F("File Not Found\n\n");
			message += F("URI: ");
			message += this->uri();
			message += F("\nMethod: ");
			message += (this->method() == HTTP_GET) ? "GET" : "POST";
			message += F("\nArguments: ");
			message += this->args();
			message += FPSTR(FP_NL);

			for (uint8_t i = 0; i < this->args(); i++) {
				message += " " + this->argName(i) + ": " + this->arg(i) + FPSTR(FP_NL);
			}

			this->send(404, "text/plain", message);
		}
		LedBluOFF();
	});
}	//FIN DU CONSTRUCTEUR

void webServer::initServeur(void)	//myServer::
{

	initOptVal();

// serves all SPIFFS Web file with 24hr max-age control
// to avoid multiple requests to ESP
this->serveStatic("/font", SPIFFS, "/font", "max-age=86400");
this->serveStatic("/js", SPIFFS, "/js", "max-age=86400");
this->serveStatic("/css", SPIFFS, "/css", "max-age=86400");
this->begin(80);	//

}
void webServer::initSpiffs(void)	//
{
	char  buffer[132];
//*****************************************SPIFFS****************************************
  // Init SPIFFS filesystem, to use web server static files
  if (!SPIFFS.begin())
  {
	  // Serious problem
	  DebuglnF("SPIFFS Mount failed !");
  }
  else
  {
	  DebuglnF("");
	  DebuglnF("SPIFFS Mount succesfull");

	  Dir dir = SPIFFS.openDir("/");
	  while (dir.next()) {
		  String fileName = dir.fileName();
		  size_t fileSize = dir.fileSize();
		  sprintf(buffer, "FS File: %s, size: %d\n", fileName.c_str(), fileSize);
		  Debug(buffer);
	  }
  }
  //*****************************************fin SPIFFS****************************************
}

/* ======================================================================
Function: handleFileRead
Purpose : return content of a file stored on SPIFFS file system
Input   : file path
Output  : true if file found and sent
Comments: -
====================================================================== */
bool webServer::handleFileRead(String path) {
	if (path.endsWith("/"))
		path += "index.htm";

	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";

	DebugF("handleFileRead ");Debug(path);

	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz)) {
			path += ".gz";
			DebugF(".gz");
		}

		DebuglnF(" found on FS");

		File file = SPIFFS.open(path, "r");
		//size_t sent = 
		this->streamFile(file, contentType);
		file.close();
		return true;
	}

	DebuglnF("");

	this->send(404, "text/plain", "File Not Found");
	return false;
}


/* ======================================================================
Function: formatNumberJSON
Purpose : check if data value is full number and send correct JSON format
Input   : String where to add response
		  char * value to check
Output  : -
Comments: 00150 => 150
		  ADCO  => "ADCO"
		  1     => 1
====================================================================== */
void webServer::formatNumberJSON(String &response, char * value)
{
	// we have at least something ?
	if (value && strlen(value))
	{
		boolean isNumber = true;
		//    uint8_t c;
		char * p = value;

		// just to be sure
		if (strlen(p) <= 16) {
			// check if value is number
			while (*p && isNumber) {
				if (*p < '0' || *p > '9')
					isNumber = false;
				p++;
			}

			// this will add "" on not number values
			if (!isNumber) {
				response += '\"';
				response += value;
				response += F("\"");
			}
			else {
				// this will remove leading zero on numbers
				p = value;
				while (*p == '0' && *(p + 1))
					p++;
				response += p;
			}
		}
		else {
			Debugln(F("formatNumberJSON error!"));
		}
	}
}




/* ======================================================================
Function: formatSize
Purpose : format a asize to human readable format
Input   : size
Output  : formated string
Comments: -
====================================================================== */
String webServer::formatSize(size_t bytes)
{
	if (bytes < 1024) {
		return String(bytes) + F(" Byte");
	}
	else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + F(" KB");
	}
	else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + F(" MB");
	}
	else {
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + F(" GB");
	}
}

/* ======================================================================
Function: getContentType
Purpose : return correct mime content type depending on file extension
Input   : -
Output  : Mime content type
Comments: -
====================================================================== */
String webServer::getContentType(String filename) {
	if (filename.endsWith(".htm")) return F("text/html");
	else if (filename.endsWith(".html")) return F("text/html");
	else if (filename.endsWith(".css")) return F("text/css");
	else if (filename.endsWith(".json")) return F("text/json");
	else if (filename.endsWith(".js")) return F("application/javascript");
	else if (filename.endsWith(".png")) return F("image/png");
	else if (filename.endsWith(".gif")) return F("image/gif");
	else if (filename.endsWith(".jpg")) return F("image/jpeg");
	else if (filename.endsWith(".ico")) return F("image/x-icon");
	else if (filename.endsWith(".xml")) return F("text/xml");
	else if (filename.endsWith(".pdf")) return F("application/x-pdf");
	else if (filename.endsWith(".zip")) return F("application/x-zip");
	else if (filename.endsWith(".gz")) return F("application/x-gzip");
	else if (filename.endsWith(".otf")) return F("application/x-font-opentype");
	else if (filename.endsWith(".eot")) return F("application/vnd.ms-fontobject");
	else if (filename.endsWith(".svg")) return F("image/svg+xml");
	else if (filename.endsWith(".woff")) return F("application/x-font-woff");
	else if (filename.endsWith(".woff2")) return F("application/x-font-woff2");
	else if (filename.endsWith(".ttf")) return F("application/x-font-ttf");
	return "text/plain";
}


/* ======================================================================
Function: getSysJSONData
Purpose : Return JSON string containing system data
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void webServer::getSysJSONData(String & response)
{
	response = "";
	char buffer[32];
	//int32_t adc =  (1000 * analogRead(A0) / 1024);

	 // Json start
	response += F("[\r\n");

	response += "{\"na\":\"Uptime\",\"va\":\"";
	response += NTP.sysinfo.sys_uptime;
	response += "\"},\r\n";

#ifdef SENSOR
	response += "{\"na\":\"Switch\",\"va\":\"";
	if (SwitchState)
		response += F("Open");  //switch ouvert
	else
		response += F("Closed");  //switch fermé

	response += "\"},\r\n";
#endif

	if (WiFi.status() == WL_CONNECTED)
	{
		response += "{\"na\":\"Wifi RSSI\",\"va\":\"";
		response += WiFi.RSSI();
		response += " dB\"},\r\n";
		response += "{\"na\":\"Wifi network\",\"va\":\"";
		response += CONFIGURATION.config.ssid;
		response += "\"},\r\n";
		uint8_t mac[] = { 0, 0, 0, 0, 0, 0 };
		uint8_t* macread = WiFi.macAddress(mac);
		char macaddress[20];
		sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
		response += "{\"na\":\"Adresse MAC station\",\"va\":\"";
		response += macaddress;
		response += "\"},\r\n";
	}
	response += "{\"na\":\"Nb reconnexions Wifi\",\"va\":\"";
	response += WIFI.getNb_reconnect();
	response += "\"},\r\n";

	response += "{\"na\":\"Alterations Data detectees\",\"va\":\"";
	response += nb_reinit;
	response += "\"},\r\n";

	response += "{\"na\":\"WifInfo Version\",\"va\":\"" WIFINFO_VERSION "\"},\r\n";

	response += "{\"na\":\"Compile le\",\"va\":\"" __DATE__ " " __TIME__ "\"},\r\n";

	response += "{\"na\":\"Options de compilation\",\"va\":\"";
	response += optval;
	response += "\"},\r\n";

	response += "{\"na\":\"SDK Version\",\"va\":\"";
	response += system_get_sdk_version();
	response += "\"},\r\n";

	response += "{\"na\":\"Chip ID\",\"va\":\"";
	sprintf_P(buffer, "0x%0X", system_get_chip_id());
	response += buffer;
	response += "\"},\r\n";

	response += "{\"na\":\"Boot Version\",\"va\":\"";
	sprintf_P(buffer, "0x%0X", system_get_boot_version());
	response += buffer;
	response += "\"},\r\n";

	response += "{\"na\":\"Flash Real Size\",\"va\":\"";
	response += formatSize(ESP.getFlashChipRealSize());
	response += "\"},\r\n";

	response += "{\"na\":\"Firmware Size\",\"va\":\"";
	response += formatSize(ESP.getSketchSize());
	response += "\"},\r\n";

	response += "{\"na\":\"Free Size\",\"va\":\"";
	response += formatSize(ESP.getFreeSketchSpace());
	response += "\"},\r\n";

	response += "{\"na\":\"Analog\",\"va\":\"";
	//adc = ((1000 * analogRead(A0)) / 1024);
	//adc = ESP.getVcc();  //pas juste du au pont 220/100k  d'après internet......
	sprintf_P(buffer, PSTR("%d mV"), (1000 * analogRead(A0) / 1024));
	response += buffer;
	response += "\"},\r\n";

	FSInfo info;
	SPIFFS.info(info);

	response += "{\"na\":\"SPIFFS Total\",\"va\":\"";
	response += formatSize(info.totalBytes);
	response += "\"},\r\n";

	response += "{\"na\":\"SPIFFS Used\",\"va\":\"";
	response += formatSize(info.usedBytes);
	response += "\"},\r\n";

	response += "{\"na\":\"SPIFFS Occupation\",\"va\":\"";
	sprintf_P(buffer, "%d%%", 100 * info.usedBytes / info.totalBytes);
	response += buffer;
	response += "\"},\r\n";

	// Free mem should be last one 
	response += "{\"na\":\"Free Ram\",\"va\":\"";
	response += formatSize(system_get_free_heap_size());
	response += "\"}\r\n"; // Last don't have comma at end

	// 

	// Json end
	response += F("]\r\n");
}

/* ======================================================================
Function: getConfigJSONData
Purpose : Return JSON string containing configuration data
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void webServer::getConfJSONData(String & r)
{
	// Json start
	r = FPSTR(FP_JSON_START);

	r += "\"";
	r += CFG_FORM_SSID;      r += FPSTR(FP_QCQ); r += CONFIGURATION.config.ssid;           r += FPSTR(FP_QCNL);
	r += CFG_FORM_PSK;       r += FPSTR(FP_QCQ); r += CONFIGURATION.config.psk;            r += FPSTR(FP_QCNL);
	r += CFG_FORM_HOST;      r += FPSTR(FP_QCQ); r += CONFIGURATION.config.host;           r += FPSTR(FP_QCNL);
	r += CFG_FORM_AP_PSK;    r += FPSTR(FP_QCQ); r += CONFIGURATION.config.ap_psk;         r += FPSTR(FP_QCNL);
	r += CFG_FORM_EMON_HOST; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.emoncms.host;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_EMON_PORT; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.emoncms.port;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_EMON_URL;  r += FPSTR(FP_QCQ); r += CONFIGURATION.config.emoncms.url;    r += FPSTR(FP_QCNL);
	r += CFG_FORM_EMON_KEY;  r += FPSTR(FP_QCQ); r += CONFIGURATION.config.emoncms.apikey; r += FPSTR(FP_QCNL);
	r += CFG_FORM_EMON_NODE; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.emoncms.node;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_EMON_FREQ; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.emoncms.freq;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_OTA_AUTH;  r += FPSTR(FP_QCQ); r += CONFIGURATION.config.ota_auth;       r += FPSTR(FP_QCNL);
	r += CFG_FORM_OTA_PORT;  r += FPSTR(FP_QCQ); r += CONFIGURATION.config.ota_port;       r += FPSTR(FP_QCNL);
	r += CFG_FORM_SYSLOG_HOST; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.syslog_host;  r += FPSTR(FP_QCNL);
	r += CFG_FORM_SYSLOG_PORT; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.syslog_port;  r += FPSTR(FP_QCNL);
	r += CFG_FORM_JDOM_HOST; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.jeedom.host;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_JDOM_PORT; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.jeedom.port;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_JDOM_URL;  r += FPSTR(FP_QCQ); r += CONFIGURATION.config.jeedom.url;    r += FPSTR(FP_QCNL);
	r += CFG_FORM_JDOM_KEY;  r += FPSTR(FP_QCQ); r += CONFIGURATION.config.jeedom.apikey; r += FPSTR(FP_QCNL);
	r += CFG_FORM_JDOM_ADCO; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.jeedom.adco;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_JDOM_FREQ; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.jeedom.freq;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_HTTPREQ_HOST; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.httpReq.host;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_HTTPREQ_PORT; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.httpReq.port;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_HTTPREQ_PATH; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.httpReq.path;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_HTTPREQ_FREQ; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.httpReq.freq;   r += FPSTR(FP_QCNL);
	r += CFG_FORM_HTTPREQ_SWIDX; r += FPSTR(FP_QCQ); r += CONFIGURATION.config.httpReq.swidx;
	r += F("\"");
	// Json end
	r += FPSTR(FP_JSON_END);

}

/* ======================================================================
Function: getSpiffsJSONData
Purpose : Return JSON string containing list of SPIFFS files
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void webServer::getSpiffsJSONData(String & response)
{
	//  char buffer[32];
	bool first_item = true;

	// Json start
	response = FPSTR(FP_JSON_START);

	// Files Array  
	response += F("\"files\":[\r\n");

	// Loop trough all files
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String fileName = dir.fileName();
		size_t fileSize = dir.fileSize();
		if (first_item)
			first_item = false;
		else
			response += ",";

		response += F("{\"na\":\"");
		response += fileName.c_str();
		response += F("\",\"va\":\"");
		response += fileSize;
		response += F("\"}\r\n");
	}
	response += F("],\r\n");


	// SPIFFS File system array
	response += F("\"spiffs\":[\r\n{");

	// Get SPIFFS File system informations
	FSInfo info;
	SPIFFS.info(info);
	response += F("\"Total\":");
	response += info.totalBytes;
	response += F(", \"Used\":");
	response += info.usedBytes;
	response += F(", \"ram\":");
	response += system_get_free_heap_size();
	response += F("}\r\n]");

	// Json end
	response += FPSTR(FP_JSON_END);
}

void webServer::initOptVal(void)
{

	//********************************Options de compilation pour page html***************************************  
	memset(optval, 0, 80);

#ifdef DEBUGSERIAL
	strcat(optval, "DEBUGSERIAL, "); //13
#else
	strcat(optval, ", ");
#endif

#ifdef SYSLOG
	strcat(optval, "SYSLOG, "); //8
#else
	strcat(optval, ", ");
#endif

#ifdef SIMUTRAMETEMPO
	strcat(optval, "SIMUTRA, ");	//9
#else
	strcat(optval, ", ");
#endif
#ifdef AVEC_NTP
	strcat(optval, "NTP, ");	//5
#else
	strcat(optval, ", ");
#endif

#ifdef IPSTATIC
	strcat(optval, "IPSTATIC, ");	//10
#else
	strcat(optval, ", ");
#endif
#ifdef MODE_HISTORIQUE
	strcat(optval, "MODE_HISTO, ");	//12
#else
	strcat(optval, ", ");
#endif
#ifdef TELEINFO_RXD2
	strcat(optval, "Tinfo RXD2");	//10
#else
	strcat(optval, "Tinfo RXD0");
#endif
	//********************************fin Options de compilation pour page html*************************************** 
	Debugln(F("=============="));
	DebugF("WifInfo V");DebuglnF(WIFINFO_VERSION);
	DebugF("Options : ");Debugln(optval);
	Debugln();
}


void webServer::incNb_reinit(void)
{
	nb_reinit++;
}

