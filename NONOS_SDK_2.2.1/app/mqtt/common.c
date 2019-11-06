/*
 * common.c
 *
 *  Created on: 2019��11��3��
 *      Author: qikai
 */
#include "common.h"
#include "stddef.h"
#include "c_types.h"

// ���в�����ʼ��������[ԭ/��/дָ��]��[װ������]��[���гߴ�]
I16 RINGBUF_Init(RINGBUF *r, U8* buf, I32 size)
{
	if(r == NULL || buf == NULL || size < 2) return -1;

	r->p_o = r->p_r = r->p_w = buf;	// [ԭ/��/дָ��]ָ����ͬλ��(�����׵�ַ)
	r->fill_cnt = 0;
	r->size = size;

	return 0;
}
//��ringbuf������һ���ֽ�����
I16 RINGBUF_Put(RINGBUF *r, U8 c)
{
	if(r->fill_cnt >= r->size) return -1;	//�����ǰ�Ļ���buf�Ѿ����ˣ��򷵻�

	r->fill_cnt++;							//������������������
	*r->p_w++ = c;							//�����ݷ��뻺��buffer��
	if(r->p_w>= r->p_o + r->size)			//�����ַ�Ѿ�����bufferĩβ
		r->p_w = r->p_o;					//��д����ָ��rollback����ͷ

	return 0;
}
//��ringbuf���ȡһ���ֽ�����
I16 RINGBUF_Get(RINGBUF *r, U8* c)
{
	if(r->fill_cnt <= 0) 	return -1;   	//��ǰbuffer��û������

	r->fill_cnt--;							//��buffer������������1

	*c = *r->p_r++;							//ȡ��һ�����ݣ�������ָ���1

	if(r->p_r >= r->p_o + r->size)			//�����ȡ����bufferĩβ
		r->p_r = r->p_o;					//�򽫶�ָ��rollback����ͷ

	return 0;
}


void  QUEUE_Init(QUEUE *queue, int bufferSize)
{
	queue->buf = (uint8_t *)os_zalloc(bufferSize);

	RINGBUF_Init(&queue->rb, queue->buf, bufferSize);	// ���ö��в���
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



// ������ʼ����Э������ṹ���ʼ��
//------------------------------------------------------------------------------
I8  PROTO_Init(PROTO_PARSER *parser, PROTO_PARSE_CALLBACK *completeCallback, U8 *buf, U16 bufSize)
{
	parser->buf = buf;						// ������ı���ָ��(ָ��ֲ�����������׵�ַ)
	parser->bufSize = bufSize;				// ����ռ��С
	parser->dataLen = 0;					// ��ǰ�������ݳ���
	parser->callback = completeCallback;	// �ص�����
	parser->isEsc = 0;						// [ǰ׺�룺0x7D]��־λ
	return 0;
}

//��buffer���ȡlen���ֽ�����
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
// ����������д�����дָ��(p_w)��ָ���λ�ã���ӡ���ʼ�롿�������롿��ǰ׺�롿��������д����ֽ���
//---------------------------------------------------------------------------------------------------------
// ע�������С���ʼ�롿=[0x7E]���������롿=[0x7F]����ǰ׺�롿=[Ox7D]��	�����е�[0x7D][0x7E][0x7F]����Ҫ�ܻ�������(ͨ�����ʽ)
I16 PROTO_AddRb(RINGBUF *rb, const U8 *packet, I16 len)
{
	U16 i = 2;

	if(RINGBUF_Put(rb, 0x7E) == -1) return -1;						//��д����ʼ�룬���д���������

	while(len--)
	{
		switch(*packet)
		{
		case 0x7D:													//���������к���[0x7D][0x7E][0x7F]����Ҫ�������⴦��
		case 0x7E:
		case 0x7F:
			if(RINGBUF_Put(rb, 0x7D) == -1) return -1;				//��������ֽ�ǰд��ǰ׺��
			if(RINGBUF_Put(rb, *packet++^0x20) == -1) return -1;	//Ȼ�󽫱���������0x20�������
			i += 2;
			break;
		default:
			if(RINGBUF_Put(rb, *packet++) == -1) return -1;			//�������������ݵı������� ����д��
			i++;
			break;
		}
	}

	if(RINGBUF_Put(rb, 0x7F) == -1) return -1;						//Ϊ����������ӽ�����

	return i;
}

// �������ݰ���һ���ֽ�(�����������⴦��)
I8  PROTO_ParseByte(PROTO_PARSER *parser, U8 value)
{
	switch(value)
	{
		case 0x7D:							// [ǰ׺�룺0x7D]
			parser->isEsc = 1;				// ǰ׺���־λ = 1
			break;

		case 0x7E:							// ��ʼ��[0x7E]
			parser->dataLen = 0;			// �ѽ������ݳ��� = 0
			parser->isEsc = 0;				// ����[ת����]
			parser->isBegin = 1;			// ��ʼ��־ = 1
			break;

		case 0x7F:							// ������[0x7F]
			if(parser->callback != NULL)	// �ж����޻ص�����
				parser->callback();			// ִ�лص�����
			parser->isBegin = 0;			// ��ʼ��־ = 0

			return 0;
			break;

		default:
			if(parser->isBegin == 0)		// ֱ���ҵ�[��ʼ��]���ŻῪʼ��������
				break;
			if(parser->isEsc)				// �����һ�ֽ���[ǰ׺�룺0x7D]
			{
				value ^= 0x20;				// ��ǰ�ֽ�^=0x20(ע��a^b^b == a)
				parser->isEsc = 0;
			}

			if(parser->dataLen < parser->bufSize)			// �������� < ������������С
				parser->buf[parser->dataLen++] = value;		// ��������������ݡ���ŵ���������������
			break;
	}
	return -1;
}

// ���������е����ݣ������ı���ָ�븳ֵ��[����2��bufOut]������[0x7F]�������򷵻�0��
I16 PROTO_ParseRb(RINGBUF *rb, U8 *bufOut, U16* len, U16 maxBufLen)
{
	U8 c;
	PROTO_PARSER proto;								// ������н����ṹ��

	PROTO_Init(&proto, NULL, bufOut, maxBufLen);	// ��ʼ�����н����ṹ��

	while(RINGBUF_Get(rb, &c) == 0)					// ��ringBuf��ȡһ���ֽ�����,ֱ��ȡ��
	{
		if(PROTO_ParseByte(&proto, c) == 0)			// �������е�һ���ֽ�
		{	// ������0x7F�������룬��������
			*len = proto.dataLen;					// ������ı��ĳ���
			return 0;
		}
	}
	return -1;
}



