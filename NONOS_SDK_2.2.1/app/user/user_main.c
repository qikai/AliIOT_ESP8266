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
// 工程：	MQTT_JX												//	注：在《esp_mqtt_proj》例程上修改									//
//															//																		//
// 平台：	【技新电子】物联网开发板 ESP8266 V1.0						//	①：添加【详实的注释】唉，不说了，说多了都是泪！！！				//
//															//																		//
// 功能：		①：设置MQTT相关参数									//	②：修改【MQTT参数数组】config.h -> device_id/mqtt_host/mqtt_pass	//
//															//																		//
//			②：与MQTT服务端，建立网络连接(TCP)						//	③：修改【MQTT_CLIENT_ID宏定义】mqtt_config.h -> MQTT_CLIENT_ID		//
//															//																		//
//			③：配置/发送【CONNECT】报文，连接MQTT服务端				//	④：修改【PROTOCOL_NAMEv31宏】mqtt_config.h -> PROTOCOL_NAMEv311	//
//															//																		//
//			④：订阅主题"LED_CTRL"								//	⑤：修改【心跳报文的发送间隔】mqtt.c ->	[mqtt_timer]函数			//
//															//																		//
//			⑤：向主题"LED_CTRL"发布"ESP8266_Online"				//	⑥：修改【SNTP服务器设置】user_main.c -> [sntpfn]函数				//
//															//																		//
//			⑥：根据接收到"LED_CTRL"主题的消息，控制LED亮灭			//	⑦：注释【遗嘱设置】user_main.c -> [user_init]函数					//
//															//																		//
//			⑦：每隔一分钟，向MQTT服务端发送【心跳】					//	⑧：添加【MQTT消息控制LED亮/灭】user_main.c -> [mqttDataCb]函数		//
//															//																		//
//	版本：	V1.1											//																		//
//															//																		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ets_sys.h"
#include "osapi.h"
#include "mqtt.h"
#include "user_interface.h"

#include "user_config.h"		// 用户配置
#include "driver/uart.h"  		// 串口
#include "driver/oled.h"  		// OLED头文件

#include "osapi.h"  			// os_XXX、软件定时器

#define		ProjectName			"AliIOT"


MQTT_Client mqttClient;			// MQTT客户端_结构体【此变量非常重要】
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

// 毫秒延时函数
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
//MQTT_Client mqttClient;			// MQTT客户端_结构体【此变量非常重要】


void ICACHE_FLASH_ATTR user_init(void)
{
	u8 LED_ON = 0 ;
	uart_init(115200,115200);	// 初始化串口波特率
	os_delay_us(10000);			// 等待串口稳定
	os_printf("\r\n=================================================\r\n");
	os_printf("\t Project:\t%s\r\n", ProjectName);
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	OLED_Init();
	OLED_ShowString(0,0,"Project=");		// 从(0,0)开始显示
	OLED_ShowString(64,0,"AliIOT");

	GPIO_OUTPUT_SET(GPIO_ID_PIN(4), LED_ON);

	CFG_Load();	// 加载/更新系统参数【WIFI参数、MQTT参数】

	//MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);

}



