/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/8/10
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_PROTOCOL_HDR_INCLIDE
#define M_PROTOCOL_HDR_INCLIDE

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN


// 20 bytes���ɹ̶�ͷ����ѡ��(���40�ֽ�)���
typedef struct _ip_hdr_t
{
	s_uint8_t  HlenVer;		// 4λͷ������ + 4λ�汾��,ͷ����4���ֽ�Ϊ��λ
	s_uint8_t  Tos;			// 8λ��������TOS
	s_uint16_t TotalLen;		// 16λ�ܳ���,����ͷ��
	s_uint16_t Ident;			// 16λ��ʶ
	s_uint16_t FragFlags;// 16λ��־λ
	s_uint8_t  Ttl;			// ����ʱ��
	s_uint8_t  Protocol;			// Э��
	s_uint16_t CheckSum;		// У���
	s_uint32_t SrcIp;
	s_uint32_t DstIp;
}ip_hdr_t;

// icmpЭ��Ĺ�����ͷ
typedef struct _icmp_hdr_t
{
	s_uint8_t Type;		// ������
	s_uint8_t Code;		// ����
	s_uint16_t CheckSum;		// У���
}icmp_hdr_t;

// icmp���ļ�����,���Ǳ�ͷ
typedef struct _icmp_type_t
{
	s_uint32_t  Type;		// ����
	s_uint32_t  Code;		// ����
	const char* Desc;		// ����
}icmp_type_t;

static const icmp_type_t gIcmpTypeTab[]=
{
	{0,0,"����Ӧ��"},		// 0
	{3,0,"���粻�ɴ�"},
	{3,1,"�������ɴ�"},
	{3,2,"Э�鲻�ɴ�"},
	{3,3,"�˿ڲ��ɴ�"},
	{3,4,"��Ҫ���з�Ƭ�������˲���Ƭ����"}, // 5
	{3,5,"Դվѡ·ʧ��"},
	{3,6,"Ŀ�����粻��ʶ"},
	{3,7,"Ŀ����������ʶ"},
	{3,8,"Դ����������"},
	{3,9,"Ŀ�����类ǿ�ƽ�ֹ"},	// 10
	{3,10,"Ŀ��������ǿ�ƽ�ֹ"},
	{3,11,"���ڷ�������TOS�����粻�ɴ�"},
	{3,12,"���ڷ�������TOS���������ɴ�"},
	{3,13,"���ڹ��ˣ�ͨ�ű�ǿ�ƽ�ֹ"},
	{3,14,"����ԽȨ"}, // 15
	{4,0,"Դ�˱��ر�"},
	{5,0,"�������ض���"},
	{5,1,"�������ض���"},
	{5,2,"�Է������ͺ������ض���"},
	{5,3,"�Է������ͺ������ض���"}, // 20
	{8,0,"�������"},
	{9,0,"·��ͨ��"},
	{10,0,"·������"},
	{11,0,"�����ڼ�����ʱ��Ϊ0"},
	{11,1,"�����ݱ���װ�ڼ�����ʱ��Ϊ0"}, // 25
	{12,0,"����IP�ײ�"},
	{12,1,"ȱ�ٱ����ѡ��"},
	{13,0,"ʱ�������"},
	{14,0,"ʱ���Ӧ��"},
	{15,0,"��Ϣ����(���ϲ���)"}, // 30
	{16,0,"��ϢӦ��(���ϲ���)"},
	{17,0,"��ַ��������"},
	{18,0,"��ַ����Ӧ��"}, // 33
};

// echo and echo_reply header
typedef struct _icmp_echo_hdr_t : public icmp_hdr_t
{
	s_uint16_t Ident;		// ��ʶ��
	s_uint16_t Seq;			// ���к�
}icmp_echo_hdr_t;

// ʱ�������Ӧ��
typedef struct _icmp_tstamp_hdr_t : public icmp_hdr_t
{
	s_uint16_t Ident;		// ��ʶ��
	s_uint16_t Seq;			// ���к�
	s_uint32_t ReqTime;	// ����ʱ��
	s_uint32_t recv_time;	// �յ�ʱ��
	s_uint32_t TransTime;	// ����ʱ��
}icmp_tstamp_hdr_t;

// ��ַ��������Ӧ��
typedef struct _icmp_mask_hdr_t : public icmp_hdr_t
{
	s_uint16_t Ident;		// ��ʶ��
	s_uint16_t Seq;			// ���к�
	s_uint32_t Mask;		// ����

}icmp_mask_hdr_t;

M_SOCKET_NAMESPACE_END
#endif