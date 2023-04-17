#include "User_Interface.h"

User_Interface::User_Interface()
{
	_state = US_EMPTY;
	_id = -1;
	x = y = 0;
	_prev_remain = 0;
}

User_Interface::~User_Interface()
{
}

void User_Interface::do_recv()
{
	DWORD recv_flag = 0;
	memset(&_recv_over._over, 0, sizeof(_recv_over._over));
	_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
	_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
	WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
}

void User_Interface::do_send(void* packet)
{
	Overlapped* send_data = new Overlapped{ reinterpret_cast<char*>(packet) };
	WSASend(_socket, &send_data->_wsabuf, 1, 0, 0, &send_data->_over, 0);
}

void User_Interface::send_move_packet(int c_id)
{
}
