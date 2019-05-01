// **********************************************************************************
// ESP8266 WifInfo WEB Server global Include file
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
//
//Using library Ticker version 1.0
//
// Modifié par marc Prieur 04/2019
//
// **********************************************************************************
#ifndef WIFINFO_H
#define WIFINFO_H

#include <Ticker.h>
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
 
// Décommenter SIMU pour compiler une version de test
//  pour un module non connecté au compteur EDF (simule un ADCO et une valeur HCHC)
// Le port Serial sera alors utilisé pour le DEBUG (accessible via USB pour l'IDE)
//#define SIMU

// Décommenter DEBUG pour une version capable d'afficher du Debug
//  soit sur Serial, soit sur Serial1 si compteur EDF raccordé sur Serial
// Attention : si SIMU n'est pas déclaré, le debug est envoyé sur Serial1
//  donc n'est pas visible au travers du port USB pour Arduino IDE !
//#define DEBUG

// Décommenter SYSLOG pour une version capable d'envoyer du Debug
//  vers un serveur rsyslog du réseau
#define SYSLOG			//messages syslog(sous windows-->visual syslog server)
//parametrer le port et l'adresse ip du serveur sur la page web
//avec visual syslog sous windows,paramétrer le parefeu

//Décommenter SENSOR pour compiler une  version capable de gérer
//  un contact sec connecté entre Ground et D5 (GPIO-14)
#define NO_SENSOR			//pas remis SENSOR dans wifinfo.ino, il faudrait générer une classe sensor. marc

/*  ===========================defines=================================== */
#define TELEINFO_RXD2			//teleinfo sur RXD2 sinon sur RXD0
#define DEBUGSERIAL				//DEBUGSERIAL debug vers TXD0
#define NO_SIMUTRAMETEMPO		//Version standard:simulation des trames tempo. strapper D4(TXD1) et D7(RXD2) ou D9(RXD0) suivant TELEINFO_RXD2
#define AVEC_NTP				//Serveur de temps
#define NO_IPSTATIC				//Essayer adresse hors plage DHCP(voir box) ---->OK mais plus de liaison avec l'extérieur(NTP...)il faudrait du  routage au niveau de la box!
#define MODE_HISTORIQUE			//Pour le linky sinon mode standard (mode standard incomplet)

#ifdef DEBUGSERIAL
	#define MACRO
	#define DEBUG_SERIAL  Serial
#else
	#define DEBUG_SERIAL  Serial1
#endif 

#define WIFINFO_VERSION "2.0.1"

#ifdef SYSLOG
	#define MACRO
	// Definit le client syslog
	#define APP_NAME "Wifinfo"
#endif

#define WIFIOK ((WiFi.status() == WL_CONNECTED) && (wifi_station_get_connect_status() == STATION_GOT_IP))
#define WIFINOOKET ((WiFi.status() != WL_CONNECTED) && (wifi_station_get_connect_status() != STATION_GOT_IP))
#define WIFINOOKOU ((WiFi.status() != WL_CONNECTED) || (wifi_station_get_connect_status() != STATION_GOT_IP))


extern Ticker Tick_emoncms;
extern Ticker Tick_jeedom;
extern Ticker Tick_httpRequest;

// Exported function located in main sketch
void Task_emoncms();
void Task_jeedom();
void Task_httpRequest();

#endif
