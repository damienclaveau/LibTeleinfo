# Teleinfo Universal Library
This is a fork of Teleinfo Universal Library for the ESP8266 MCU  
This is a generic Teleinfo French Meter Measure Library  
- Initial Github source : <https://github.com/hallard/LibTeleinfo>
- Modified Github source : <https://github.com/Doume/LibTeleinfo>

# Modifications par Doume (version 1.0.6) branche 'syslog' :

- Permettre l'envoi des messages de debugging à un serveur rsyslog du réseau local

   les paramètres peuvent être configurés via l'interface Web, onglet 'Configuration'
   panel 'Avancée'
   
   Il suffit de laisser le paramètre 'Syslog host' vide, pour désactiver cette fonction.
   Les envois sur le réseau utilisent le protocole UDP pour alimenter le serveur
   distant
   
   Pour compiler avec l'option SYSLOG, vous devrez installer la librairie Syslog-master.zip
   qui se trouve dans le répertoire 'librairie', dans votre environnement Arduino IDE
			
# Modifications par Doume (version 1.0.5a) branche 'static' :

- Ajout de la gestion d'un contact sec, dont l'état peut être remonté vers Domoticz
     exemple : le compteur EDF est souvent près du portail, donc on peut notifier
     			l'état ouvert/fermé du portail en utilisant Wifinfo
			
# Modifications par Doume (version 1.0.5) branche 'static' :

- Add support for request /emoncms.json

        return a json list formated to interface emoncms server from a 3rd party 
        That allow to don't activate emoncms http client inside Wifinfo module
	
- Add possibility to compile a version of sketch for module not connected to EDF counter

        Activate #define SIMU into Wifinfo.h, to obtain a version which will create
         2 variables and update one of them each second, to verify package
	 
- Add a check on variable names, and force a reinit of LibTeleinfo interface if an
  alteration is detected (each restart is counted, and displayed in 'system' page )
  
- Change library LibTeleinfo.cpp, to don't use anymore malloc/free system APIs

		Variables are now stored in static table, allocated on start
		
			50 entries max, name length 16 bytes max, value length 16 bytes max
			
		To use this library version :
		
			First, backup your old version of LibTeleinfo....
			
			copy files src/LibTeleinfo.cpp and src/LibTeleinfo.h into your Arduino
			environment (generally ~/Arduino/libraries/LibTeleinfo-master/src )
			before to compile sketch

# Added features :
- Add possibility to configure HttpRequest to send parameters/values to Domoticz
- Add all possible variable as listed below : 
- Add some informations to 'System' page, like Wifi link quality, Wifi network name, and MAC address

Ces différents messages donnent les indications suivantes en fonction de l’abonnement souscrit :
- N° d’identification du compteur : ADCO (12 caractères)
- Option tarifaire (type d’abonnement) : OPTARIF (4 car.)
- Intensité souscrite : ISOUSC ( 2 car. unité = ampères)
- Index si option = base : BASE ( 9 car. unité = Wh)
- Index heures creuses si option = heures creuses : HCHC ( 9 car. unité = Wh)
- Index heures pleines si option = heures creuses : HCHP ( 9 car. unité = Wh)
- Index heures normales si option = EJP : EJP HN ( 9 car. unité = Wh)
- Index heures de pointe mobile si option = EJP : EJP HPM ( 9 car. unité = Wh)
- Index heures creuses jours bleus si option = tempo : BBR HC JB ( 9 car. unité = Wh)
- Index heures pleines jours bleus si option = tempo : BBR HP JB ( 9 car. unité = Wh)
- Index heures creuses jours blancs si option = tempo : BBR HC JW ( 9 car. unité = Wh)
- Index heures pleines jours blancs si option = tempo : BBR HP JW ( 9 car. unité = Wh)
- Index heures creuses jours rouges si option = tempo : BBR HC JR ( 9 car. unité = Wh)
- Index heures pleines jours rouges si option = tempo : BBR HP JR ( 9 car. unité = Wh)
- Préavis EJP si option = EJP : PEJP ( 2 car.) 30mn avant période EJP
- Période tarifaire en cours : PTEC ( 4 car.)
- Couleur du lendemain si option = tempo : DEMAIN
- Intensité instantanée : IINST ( 3 car. unité = ampères)
- Avertissement de dépassement de puissance souscrite : ADPS ( 3 car. unité = ampères) (message émis uniquement en cas de dépassement effectif, dans ce cas il est immédiat)
- Intensité maximale : IMAX ( 3 car. unité = ampères)
- Puissance apparente : PAPP ( 5 car. unité = Volt.ampères)
- Groupe horaire si option = heures creuses ou tempo : HHPHC (1 car.)
- Mot d’état (autocontrôle) : MOTDETAT (6 car.)


Teleinfo Universal Library
This is a fork of Teleinfo Universal Library for the ESP8266 MCU
This is a generic Teleinfo French Meter Measure Library

Initial Github source : https://github.com/hallard/LibTeleinfo
Modified Github source : https://github.com/Doume/LibTeleinfo




# Modified Github source :
						https://github.com/marco402/LibTeleinfo

## version 2.0.0 réorganization of Wifinfo
Reorganization of the WifInfo example linked to the teleinfo library.

I undertook this work in order to use this software for various applications with or without a teleinfo link.

In a first phase I tried to keep the original features of WifInfo version with syslog.

However, I added (in parenthesis the compilation options):

-Time setting by NTP(AVEC_NTP).
-The teleinfo frame simulation, historic version (SIMUTRAMETEMPO).
-The possibility to receive the teleinfo on RXD0(TELEINFO_RXD2).

-I have not retested:
	-The leds outputs(leds.cpp and leds.h),I kept the original functions but I have another version with leds APA106.
	-The emoncms,jeedom and httpRequest outputs, functions that are original to the implementation of the classes.


## version 2.0.0
Réorganisation de l'exemple WifInfo lié à la bibliothèque teleinfo.

J'ai entrepris ce travail afin d'utiliser ce logiciel pour divers applications avec ou sans liaison teleinfo.

Dans une première phase j'ai essayé de conserver les fonctions d'origine de WifInfo version avec syslog.

Cependant, j’ai ajouté(entre parenthèse les options de compilation):
	-La mise à l'heure par NTP(AVEC_NTP).
	-La simulation de trame teleinfo version historique(SIMUTRAMETEMPO).
	-La possibilitée de recevoir la teleinfo sur RXD0(TELEINFO_RXD2).
	
-Je n'ai pas retesté:
	-Les sorties leds(leds.cpp et leds.h),j'ai conservé les fonctions d'origine mais j'ai une autre version avec des leds APA106.
	-Les sortie emoncms,jeedom et httpRequest, fonctions qui sont d'origine à la mise en place des classes près.

## version 2.0.1 correction checksum mode standard et ajout classe myTinfo
-Correction checksum.
-Addition of the class myTinfo(myTinfo.h et myTinfo.cpp)

## version 2.0.1
-Correction checksum.
-Ajout de la classe myTinfo(myTinfo.h et myTinfo.cpp)

## version 2.0.2
-Modification emoncms
	