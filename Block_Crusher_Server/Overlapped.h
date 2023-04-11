#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>

#include "protocol.h"

class Overlapped
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];

	Overlapped();
	Overlapped(char* packet);
};

