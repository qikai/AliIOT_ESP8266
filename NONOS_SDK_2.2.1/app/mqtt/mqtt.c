/*
 * mqtt.c
 *
 *  Created on: 2019年11月2日
 *      Author: qikai
 */
#include "mqtt.h"
#include "mqtt_config.h"
#include "common.h"
#include "osapi.h"

SYSCFG sysCfg;			// 系统配置缓存

void CFG_Load(void)
{
	//os_strncpy(sysCfg.mqtt_user, MQTT_USER, sizeof(sysCfg.mqtt_user)-1);
	os_strncpy(sysCfg.device_id, MQTT_CLIENT_ID, os_strlen(MQTT_CLIENT_ID));//拷貝MQTT登錄的ID
	os_strncpy(sysCfg.mqtt_host, MQTT_HOST, os_strlen(MQTT_HOST));			//MQTT服務器地址
	os_strncpy(sysCfg.mqtt_pass, MQTT_PASS, os_strlen(MQTT_PASS));			//MQTT登錄密碼
	os_strncpy(sysCfg.mqtt_user, MQTT_USER, os_strlen(MQTT_USER));			//MQTT登錄用戶名
	sysCfg.mqtt_keepalive = MQTT_KEEPALIVE;									//心跳間隔
	sysCfg.mqtt_port = MQTT_PORT;
	sysCfg.security = DEFAULT_SECURITY;										//不加密

	os_strncpy(sysCfg.sta_ssid, STA_SSID, os_strlen(STA_SSID));
	os_strncpy(sysCfg.sta_pwd, STA_PASS, os_strlen(STA_PASS));
	sysCfg.sta_type = STA_TYPE;
}

/**
  * @brief  MQTT initialization connection function
  * @param  client:     MQTT_Client reference
  * @param  host:     Domain or IP string
  * @param  port:     Port to connect
  * @param  security:        1 for ssl, 0 for none
  * @retval None
  */
// 网络连接参数赋值：服务端域名【a18BP50eRCH.iot-as-mqtt.cn-shanghai.aliyuncs.com】、网络连接端口【1883】、安全类型【0：NO_TLS】
void MQTT_InitConnection(MQTT_Client *mqttClient, uint8_t* host, uint32_t port, uint8_t security)
{
	uint32_t temp;

	INFO("MQTT_InitConnection\r\n");

	os_memset(mqttClient, 0, sizeof(MQTT_Client));		//将mqtt客户端结构体清零

	temp = os_strlen(host);								//获取服务器地址字符串长度
	mqttClient->host = (uint8_t*)os_zalloc(temp+1);		//为mqttClient结构体主机字符串指针分配空间

	os_strcpy(mqttClient->host, host);					//将字符串拷贝到刚刚分配的空间中
	mqttClient->host[temp] = 0;							//在字符串最后添加'\0'

	mqttClient->port = port;							//端口赋值

	mqttClient->security = security;					//安全等级赋值

}

