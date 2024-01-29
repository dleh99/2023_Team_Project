#pragma once
#include "header.h"
#include "protocol.h"

enum COMP_TYPE { OT_ACCEPT, OT_RECV, OT_SEND, OT_RESPAWN };
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

