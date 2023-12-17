#pragma once
#include "header.h"
#include "Overlapped.h"
#include "GameObject.h"
#include "protocol.h"

constexpr int MAX_BULLET_NUM = 1000;

enum USER_STATE { US_EMPTY, US_CONNECTING, US_INGAME };
class User_Interface{
	Overlapped _recv_over;

public:
	USER_STATE		_state;
	SOCKET			_socket;
	int				_id;
	XMFLOAT3		pos;
	int				_prev_remain;				// 패킷 재조립 시 이전에 남은 데이터 양

	float			cx, cy;						// 마우스 움직임 량

	BulletObject	bullet[MAX_BULLET_NUM];		// 클라이언트가 가진 총알 배열

	float			_player_radius;				// 충돌처리에 사용될 바운딩 박스 radius 값

	int				hp;							// hp값 어떻게 할거야?
	bool			isinvincible;				// 맞아서 무적 타임인가?
	float			invincible_time;			// 무적 시간

	bool			isDeath;					// 죽었나?
	float			Death_time;					// 리스폰 시간

public:
	User_Interface();
	~User_Interface();

	void do_recv();
	void do_send(void* packet);

	void send_login_info_packet();
	void send_start_packet(char mapkey);
	void send_move_packet(User_Interface* clients, int c_id, Animation animation_state);
	void send_bullet_add_packet(User_Interface* clients, int c_id, int bullet_num);
	void send_collision_packet(int id_1, int id_2);
	void send_bullet_collision_packet(int id_1, int id_2, int c_id, int block_type);
	void send_hit_packet(int bullet_id, int player_id, int enemy_id);
	void send_dead_packet(int bullet_id, int player_id, int death_id);
	void send_respawn_packet(float x, float y, float z, int player_id);
	void send_fall_packet(int fall_id);
};