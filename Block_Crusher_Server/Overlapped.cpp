#include "Overlapped.h"

Overlapped::Overlapped()
{
	_wsabuf.len = BUF_SIZE;
	_wsabuf.buf = _send_buf;
	_overlapped_type = OT_RECV;
	ZeroMemory(&_over, sizeof(_over));
}

Overlapped::Overlapped(char* packet)
{
	_wsabuf.len = packet[0];
	_wsabuf.buf = _send_buf;
	ZeroMemory(&_over, sizeof(_over));
	_overlapped_type = OT_SEND;
	memcpy(_send_buf, packet, packet[0]);
}
