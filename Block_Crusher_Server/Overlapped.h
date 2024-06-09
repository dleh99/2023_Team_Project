#pragma once
#include "header.h"
#include "protocol.h"

enum COMP_TYPE { OT_ACCEPT, OT_RECV, OT_SEND, OT_RESPAWN, OT_SIGNUP, OT_LOGIN_SUCCESS, OT_LOGIN_FAIL, OT_ALREADY_INGAME, OT_DISCONNECT, OT_UPDATE_SCORE, OT_RESTART };
class Overlapped
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _overlapped_type;

	Overlapped();
	Overlapped(char* packet);
};

