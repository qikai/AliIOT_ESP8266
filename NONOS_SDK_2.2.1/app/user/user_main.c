/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															//																		//
// ���̣�	MQTT_JX												//	ע���ڡ�esp_mqtt_proj���������޸�									//
//															//																		//
// ƽ̨��	�����µ��ӡ������������� ESP8266 V1.0						//	�٣���ӡ���ʵ��ע�͡�������˵�ˣ�˵���˶����ᣡ����				//
//															//																		//
// ���ܣ�		�٣�����MQTT��ز���									//	�ڣ��޸ġ�MQTT�������顿config.h -> device_id/mqtt_host/mqtt_pass	//
//															//																		//
//			�ڣ���MQTT����ˣ�������������(TCP)						//	�ۣ��޸ġ�MQTT_CLIENT_ID�궨�塿mqtt_config.h -> MQTT_CLIENT_ID		//
//															//																		//
//			�ۣ�����/���͡�CONNECT�����ģ�����MQTT�����				//	�ܣ��޸ġ�PROTOCOL_NAMEv31�꡿mqtt_config.h -> PROTOCOL_NAMEv311	//
//															//																		//
//			�ܣ���������"LED_CTRL"								//	�ݣ��޸ġ��������ĵķ��ͼ����mqtt.c ->	[mqtt_timer]����			//
//															//																		//
//			�ݣ�������"LED_CTRL"����"ESP8266_Online"				//	�ޣ��޸ġ�SNTP���������á�user_main.c -> [sntpfn]����				//
//															//																		//
//			�ޣ����ݽ��յ�"LED_CTRL"�������Ϣ������LED����			//	�ߣ�ע�͡��������á�user_main.c -> [user_init]����					//
//															//																		//
//			�ߣ�ÿ��һ���ӣ���MQTT����˷��͡�������					//	�ࣺ��ӡ�MQTT��Ϣ����LED��/��user_main.c -> [mqttDataCb]����		//
//															//																		//
//	�汾��	V1.1											//																		//
//															//																		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ets_sys.h"
#include "osapi.h"
#include "mqtt.h"
#include "user_interface.h"

#include "user_config.h"		// �û�����
#include "driver/uart.h"  		// ����
#include "driver/oled.h"  		// OLEDͷ�ļ�

#include "osapi.h"  			// os_XXX�������ʱ��

#define		ProjectName			"AliIOT"


MQTT_Client mqttClient;			// MQTT�ͻ���_�ṹ�塾�˱����ǳ���Ҫ��
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
            rf_cal_sec = 512 - 5;
            break;
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}



void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

// ������ʱ����
//===========================================
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
//MQTT_Client mqttClient;			// MQTT�ͻ���_�ṹ�塾�˱����ǳ���Ҫ��


void ICACHE_FLASH_ATTR user_init(void)
{
	u8 LED_ON = 0 ;
	uart_init(115200,115200);	// ��ʼ�����ڲ�����
	os_delay_us(10000);			// �ȴ������ȶ�
	os_printf("\r\n=================================================\r\n");
	os_printf("\t Project:\t%s\r\n", ProjectName);
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	OLED_Init();
	OLED_ShowString(0,0,"Project=");		// ��(0,0)��ʼ��ʾ
	OLED_ShowString(64,0,"AliIOT");

	GPIO_OUTPUT_SET(GPIO_ID_PIN(4), LED_ON);

	CFG_Load();	// ����/����ϵͳ������WIFI������MQTT������

	//MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);

}



