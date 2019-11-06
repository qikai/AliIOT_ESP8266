/*
 * mqtt.h
 *
 *  Created on: 2019年11月2日
 *      Author: qikai
 */

#ifndef APP_INCLUDE_MQTT_H_
#define APP_INCLUDE_MQTT_H_
#include "ip_addr.h"
#include "common.h"
#include "ets_sys.h"
// MQTT控制报文[指针]、[长度]
//------------------------------------------------------------------------
typedef struct mqtt_message
{
  uint8_t* data;	// MQTT控制报文指针
  uint16_t length;	// MQTT控制报文长度(配置报文时，作为报文指针的偏移值)

} mqtt_message_t;

// MQTT报文
//------------------------------------------------------------------------
typedef struct mqtt_connection
{
  mqtt_message_t message;	// MQTT控制报文[指针]、[长度]

  uint16_t message_id;		// 报文标识符
  uint8_t* buffer;			// 出站报文缓存区指针	buffer = os_zalloc(1024)
  uint16_t buffer_length;	// 出站报文缓存区长度	1024

} mqtt_connection_t;
//------------------------------------------------------------------------

// MQTT【CONNECT】报文的连接参数
//---------------------------------------
typedef struct mqtt_connect_info
{
  char* client_id;		// 客户端标识符
  char* username;		// MQTT用户名
  char* password;		// MQTT密码
  char* will_topic;		// 遗嘱主题
  char* will_message;  	// 遗嘱消息
  int keepalive;		// 保持连接时长
  int will_qos;			// 遗嘱消息质量
  int will_retain;		// 遗嘱保留
  int clean_session;	// 清除会话
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
	uint32_t cfg_holder;		// 持有人标识(只有更新此数值，系统参数才会更新)
	uint8_t device_id[64];		// 客户端ID[64]		【官方例程中是[32]，将其改为[64]】

	uint8_t sta_ssid[64];		// WIFI名[64]
	uint8_t sta_pwd[64];		// WIFI密[64]
	uint32_t sta_type;			// STA类型

	uint8_t mqtt_host[64];		// MQTT服务端域名[64]
	uint32_t mqtt_port;			// MQTT端口

	uint8_t mqtt_user[64];		// MQTT用户名[64]	【官方例程中是[32]，将其改为[64]】
	uint8_t mqtt_pass[64];		// MQTT密码[64]		【官方例程中是[32]，将其改为[64]】

	uint32_t mqtt_keepalive;	// 保持连接时长
	uint8_t security;			// 安全类型
} SYSCFG;

extern SYSCFG sysCfg;
// MQTT客户端
//-----------------------------------------------------------------------------
typedef struct
{
	struct espconn *pCon;				// TCP连接结构体指针
	uint8_t security;					// 安全类型
	uint8_t* host;						// 服务端域名/地址
	uint32_t port;						// 网络连接端口号
	ip_addr_t ip;						// 32位IP地址

	mqtt_state_t  mqtt_state;			// MQTT状态

	mqtt_connect_info_t connect_info;	// MQTT【CONNECT】报文的连接参数

	MqttCallback connectedCb;			// MQTT连接成功_回调
	MqttCallback disconnectedCb;		// MQTT断开连接_回调
	MqttCallback publishedCb;			// MQTT发布成功_回调
	MqttCallback timeoutCb;				// MQTT超时_回调
	MqttDataCallback dataCb;			// MQTT接收数据_回调

	ETSTimer mqttTimer;					// MQTT定时器

	uint32_t keepAliveTick;				// MQTT客户端(ESP8266)心跳计数
	uint32_t reconnectTick;				// 重连等待计时
	uint32_t sendTimeout;				// 报文发送超时时间

	tConnState connState;				// ESP8266运行状态

	QUEUE msgQueue;						// 消息队列

	void* user_data;					// 用户数据（预留给用户的指针）

} MQTT_Client;


void CFG_Load(void);
void MQTT_InitConnection(MQTT_Client *mqttClient, uint8_t* host, uint32_t port, uint8_t security);

#endif /* APP_INCLUDE_MQTT_H_ */
