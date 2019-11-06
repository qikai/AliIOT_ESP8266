/*
 * mqtt_config.h
 *
 *  Created on: 2019年11月2日
 *      Author: qikai
 */

#ifndef APP_INCLUDE_DRIVER_MQTT_CONFIG_H_
#define APP_INCLUDE_DRIVER_MQTT_CONFIG_H_



#if defined(MQTT_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif


typedef enum{
  NO_TLS = 0,                       // 0: disable SSL/TLS, there must be no certificate verify between MQTT server and ESP8266
  TLS_WITHOUT_AUTHENTICATION = 1,   // 1: enable SSL/TLS, but there is no a certificate verify
  ONE_WAY_ANTHENTICATION = 2,       // 2: enable SSL/TLS, ESP8266 would verify the SSL server certificate at the same time
  TWO_WAY_ANTHENTICATION = 3,       // 3: enable SSL/TLS, ESP8266 would verify the SSL server certificate and SSL server would verify ESP8266 certificate
}TLS_LEVEL;


/*IMPORTANT: the following configuration maybe need modified*/
/***********************************************************************************************************************************************************************************************************************************************************/
#define CFG_HOLDER    		0x66666663	// ³ÖÓÐÈË±êÊ¶(Ö»ÓÐ¸üÐÂ´ËÊýÖµ£¬ÏµÍ³²ÎÊý²Å»á¸üÐÂ)		/* Change this value to load default configurations */

/*DEFAULT CONFIGURATIONS*/
// ×¢£º¡¾MQTTÐ­Òé¹æ¶¨£ºÁ¬½Ó·þÎñ¶ËµÄÃ¿¸ö¿Í»§¶Ë¶¼±ØÐëÓÐÎ¨Ò»µÄ¿Í»§¶Ë±êÊ¶·û£¨ClientId£©¡¿¡£Èç¹ûÁ½ÏàÍ¬IDµÄ¿Í»§¶Ë²»¶ÏÖØÁ¬£¬¾Í»á½øÈë»¥ÌßËÀÑ­»·
//--------------------------------------------------------------------------------------------------------------------------------------
#define MQTT_HOST			"a18BP50eRCH.iot-as-mqtt.cn-shanghai.aliyuncs.com" 		// MQTT·þÎñ¶ËÓòÃû/IPµØÖ·	// the IP address or domain name of your MQTT server or MQTT broker ,such as "mqtt.yourdomain.com"
#define MQTT_PORT       	1883    										// ÍøÂçÁ¬½Ó¶Ë¿ÚºÅ			// the listening port of your MQTT server or MQTT broker
#define MQTT_CLIENT_ID   	"123456|securemode=3,signmethod=hmacsha1|"	// ¹Ù·½Àý³ÌÖÐÊÇ"Device_ID"		// ¿Í»§¶Ë±êÊ¶·û				// the ID of yourself, any string is OK,client would use this ID register itself to MQTT server
#define MQTT_USER        	"Esp8266Test&a18BP50eRCH" 			// MQTTÓÃ»§Ãû				// your MQTT login name, if MQTT server allow anonymous login,any string is OK, otherwise, please input valid login name which you had registered
#define MQTT_PASS        	"A23E8A046FB4C9ACF725095EE473550D9F7F4C3F" 	// MQTTÃÜÂë					// you MQTT login password, same as above

#define STA_SSID 			"ChinaNet-mKJx"    	// WIFIÃû³Æ					// your AP/router SSID to config your device networking
#define STA_PASS 			"zfahwudk" 	// WIFIÃÜÂë					// your AP/router password
#define STA_TYPE			AUTH_WPA2_PSK

#define DEFAULT_SECURITY	NO_TLS      		// ¼ÓÃÜ´«ÊäÀàÐÍ¡¾Ä¬ÈÏ²»¼ÓÃÜ¡¿	// very important: you must config DEFAULT_SECURITY for SSL/TLS

#define CA_CERT_FLASH_ADDRESS 		0x77   		// ¡¾CAÖ¤Êé¡¿ÉÕÂ¼µØÖ·			// CA certificate address in flash to read, 0x77 means address 0x77000
#define CLIENT_CERT_FLASH_ADDRESS 	0x78 		// ¡¾Éè±¸Ö¤Êé¡¿ÉÕÂ¼µØÖ·			// client certificate and private key address in flash to read, 0x78 means address 0x78000
/*********************************************************************************************************************************************************************************************************************************************************************************/


/*Please Keep the following configuration if you have no very deep understanding of ESP SSL/TLS*/
#define CFG_LOCATION    			0x79		// ÏµÍ³²ÎÊýµÄÆðÊ¼ÉÈÇø	/* Please don't change or if you know what you doing */
#define MQTT_BUF_SIZE       		1024		// MQTT»º´æ´óÐ¡
#define MQTT_KEEPALIVE        		120     	// ±£³ÖÁ¬½ÓÊ±³¤			/*second*/
#define MQTT_RECONNECT_TIMEOUT    	5    		// ÖØÁ¬³¬Ê±Ê±³¤			/*second*/

#define MQTT_SSL_ENABLE				// SSLÊ¹ÄÜ	//* Please don't change or if you know what you doing */

#define QUEUE_BUFFER_SIZE      		2048		// ÏûÏ¢¶ÓÁÐµÄ»º´æ´óÐ¡

//#define PROTOCOL_NAMEv31    		// Ê¹ÓÃMQTTÐ­Òé¡¾v31¡¿°æ±¾		/*MQTT version 3.1 compatible with Mosquitto v0.15*/
#define PROTOCOL_NAMEv311      		// Ê¹ÓÃMQTTÐ­Òé¡¾v311¡¿°æ±¾		/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/




#endif /* APP_INCLUDE_DRIVER_MQTT_CONFIG_H_ */
