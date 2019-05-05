// **********************************************************************************
// ESP8266 Teleinfo WEB Client, web client functions
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
//		-V2.0.2:mise en place TAILLEBUFEMONCMS modidié les messages de httpPost
//             :dans build_emoncms_json test si la liste est vide sinon plantage si on a jamais recu de trame.
//		-V2.0.3 modification pour affichage DEMAIN sur emoncms passé de chaine à numériqueà préciser
// Using library ESP8266HTTPClient version 1.1
//
// ************************************************************************************************************
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include "Wifinfo.h"
#include "mySyslog.h"
#include "LibTeleinfo.h"
#include "config.h"
#include "myNTP.h"
#include "simuTempo.h"
#include "webclient.h"

webClient::webClient(boolean modeLinkyHistorique)
{
	this->modeLinkyHistorique = modeLinkyHistorique;
}


/* ======================================================================
Function: httpPost
Purpose : Do a http post
Input   : hostname
          port
          url
Output  : true if received 200 OK
Comments: -
====================================================================== */
boolean webClient::httpPost(char * host, uint16_t port, char * url)
{
  HTTPClient http;
  bool ret = false;

  unsigned long start = millis();

  // configure traged server and url
  http.begin(host, port, url); 
  //http.begin("http://emoncms.org/input/post.json?node=20&apikey=2f13e4608d411d20354485f72747de7b&json={PAPP:100}");
  //http.begin("emoncms.org", 80, "/input/post.json?node=20&apikey=2f13e4608d411d20354485f72747de7b&json={}"); //HTTP
  String errorMes;
  char  buffer[TAILLEBUFEMONCMS];  //marc 132-->300
  if (strlen(url)+ CFG_EMON_HOST_SIZE+27 < TAILLEBUFEMONCMS)
  {
	  sprintf(buffer, "http%s://%s:%d%s => ", port == 443 ? "s" : "", host, port, url);
	  Debug(buffer);
	  //?http.addHeader("Content-Type", "text/plain");
	  // start connection and send HTTP header
	  ESP.wdtFeed();  //Force software watchdog to restart from 0  ajout marc
	  int httpCode = http.GET();
	  if (httpCode) {
		  // HTTP header has been send and Server response header has been handled
		  //errorMes = "OK:httpCode:" + httpCode;
		  // file found at server
		  if (httpCode == 200) {
			  String payload = http.getString();
			  errorMes = "OK:payload:" + payload;
			  ret = true;
		  }
		  else
			  errorMes = "PB:httpCode:" + String(httpCode);
	  }
	  else {
		  errorMes = "PB:httpCode:" + String(httpCode);
	  }
  }
  else {
	  errorMes = "PB:TAILLEBUFEMONCMS trop petite" ;
  }
  Debugln(errorMes);

  sprintf(buffer," in %ld ms\r\n",millis()-start);
  Debugln(buffer);
  http.end();		//marc
  return ret;
}
/* ======================================================================
Function: build_emoncms_json string (usable by webserver.cpp)
Purpose : construct the json part of emoncms url
Input   : -
Output  : String if some Teleinfo data available
Comments: -
====================================================================== */
String webClient::build_emoncms_json(void)
{

  boolean first_item = true;
  ESP.wdtFeed();  //Force software watchdog to restart from 0  ajout marc
  String url = "{" ;

  ValueList * me = TINFO.getList();
  if (me) {
      // Loop thru the node
      while (me->next) {
       if(! first_item) 
          // go to next node
         me = me->next;
	   //marc
	   else
		   if (me->free) {
			   //1st item is free : empty list !
			   Debugln("Teleinfo list is empty !");
			   break;
		   }
	   //fin marc
        if( ! me->free ) {
                
             
            // On first item, do not add , separator
            if (first_item)
              first_item = false;
            else
              url += ",";
          

            if(validate_value_name(me->name)) {
              url +=  me->name ;
              url += ":" ;
      
              // EMONCMS ne sait traiter que des valeurs numériques, donc ici il faut faire une 
              // table de mappage, tout à fait arbitraire, mais c"est celle-ci dont je me sers 
              // depuis mes débuts avec la téléinfo
              if (!strcmp(me->name, "OPTARIF")) {
                // L'option tarifaire choisie (Groupe "OPTARIF") est codée sur 4 caractères alphanumériques 
                /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
                je mets le 4eme char à 0, trop de possibilités
                BASE => Option Base. 
                HC.. => Option Heures Creuses. 
                EJP. => Option EJP. 
                BBRx => Option Tempo
                */
                char * p = me->value;
                  
                     if (*p=='B'&&*(p+1)=='A'&&*(p+2)=='S') url += "1";
                else if (*p=='H'&&*(p+1)=='C'&&*(p+2)=='.') url += "2";
                else if (*p=='E'&&*(p+1)=='J'&&*(p+2)=='P') url += "3";
                else if (*p=='B'&&*(p+1)=='B'&&*(p+2)=='R') url += "4";
                else url +="0";
              } else if (!strcmp(me->name, "HHPHC")) {
                // L'horaire heures pleines/heures creuses (Groupe "HHPHC") est codé par un caractère A à Y 
                // J'ai choisi de prendre son code ASCII
                int code = *me->value;
                url += String(code);
              } else if (!strcmp(me->name, "PTEC")) {
                // La période tarifaire en cours (Groupe "PTEC"), est codée sur 4 caractères 
                /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
                TH.. => Toutes les Heures. 
                HC.. => Heures Creuses. 
                HP.. => Heures Pleines. 
                HN.. => Heures Normales. 
                PM.. => Heures de Pointe Mobile. 
                HCJB => Heures Creuses Jours Bleus. 
                HCJW => Heures Creuses Jours Blancs (White). 
                HCJR => Heures Creuses Jours Rouges. 
                HPJB => Heures Pleines Jours Bleus. 
                HPJW => Heures Pleines Jours Blancs (White). 
                HPJR => Heures Pleines Jours Rouges. 
                */
                     if (!strcmp(me->value, "TH..")) url += "1";
                else if (!strcmp(me->value, "HC..")) url += "2";
                else if (!strcmp(me->value, "HP..")) url += "3";
                else if (!strcmp(me->value, "HN..")) url += "4";
                else if (!strcmp(me->value, "PM..")) url += "5";
                else if (!strcmp(me->value, "HCJB")) url += "6";
                else if (!strcmp(me->value, "HCJW")) url += "7";
                else if (!strcmp(me->value, "HCJR")) url += "8";
                else if (!strcmp(me->value, "HPJB")) url += "9";
                else if (!strcmp(me->value, "HPJW")) url += "10";
                else if (!strcmp(me->value, "HPJR")) url += "11";
                else url +="0";
              } 
			  //ajout pour affichage sur emoncms à préciser numérique sinon chaine
			  //on pourrait supprimer demain, peu d'interet pour emoncms??
			  else if (!strcmp(me->name, "DEMAIN")) {
				  if (!strcmp(me->value, "BLEU"))
					url += ETAT_JOUR::ETAT_JOUR_BLEU;
				  else if (!strcmp(me->value, "BLAN"))
					url += ETAT_JOUR::ETAT_JOUR_BLANC;
				  else if (!strcmp(me->value, "ROUG"))
					url += ETAT_JOUR::ETAT_JOUR_ROUGE;
				  }
			  //fin ajout
			  else {
                url += me->value;
              }
            } else {
              //Value name not valid : ignore this value, and
              //  force Teleinfo to reinit on next loop !
				TINFO.setReinit();
            }
         } //not free entry
		//else
		//	 me = me->next;          //marc  si le premier est free
      } // While next

  } //if me
  // Json end
  url += "}";
      
  return url;
}

/* ======================================================================
Function: emoncmsPost (called by main sketch on timer, if activated)
Purpose : Do a http post to emoncms
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean webClient::emoncmsPost(void)
{
  boolean ret = false;
  // Some basic checking
  if (*CONFIGURATION.config.emoncms.host) {
    ValueList * me = TINFO.getList();
    // Got at least one ?
    if (me && me->next) {
      String url ; 
      

      url = *CONFIGURATION.config.emoncms.url ? CONFIGURATION.config.emoncms.url : "/";
      url += "?";
      if (CONFIGURATION.config.emoncms.node>0) {
        url+= F("node=");
        url+= String(CONFIGURATION.config.emoncms.node);
        url+= "&";
      } 

      url += F("apikey=") ;
      url += CONFIGURATION.config.emoncms.apikey;

      //append json list of values
      url += F("&json=") ;
      url += build_emoncms_json();  //Get Teleinfo list of values
      // And submit all to emoncms
      ret = httpPost(CONFIGURATION.config.emoncms.host, CONFIGURATION.config.emoncms.port, (char *) url.c_str()) ;
    } // if me
  } // if host
  return ret;
}

/* ======================================================================
Function: jeedomPost
Purpose : Do a http post to jeedom server
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean webClient::jeedomPost(void)
{
  boolean ret = false;

  // Some basic checking
  if (*CONFIGURATION.config.jeedom.host) {
    ValueList * me = TINFO.getList();
    // Got at least one ?
    if (me && me->next) {
      String url ; 
      boolean skip_item;

      url = *CONFIGURATION.config.jeedom.url ? CONFIGURATION.config.jeedom.url : "/";
      url += "?";

      // Config identifiant forcée ?
      if (*CONFIGURATION.config.jeedom.adco) {
        url+= F("ADCO=");
        url+= CONFIGURATION.config.jeedom.adco;
        url+= "&";
      } 

      url += F("api=") ;
      url += CONFIGURATION.config.jeedom.apikey;
      url += F("&") ;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;
        skip_item = false;

        // Si ADCO déjà renseigné, on le remet pas
        if (!strcmp(me->name, "ADCO")) {
          if (*CONFIGURATION.config.jeedom.adco)
            skip_item = true;
        }

        // Si Item virtuel, on le met pas
        if (*me->name =='_')
          skip_item = true;

        // On doit ajouter l'item ?
        if (!skip_item) {
          url +=  me->name ;
          url += "=" ;
          url +=  me->value;
          url += "&" ;
        }
      } // While me

      ret = httpPost(CONFIGURATION.config.jeedom.host, CONFIGURATION.config.jeedom.port, (char *) url.c_str()) ;
    } // if me
  } // if host
  return ret;
}

/* ======================================================================
Function: HTTP Request
Purpose : Do a http request
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean webClient::httpRequest(void)
{
  boolean ret = false;

  // Some basic checking
  if (*CONFIGURATION.config.httpReq.host)
  {
    ValueList * me = TINFO.getList();
    // Got at least one ?
    if (me && me->next)
    {
      String url ; 
      boolean skip_item;

      url = *CONFIGURATION.config.httpReq.path ? CONFIGURATION.config.httpReq.path : "/";
      url += "?";

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;
        skip_item = false;

        // Si Item virtuel, on le met pas
        if (*me->name =='_')
          skip_item = true;

        // On doit ajouter l'item ?
		if (!skip_item)
		{
			String valName = String(me->name);
			if (valName == "HCHP")
			{
				url.replace("%HCHP%", me->value);
			}
			if (valName == "HCHC")
			{
				url.replace("%HCHC%", me->value);
			}
			if (valName == "PAPP")
			{
				url.replace("%PAPP%", me->value);
			}
			if (valName == "ADCO")
			{
				url.replace("%ADCO%", me->value);
			}
			if (valName == "OPTARIF")
			{
				url.replace("%OPTARIF%", me->value);
			}
			if (valName == "ISOUC")
			{
				url.replace("%ISOUC%", me->value);
			}
			if (valName == "PTEC")
			{
				url.replace("%PTEC%", me->value);
			}
			if (valName == "IINST")
			{
				url.replace("%IINST%", me->value);
			}
			if (valName == "IMAX")
			{
				url.replace("%IMAX%", me->value);
			}
			if (valName == "HHPHC")
			{
				url.replace("%HHPHC%", me->value);
			}
			if (valName == "MOTDETAT")
			{
				url.replace("%MOTDETAT%", me->value);
			}
			if (valName == "BASE")
			{
				url.replace("%BASE%", me->value);
			}
			if (!this->modeLinkyHistorique)
			{
				if (valName == "SINSTS") {
					url.replace("%SINSTS%", String(atol(me->value)));//Puissance app. Instantanée soutirée
				}
				if (valName == "EAST") {
					url.replace("%EAST%", String(atol(me->value)));	//Energie active soutirée totale
				}
				if (valName == "EASF01") {
					url.replace("%EASF01%", String(atol(me->value)));//Energie active soutirée Fournisseur, index 01
				}
				if (valName == "EASF02") {
					url.replace("%EASF02%", String(atol(me->value)));//Energie active soutirée Fournisseur, index 02
				}
			}
	      }
      } // While me

      ret = httpPost(CONFIGURATION.config.httpReq.host, CONFIGURATION.config.httpReq.port, (char *) url.c_str()) ;
    } // if me
  } // if host
  return ret;
}
#ifdef SENSOR
/* ======================================================================
Function: UPD_switch
Purpose : Do a http request to update Switch state into Domoticz
Input   :
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean UPD_switch(void)
{
	boolean ret = false;

	// Some basic checking
	if (*config.httpReq.host && (config.httpReq.swidx != 0))
	{
		char url[128];
		char State[5];
		uint16_t port = config.httpReq.port;

		if (port == 0)
			port = 80;

		if (SwitchState)
			sprintf(State, "Off");  //switch ouvert
		else
			sprintf(State, "On");   //switch fermé : portail fermé 

		sprintf(url, "/json.htm?type=command&param=switchlight&idx=%d&switchcmd=%s", (int)config.httpReq.swidx, State);
		//Debugf("Updating switch: <%s>\n",  url );
		ret = httpPost(config.httpReq.host, port, url);

	} // if host & idx
	return ret;
}
#endif
/* ======================================================================
Function: validate_value_name
Purpose : check if value name is in known range of values....
Input   : name to check
Output  : true if OK, false otherwise
Comments: -
====================================================================== */
bool webClient::validate_value_name(String name)
{

	for (unsigned int i = 0; i < TAILLETABNAMES; i++) {
		if ((tabnames[i].length() == name.length()) && (tabnames[i] == name)) {
			return true;
		}
	}
	return false; //Not an existing name !
}



