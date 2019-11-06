/*
 * common.c
 *
 *  Created on: 2019年11月3日
 *      Author: qikai
 */
#include "common.h"
#include "stddef.h"
#include "c_types.h"

// 队列参数初始化：设置[原/读/写指针]、[装填数量]、[队列尺寸]
I16 RINGBUF_Init(RINGBUF *r, U8* buf, I32 size)
{
	if(r == NULL || buf == NULL || size < 2) return -1;

	r->p_o = r->p_r = r->p_w = buf;	// [原/读/写指针]指向相同位置(队列首地址)
	r->fill_cnt = 0;
	r->size = size;

	return 0;
}
//往ringbuf里增加一个字节数据
I16 RINGBUF_Put(RINGBUF *r, U8 c)
{
	if(r->fill_cnt >= r->size) return -1;	//如果当前的缓冲buf已经满了，则返回

	r->fill_cnt++;							//增加已填充的数据数量
	*r->p_w++ = c;							//将数据放入缓冲buffer里
	if(r->p_w>= r->p_o + r->size)			//如果地址已经超出buffer末尾
		r->p_w = r->p_o;					//则将写数据指针rollback至开头

	return 0;
}
//从ringbuf里读取一个字节数据
I16 RINGBUF_Get(RINGBUF *r, U8* c)
{
	if(r->fill_cnt <= 0) 	return -1;   	//当前buffer里没有数据

	r->fill_cnt--;							//将buffer的数据数量减1

	*c = *r->p_r++;							//取出一个数据，并将读指针加1

	if(r->p_r >= r->p_o + r->size)			//如果读取到了buffer末尾
		r->p_r = r->p_o;					//则将读指针rollback至开头

	return 0;
}


void  QUEUE_Init(QUEUE *queue, int bufferSize)
{
	queue->buf = (uint8_t *)os_zalloc(bufferSize);

	RINGBUF_Init(&queue->rb, queue->buf, bufferSize);	// 设置队列参数
}

int32_t  QUEUE_Puts(QUEUE *queue, uint8_t* buffer, uint16_t len)
{
	return PROTO_AddRb(&queue->rb, buffer, len);
}

int32_t  QUEUE_Gets(QUEUE *queue, uint8_t* buffer, uint16_t* len, uint16_t maxLen)
{
	return PROTO_ParseRb(&queue->rb, buffer, len, maxLen);
}

BOOL  QUEUE_IsEmpty(QUEUE *queue)
{
	if(queue->rb.fill_cnt <= 0)
		return TRUE;
	return FALSE;
}



// 解析初始化：协议解析结构体初始化
//------------------------------------------------------------------------------
I8  PROTO_Init(PROTO_PARSER *parser, PROTO_PARSE_CALLBACK *completeCallback, U8 *buf, U16 bufSize)
{
	parser->buf = buf;						// 解析后的报文指针(指向局部数组变量的首地址)
	parser->bufSize = bufSize;				// 缓存空间大小
	parser->dataLen = 0;					// 当前解析数据长度
	parser->callback = completeCallback;	// 回调函数
	parser->isEsc = 0;						// [前缀码：0x7D]标志位
	return 0;
}

//从buffer里读取len个字节数据
I8  PROTO_Parse(PROTO_PARSER *parser, U8 *buf, U16 len)
{
	while(len--)
		PROTO_ParseByte(parser, *buf++);
	return 0;
}

I16 PROTO_Add(U8 *buf, const U8 *packet, I16 bufSize)
{
	U16 i = 2;
	U16 len = *(U16 *)packet;

	if(bufSize < 1) return -1;

	*buf++ = 0x7E;
	bufSize--;

	while(len--){
		switch(*packet){
		case 0x7D:
		case 0x7E:
		case 0x7F:
			if(bufSize < 2) return -1;
			*buf++ = 0x7D;
			*buf++ = *packet++ ^ 0x20;
			i += 2;
			bufSize -=2;
			break;
		default:
			if(bufSize < 1) return -1;
			*buf++ = *packet++;
			i++;
			bufSize--;
			break;
		}
	}

	if(bufSize < 1) return -1;

	*buf++ = 0x7F;

	return i;


}
// 将报文依次写入队列写指针(p_w)所指向的位置，添加【起始码】【结束码】【前缀码】，并返回写入的字节数
//---------------------------------------------------------------------------------------------------------
// 注：队列中【起始码】=[0x7E]、【结束码】=[0x7F]、【前缀码】=[Ox7D]。	报文中的[0x7D][0x7E][0x7F]，需要避讳特殊码(通过异或方式)
I16 PROTO_AddRb(RINGBUF *rb, const U8 *packet, I16 len)
{
	U16 i = 2;

	if(RINGBUF_Put(rb, 0x7E) == -1) return -1;						//先写入起始码，如果写入出错，返回

	while(len--)
	{
		switch(*packet)
		{
		case 0x7D:													//报文正文中含有[0x7D][0x7E][0x7F]，需要进行特殊处理
		case 0x7E:
		case 0x7F:
			if(RINGBUF_Put(rb, 0x7D) == -1) return -1;				//先在这个字节前写入前缀码
			if(RINGBUF_Put(rb, *packet++^0x20) == -1) return -1;	//然后将报文内容与0x20进行异或
			i += 2;
			break;
		default:
			if(RINGBUF_Put(rb, *packet++) == -1) return -1;			//不含有特殊内容的报文数据 正常写入
			i++;
			break;
		}
	}

	if(RINGBUF_Put(rb, 0x7F) == -1) return -1;						//为本条报文添加结束码

	return i;
}

// 解析数据包的一个字节(特殊码需特殊处理)
I8  PROTO_ParseByte(PROTO_PARSER *parser, U8 value)
{
	switch(value)
	{
		case 0x7D:							// [前缀码：0x7D]
			parser->isEsc = 1;				// 前缀码标志位 = 1
			break;

		case 0x7E:							// 开始码[0x7E]
			parser->dataLen = 0;			// 已解析数据长度 = 0
			parser->isEsc = 0;				// 不是[转义码]
			parser->isBegin = 1;			// 开始标志 = 1
			break;

		case 0x7F:							// 结束码[0x7F]
			if(parser->callback != NULL)	// 判断有无回调函数
				parser->callback();			// 执行回调函数
			parser->isBegin = 0;			// 开始标志 = 0

			return 0;
			break;

		default:
			if(parser->isBegin == 0)		// 直到找到[开始码]，才会开始解析队列
				break;
			if(parser->isEsc)				// 如果上一字节是[前缀码：0x7D]
			{
				value ^= 0x20;				// 当前字节^=0x20(注：a^b^b == a)
				parser->isEsc = 0;
			}

			if(parser->dataLen < parser->bufSize)			// 解析数量 < 解析缓存区大小
				parser->buf[parser->dataLen++] = value;		// 将【解析后的数据】存放到【解析缓存区】
			break;
	}
	return -1;
}

// 解析队列中的数据，解析的报文指针赋值给[参数2：bufOut]【遇到[0x7F]结束码则返回0】
I16 PROTO_ParseRb(RINGBUF *rb, U8 *bufOut, U16* len, U16 maxBufLen)
{
	U8 c;
	PROTO_PARSER proto;								// 定义队列解析结构体

	PROTO_Init(&proto, NULL, bufOut, maxBufLen);	// 初始化队列解析结构体

	while(RINGBUF_Get(rb, &c) == 0)					// 从ringBuf里取一个字节数据,直到取完
	{
		if(PROTO_ParseByte(&proto, c) == 0)			// 解析队列的一个字节
		{	// 遇到【0x7F】结束码，进入这里
			*len = proto.dataLen;					// 解析后的报文长度
			return 0;
		}
	}
	return -1;
}



