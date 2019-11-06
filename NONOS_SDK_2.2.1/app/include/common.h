/*
 * common.h
 *
 *  Created on: 2019年11月3日
 *      Author: qikai
 */

#ifndef APP_INCLUDE_COMMON_H_
#define APP_INCLUDE_COMMON_H_
#include "c_types.h"

typedef char I8;
typedef unsigned char U8;
typedef short I16;
typedef unsigned short U16;
typedef long I32;
typedef unsigned long U32;
typedef unsigned long long U64;

#define true            (1)
#define false           (0)
#define TRUE            true
#define FALSE           false

//typedef unsigned char   bool;

//#define BOOL            U8

typedef struct{
	U8* p_o;				/**< Original pointer */		// 队列首指针
	U8* volatile p_r;		/**< Read pointer */			// 读指针
	U8* volatile p_w;		/**< Write pointer */			// 写指针
	volatile I32 fill_cnt;	/**< Number of filled slots */	// 装填数量
	I32 size;				/**< Buffer size */				// 队列大小（2048）
}RINGBUF;

typedef void(PROTO_PARSE_CALLBACK)();


// 队列解析结构体
//---------------------------------------------------------------
typedef struct{
	U8 *buf;							// 解析后的报文指针
	U16 bufSize;						// 解析缓存区大小
	U16 dataLen;						// 已解析数据长度
	U8 isEsc;							// [前缀码：0x7D]标志位
	U8 isBegin;							// [起始码：0x7E]标志位
	PROTO_PARSE_CALLBACK* callback;		// 回调函数
}PROTO_PARSER;
//---------------------------------------------------------------

I8  PROTO_Init(PROTO_PARSER *parser, PROTO_PARSE_CALLBACK *completeCallback, U8 *buf, U16 bufSize);
I8  PROTO_Parse(PROTO_PARSER *parser, U8 *buf, U16 len);
I16 PROTO_Add(U8 *buf, const U8 *packet, I16 bufSize);
I16 PROTO_AddRb(RINGBUF *rb, const U8 *packet, I16 len);
I8  PROTO_ParseByte(PROTO_PARSER *parser, U8 value);
I16 PROTO_ParseRb(RINGBUF *rb, U8 *bufOut, U16* len, U16 maxBufLen);




// MQTT消息队列
//---------------------------------------------------------------
typedef struct
{
	uint8_t * buf;	// 队列缓存		queue->buf = os_zalloc(2048)
	RINGBUF rb;		// 队列参数

} QUEUE;
//---------------------------------------------------------------

void  QUEUE_Init(QUEUE *queue, int bufferSize);
int32_t  QUEUE_Puts(QUEUE *queue, uint8_t* buffer, uint16_t len);
int32_t  QUEUE_Gets(QUEUE *queue, uint8_t* buffer, uint16_t* len, uint16_t maxLen);
BOOL  QUEUE_IsEmpty(QUEUE *queue);



//---------------------------Ring Buf----------------------------
I16 RINGBUF_Init(RINGBUF *r, U8* buf, I32 size);
I16 RINGBUF_Put(RINGBUF *r, U8 c);
I16 RINGBUF_Get(RINGBUF *r, U8* c);

#endif /* APP_INCLUDE_COMMON_H_ */
