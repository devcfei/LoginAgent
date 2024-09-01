#pragma once


#include <stdint.h>

#pragma pack(push,1)

//
// Packet Header
//
typedef struct _PACKET_HEADER
{
	uint32_t Size;
	uint32_t Uid;
	uint8_t Ctrl;
	uint8_t Cmd;
}PACKET_HEADER;


//
// Command ID
//
#define C2LA_CMD_E0_LOGIN_REQUEST	0xE0
#define C2LA_CMD_E1_QUERY_SERVER	0xE1

#define LA2C_CMD_E1_LOGIN_RESP		0xE1
#define LA2C_CMD_E2_SERVER_CFG		0xE2

//
// Login Request
//	Client -> LA
//

typedef struct _PACKET_C2LA_LOGIN_REQUEST
{
	PACKET_HEADER Header;

	char username[21];
	char password[21];
	uint8_t unk0[12];
}PACKET_C2LA_LOGIN_REQUEST;

C_ASSERT(sizeof(PACKET_C2LA_LOGIN_REQUEST) == 0x40);

//
// Login Response
//	LA -> Client
//
typedef struct _PACKET_LA2C_LOGIN_RESP
{
	PACKET_HEADER Header;
	uint8_t count;
	uint8_t fix[2];
	char name[17];
	char online[0x51];
}PACKET_LA2C_LOGIN_RESP;


C_ASSERT(sizeof(PACKET_LA2C_LOGIN_RESP) == 0x6F);


//
// Select Server
//	Client -> LA
//
typedef struct _PACKET_C2LA_QUERY_SERVER
{
	PACKET_HEADER Header;
	uint8_t index;
}PACKET_C2LA_QUERY_SERVER;

C_ASSERT(sizeof(PACKET_C2LA_QUERY_SERVER) == 0xB);


//
// Server Config
//	LA -> Client
//
typedef struct _PACKET_LA2C_SERVER_CONFIG
{
	PACKET_HEADER Header;

	uint32_t unknown0;
	char ip[16]; // ip;
	uint16_t port;
	uint8_t unknown1[2];
}PACKET_LA2C_SERVER_CONFIG;

C_ASSERT(sizeof(PACKET_LA2C_SERVER_CONFIG) == 0x22);



#pragma pack(pop)