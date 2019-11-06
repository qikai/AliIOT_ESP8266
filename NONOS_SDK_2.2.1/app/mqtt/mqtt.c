/*
 * mqtt.c
 *
 *  Created on: 2019��11��2��
 *      Author: qikai
 */
#include "mqtt.h"
#include "mqtt_config.h"
#include "common.h"
#include "osapi.h"

SYSCFG sysCfg;			// ϵͳ���û���

void CFG_Load(void)
{
	//os_strncpy(sysCfg.mqtt_user, MQTT_USER, sizeof(sysCfg.mqtt_user)-1);
	os_strncpy(sysCfg.device_id, MQTT_CLIENT_ID, os_strlen(MQTT_CLIENT_ID));//��ؐMQTT��䛵�ID
	os_strncpy(sysCfg.mqtt_host, MQTT_HOST, os_strlen(MQTT_HOST));			//MQTT��������ַ
	os_strncpy(sysCfg.mqtt_pass, MQTT_PASS, os_strlen(MQTT_PASS));			//MQTT����ܴa
	os_strncpy(sysCfg.mqtt_user, MQTT_USER, os_strlen(MQTT_USER));			//MQTT����Ñ���
	sysCfg.mqtt_keepalive = MQTT_KEEPALIVE;									//�����g��
	sysCfg.mqtt_port = MQTT_PORT;
	sysCfg.security = DEFAULT_SECURITY;										//������

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
// �������Ӳ�����ֵ�������������a18BP50eRCH.iot-as-mqtt.cn-shanghai.aliyuncs.com�����������Ӷ˿ڡ�1883������ȫ���͡�0��NO_TLS��
void MQTT_InitConnection(MQTT_Client *mqttClient, uint8_t* host, uint32_t port, uint8_t security)
{
	uint32_t temp;

	INFO("MQTT_InitConnection\r\n");

	os_memset(mqttClient, 0, sizeof(MQTT_Client));		//��mqtt�ͻ��˽ṹ������

	temp = os_strlen(host);								//��ȡ��������ַ�ַ�������
	mqttClient->host = (uint8_t*)os_zalloc(temp+1);		//ΪmqttClient�ṹ�������ַ���ָ�����ռ�

	os_strcpy(mqttClient->host, host);					//���ַ����������ոշ���Ŀռ���
	mqttClient->host[temp] = 0;							//���ַ���������'\0'

	mqttClient->port = port;							//�˿ڸ�ֵ

	mqttClient->security = security;					//��ȫ�ȼ���ֵ

}

