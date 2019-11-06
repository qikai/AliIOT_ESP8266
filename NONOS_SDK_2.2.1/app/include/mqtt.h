/*
 * mqtt.h
 *
 *  Created on: 2019��11��2��
 *      Author: qikai
 */

#ifndef APP_INCLUDE_MQTT_H_
#define APP_INCLUDE_MQTT_H_
#include "ip_addr.h"
#include "common.h"
#include "ets_sys.h"
// MQTT���Ʊ���[ָ��]��[����]
//------------------------------------------------------------------------
typedef struct mqtt_message
{
  uint8_t* data;	// MQTT���Ʊ���ָ��
  uint16_t length;	// MQTT���Ʊ��ĳ���(���ñ���ʱ����Ϊ����ָ���ƫ��ֵ)

} mqtt_message_t;

// MQTT����
//------------------------------------------------------------------------
typedef struct mqtt_connection
{
  mqtt_message_t message;	// MQTT���Ʊ���[ָ��]��[����]

  uint16_t message_id;		// ���ı�ʶ��
  uint8_t* buffer;			// ��վ���Ļ�����ָ��	buffer = os_zalloc(1024)
  uint16_t buffer_length;	// ��վ���Ļ���������	1024

} mqtt_connection_t;
//------------------------------------------------------------------------

// MQTT��CONNECT�����ĵ����Ӳ���
//---------------------------------------
typedef struct mqtt_connect_info
{
  char* client_id;		// �ͻ��˱�ʶ��
  char* username;		// MQTT�û���
  char* password;		// MQTT����
  char* will_topic;		// ��������
  char* will_message;  	// ������Ϣ
  int keepalive;		// ��������ʱ��
  int will_qos;			// ������Ϣ����
  int will_retain;		// ��������
  int clean_session;	// ����Ự
} mqtt_connect_info_t;



typedef struct mqtt_event_data_t
{
  uint8_t type;
  const char* topic;
  const char* data;
  uint16_t topic_length;
  uint16_t data_length;
  uint16_t data_offset;
} mqtt_event_data_t;

typedef struct mqtt_state_t
{
  uint16_t port;
  int auto_reconnect;
  mqtt_connect_info_t* connect_info;
  uint8_t* in_buffer;
  uint8_t* out_buffer;
  int in_buffer_length;
  int out_buffer_length;
  uint16_t message_length;
  uint16_t message_length_read;
  mqtt_message_t* outbound_message;
  mqtt_connection_t mqtt_connection;
  uint16_t pending_msg_id;
  int pending_msg_type;
  int pending_publish_qos;
} mqtt_state_t;

typedef enum {
	WIFI_INIT,
	WIFI_CONNECTING,
	WIFI_CONNECTING_ERROR,
	WIFI_CONNECTED,
	DNS_RESOLVE,
	TCP_DISCONNECTING,
	TCP_DISCONNECTED,
	TCP_RECONNECT_DISCONNECTING,
	TCP_RECONNECT_REQ,
	TCP_RECONNECT,
	TCP_CONNECTING,
	TCP_CONNECTING_ERROR,
	TCP_CONNECTED,
	MQTT_CONNECT_SEND,
	MQTT_CONNECT_SENDING,
	MQTT_SUBSCIBE_SEND,
	MQTT_SUBSCIBE_SENDING,
	MQTT_DATA,
	MQTT_KEEPALIVE_SEND,
	MQTT_PUBLISH_RECV,
	MQTT_PUBLISHING,
	MQTT_DELETING,
	MQTT_DELETED,
} tConnState;

typedef void (*MqttCallback)(uint32_t *args);
typedef void (*MqttDataCallback)(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t lengh);


typedef struct{
	uint32_t cfg_holder;		// �����˱�ʶ(ֻ�и��´���ֵ��ϵͳ�����Ż����)
	uint8_t device_id[64];		// �ͻ���ID[64]		���ٷ���������[32]�������Ϊ[64]��

	uint8_t sta_ssid[64];		// WIFI��[64]
	uint8_t sta_pwd[64];		// WIFI��[64]
	uint32_t sta_type;			// STA����

	uint8_t mqtt_host[64];		// MQTT���������[64]
	uint32_t mqtt_port;			// MQTT�˿�

	uint8_t mqtt_user[64];		// MQTT�û���[64]	���ٷ���������[32]�������Ϊ[64]��
	uint8_t mqtt_pass[64];		// MQTT����[64]		���ٷ���������[32]�������Ϊ[64]��

	uint32_t mqtt_keepalive;	// ��������ʱ��
	uint8_t security;			// ��ȫ����
} SYSCFG;

extern SYSCFG sysCfg;
// MQTT�ͻ���
//-----------------------------------------------------------------------------
typedef struct
{
	struct espconn *pCon;				// TCP���ӽṹ��ָ��
	uint8_t security;					// ��ȫ����
	uint8_t* host;						// ���������/��ַ
	uint32_t port;						// �������Ӷ˿ں�
	ip_addr_t ip;						// 32λIP��ַ

	mqtt_state_t  mqtt_state;			// MQTT״̬

	mqtt_connect_info_t connect_info;	// MQTT��CONNECT�����ĵ����Ӳ���

	MqttCallback connectedCb;			// MQTT���ӳɹ�_�ص�
	MqttCallback disconnectedCb;		// MQTT�Ͽ�����_�ص�
	MqttCallback publishedCb;			// MQTT�����ɹ�_�ص�
	MqttCallback timeoutCb;				// MQTT��ʱ_�ص�
	MqttDataCallback dataCb;			// MQTT��������_�ص�

	ETSTimer mqttTimer;					// MQTT��ʱ��

	uint32_t keepAliveTick;				// MQTT�ͻ���(ESP8266)��������
	uint32_t reconnectTick;				// �����ȴ���ʱ
	uint32_t sendTimeout;				// ���ķ��ͳ�ʱʱ��

	tConnState connState;				// ESP8266����״̬

	QUEUE msgQueue;						// ��Ϣ����

	void* user_data;					// �û����ݣ�Ԥ�����û���ָ�룩

} MQTT_Client;


void CFG_Load(void);
void MQTT_InitConnection(MQTT_Client *mqttClient, uint8_t* host, uint32_t port, uint8_t security);

#endif /* APP_INCLUDE_MQTT_H_ */
