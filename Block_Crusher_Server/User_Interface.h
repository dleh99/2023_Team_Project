#pragma once
#include "header.h"
#include "Overlapped.h"
#include "GameObject.h"

constexpr int MAX_BULLET_NUM = 1000;

enum USER_STATE { US_EMPTY, US_CONNECTING, US_INGAME };
class User_Interface{
	Overlapped _recv_over;

public:
	USER_STATE		_state;
	SOCKET			_socket;
	int				_id;
	short			x, y, z;
	int				_prev_remain;				// 패킷 재조립 시 이전에 남은 데이터 양

	float			cx, cy;						// 마우스 움직임 량

	BulletObject	bullet[MAX_BULLET_NUM];

public:
	User_Interface();
	~User_Interface();

	void do_recv();
	void do_send(void* packet);

	void send_login_info_packet();
	void send_start_packet();
	void send_move_packet(User_Interface* clients, int c_id);
	void send_bullet_add_packet(User_Interface* clients, int c_id, int bullet_num);
};