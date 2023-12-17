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
	int				_prev_remain;				// ��Ŷ ������ �� ������ ���� ������ ��

	float			cx, cy;						// ���콺 ������ ��

	BulletObject	bullet[MAX_BULLET_NUM];		// Ŭ���̾�Ʈ�� ���� �Ѿ� �迭

	float			_player_radius;				// �浹ó���� ���� �ٿ�� �ڽ� radius ��

	int				hp;							// hp�� ��� �Ұž�?
	bool			isinvincible;				// �¾Ƽ� ���� Ÿ���ΰ�?
	float			invincible_time;			// ���� �ð�

	bool			isDeath;					// �׾���?
	float			Death_time;					// ������ �ð�

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