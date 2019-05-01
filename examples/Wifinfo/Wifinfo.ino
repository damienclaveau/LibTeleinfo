// **********************************************************************************
// ESP8266 Teleinfo WEB Server
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
//       Version 1.0.5
//       Librairie LibTeleInfo : Allocation statique d'un tableau de stockage 
//           des variables (50 entrées) afin de proscrire les malloc/free
//           pour éviter les altérations des noms & valeurs
//       Modification en conséquence des séquences de scanning du tableau
//       ATTENTION : Nécessite probablement un ESP-8266 type Wemos D1,
//        car les variables globales occupent 42.284 octets
//
//       Version 1.0.5a (11/01/2018)
//       Permettre la mise à jour OTA à partir de fichiers .ino.bin (Auduino IDE 1.8.3)
//       Ajout de la gestion d'un switch (Contact sec) relié à GND et D5 (GPIO-14)
//          Décommenter le #define SENSOR dans Wifinfo.h
//          Pour être utilisable avec Domoticz, au moins l'URL du serveur et le port
//          doivent être renseignés dans la configuration HTTP Request, ainsi que 
//          l'index du switch (déclaré dans Domoticz)
//          L'état du switch (On/Off) est envoyé à Domoticz au boot, et à chaque
//            changement d'état
//       Note : Nécessité de flasher le SPIFFS pour pouvoir configurer l'IDX du switch
//              et flasher le sketch winfinfo.ino.bin via interface Web
//       Rendre possible la compilation si define SENSOR en commentaire
//              et DEFINE_DEBUG en commentaire (aucun debug, version Production...)
//
//       Version 1.0.6 (04/02/2018) Branche 'syslog' du github
//		      Ajout de la fonctionnalité 'Remote Syslog'
//		        Pour utiliser un serveur du réseau comme collecteur des messages Debug
//            Note : Nécessité de flasher le SPIFFS pour pouvoir configurer le remote syslog
//          Affichage des options de compilation sélectionnées dans l'onglet 'Système'
//            et au début du Debug + syslog éventuels
// **********************************************************************************
// Modifié par marc PRIEUR 2019-03-21
//		2019/05/01 ajout de la classe myTinfo
//###################################includes##################################  

//version wifinfo syslog d'origine:352 392 bytes

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#include "Wifinfo.h"
#include "myNTP.h"
#include "mySyslog.h"
#include "myOTA.h"


#include "simuTempo.h"
#include "LibTeleinfo.h"

#include "leds.h"
#include "webServer.h"
#include "webclient.h"
#include "config.h"
#include "myWifi.h"
#include "myTinfo.h"

//###################################création des objets##################################  

TInfo TINFO;
myTinfo MYTINFO;
lesLeds LESLEDS;
#ifdef SIMUTRAMETEMPO
	SimuTempo SIMU_TEMPO;
#endif 

Ticker Tick_emoncms;
Ticker Tick_jeedom;
Ticker Tick_httpRequest;

myntp NTP; 
 
myOTA MYOTA;
webServer WEBSERVER;
configuration CONFIGURATION;
#ifdef SYSLOG
mySyslog MYSYSLOG;
#endif
myWifi WIFI;
#ifdef MODE_HISTORIQUE
	webClient WEBCLIENT(true);
#else
	webClient WEBCLIENT(false);
#endif

//###################################SETUP##################################  
/* ======================================================================
Function: setup
Purpose : Setup I/O and other one time startup stuff
Input   : -
Output  : - 
Comments: -
====================================================================== */
void setup() {

  system_update_cpu_freq(160);

#ifdef SYSLOG
  MYSYSLOG.setSYSLOGselected();
  MYSYSLOG.clearLinesSyslog();
#endif

#ifdef DEBUGSERIAL
	   DEBUG_SERIAL.begin(115200);		//pour test recup SPI
  while (!Serial) {
	  ; // wait for serial port to connect. Needed for native USB port only
  }
#endif

  LESLEDS.InitLed();
  CONFIGURATION.initConfig();
  WIFI.WifiHandleConn(true);

#ifdef SYSLOG
  MYSYSLOG.sendSyslog();
#endif

  WEBSERVER.initSpiffs();

  NTP.UpdateSysinfo(true, true);

  WEBSERVER.initServeur();
 
  CONFIGURATION.showConfig();
  #ifdef  TELEINFO_RXD2 
    DebuglnF("Fin des traces consoles voir avec syslog,la teleinfo est recue sur RXD2");
	DebuglnF("Changement de vitesse des traces consoles voir avec syslog,la teleinfo est maintenant recu sur RXD0 a la place de l'USB(OU RXD2 si Serial.swap())");
  #else
    DebuglnF("Les traces consoles continue de fonctionner sur TXD0 à 1200 bauds ou VITESSE_SIMUTRAMETEMPO si SIMUTRAMETEMPO");
	DebuglnF("La teleinfo est recue sur RXD0");
  #endif
	#ifdef MODE_HISTORIQUE
		#ifdef SIMUTRAMETEMPO
			#ifdef TELEINFO_RXD2
				DebuglnF("Pour la simulation:strapper D4(TXD1) et D7(RXD2)");
			#else
				DebuglnF("Pour la simulation:strapper D4(TXD1) et D9(RXD0");
			#endif

			const int VITESSE_SIMUTRAMETEMPO = 115200;

			Serial.begin(VITESSE_SIMUTRAMETEMPO); 
			Serial.setRxBufferSize(1024);
		#else
			Serial.begin(1200, SERIAL_7E1);
			DebuglnF("Serial.begin");
		#endif
	#else
		Serial.begin(9600, SERIAL_7E1);       //5.3.5. Couche physique document enedis Enedis-NOI-CPT_54E.pdf 
	#endif
#ifdef TELEINFO_RXD2
	Serial.swap();  // reception teleinfo sur rxd2 sinon passe la reception teleinfo sur rx0 pour recuperer rx2 pour mosi
						//fonctionne correctement sur rx0, il faut juste penser à enlever le strap de la simulation si present
						//pour programmer ou debuguer via la console, pas de pb en OTA.
#endif

	#ifdef SIMUTRAMETEMPO
	  SIMU_TEMPO.initSimuTrameTempo();
	#endif

#ifdef SIMUTRAMETEMPO
	SerialSimu.begin(VITESSE_SIMUTRAMETEMPO);	//19200, SERIAL_7E1
#endif
#ifdef MODE_HISTORIQUE
  TINFO.init(true);
#else
  TINFO.init(false);
#endif
  MYTINFO.init();
  LESLEDS.LedRGBOFF();
  uint8_t timeout = 5;
  while (WIFINOOKOU && timeout)
  {
	  delay(500);
	  DebugF(".");
	  --timeout;
  }
#ifdef AVEC_NTP 
  NTP.init();
#endif

}


//###################################LOOP##################################  

/* ======================================================================
Function: loop
Purpose : infinite loop main code
Input   : -
Output  : - 
Comments: -
====================================================================== */

void loop()
{
static unsigned long start = 0;
static unsigned long duree = 0;
static unsigned long dureeMax = 0;

  start = millis();
 
//************************************************1 fois par seconde*************************************************************************************
  // Only once task per loop, let system do its own task
  if (NTP.getCycle1Seconde()) {
	  WEBSERVER.handleClient();
	  MYOTA.handle();

#ifdef AVEC_NTP
	  NTP.refreshTimeIfMidi();		//si midi et wifi :remise a l'heure
#endif
	  NTP.UpdateSysinfo(false, false);

	  WIFI.testWifi();

#ifdef SIMUTRAMETEMPO
  SIMU_TEMPO.traite1Trame(NTP.getSeconds1970());
#endif
DebugF("dureeMax:");
  Debugln((long)dureeMax);
  dureeMax = 0;

  }
  //*************************************************************************************************************************************
	else if (MYTINFO.getTask_emoncms()) { 
		WEBCLIENT.emoncmsPost(); 
  } else if (MYTINFO.getTask_jeedom()) {
		WEBCLIENT.jeedomPost();
  } else if (MYTINFO.getTask_httpRequest()) {
		WEBCLIENT.httpRequest();
  } 
	if (TINFO.getReinit())
	{
 		WEBSERVER.incNb_reinit();    //account of reinit operations, for system infos
#ifdef MODE_HISTORIQUE
		TINFO.init(true);//Clear ListValues, buffer, and wait for next STX
#else
		TINFO.init(false);
#endif
	} 
	else
	{
	  if ( Serial.available() )
	    { 
				char c = Serial.read();          //pour test recup SPI
				TINFO.process(c);
		}
	}
	unsigned long temp = millis();
	duree = temp - start;	
	if (duree > dureeMax)
  	  dureeMax = duree;
 }   //loop
