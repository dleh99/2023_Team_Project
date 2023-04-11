#include "Overlapped.h"

Overlapped::Overlapped()
{
	_wsabuf.len = BUF_SIZE;
	_wsabuf.buf = _send_buf;
	ZeroMemory(&_over, sizeof(_over));
}

Overlapped::Overlapped(char* packet)
{
	_wsabuf.len = packet[0];
	_wsabuf.buf = _send_buf;
	ZeroMemory(&_over, sizeof(_over));
	memcpy(_send_buf, packet, packet[0]);
}
