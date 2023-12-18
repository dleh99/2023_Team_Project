#include "User_Interface.h"

User_Interface::User_Interface()
{
	_state = US_EMPTY;
	_id = -1;
	pos.x = pos.y = pos.z = 0.f;
	_prev_remain = 0;
	cx = cy = 0;
	_player_radius = 7.f;
	hp = 10;
	isinvincible = false;
	invincible_time = 0.f;
	isDeath = false;
	Death_time = 0.f;
	score = -1;
	isWin = false;
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

void User_Interface::send_login_info_packet()
{
	SC_LOGININFO_PACKET p;
	p.size = sizeof(SC_LOGININFO_PACKET);
	p.type = SC_LOGIN;
	p.id = _id;
	p.x = 0.0f;
	p.y = 0.0f;
	p.z = -50.f;
	do_send(&p);
}

void User_Interface::send_start_packet(char mapkey)
{
	SC_START_PACKET p;
	p.size = sizeof(SC_START_PACKET);
	p.type = SC_START;
	p.map_key = mapkey;
	//std::cout << mapkey << "¸¦ º¸³¿" << std::endl;
	do_send(&p);
}

void User_Interface::send_move_packet(User_Interface* clients, int c_id, Animation animation_state)
{
	SC_MOVE_PACKET p;
	p.size = sizeof(SC_MOVE_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.id = c_id;
	p.x = clients[c_id].pos.x;
	p.y = clients[c_id].pos.y;
	p.z = clients[c_id].pos.z;
	p.cxDelta = clients[c_id].cx;
	p.cyDelta = clients[c_id].cy;
	p.animation_state = animation_state;
	do_send(&p);
}

void User_Interface::send_bullet_add_packet(User_Interface* clients, int c_id, int bullet_num)
{
	SC_BULLET_ADD_PACKET p;
	p.size = sizeof(SC_BULLET_ADD_PACKET);
	p.type = SC_BULLET_ADD;
	p.s_x = clients[c_id].bullet[bullet_num].GetPosition().x;
	p.s_y = clients[c_id].bullet[bullet_num].GetPosition().y;
	p.s_z = clients[c_id].bullet[bullet_num].GetPosition().z;
	p.b_x = clients[c_id].bullet[bullet_num].GetBulletVec().x;
	p.b_y = clients[c_id].bullet[bullet_num].GetBulletVec().y;
	p.b_z = clients[c_id].bullet[bullet_num].GetBulletVec().z;
	p.player_id = c_id;
	p.bullet_id = clients[c_id].bullet[bullet_num].GetbulletId();
	do_send(&p);
}

void User_Interface::send_collision_packet(int id_1, int id_2)
{
	SC_COLLISION_PACKET p;
	p.size = sizeof(SC_COLLISION_PACKET);
	p.type = SC_COLLISION;
	p.coll_obj_id1 = id_1;
	p.coll_obj_id2 = id_2;
	do_send(&p);
}

void User_Interface::send_bullet_collision_packet(int bullet_id, int block_id, int c_id, int block_type)
{
	SC_BULLET_COLLISION_PACKET p;
	p.size = sizeof(SC_BULLET_COLLISION_PACKET);
	p.type = SC_BULLET_COLLISION;
	p.bullet_id = bullet_id;
	p.block_id = block_id;
	p.player_id = c_id;
	p.block_type = block_type;
	do_send(&p);
}

void User_Interface::send_hit_packet(int bullet_id, int player_id, int enemy_id)
{
	SC_HIT_PACKET p;
	p.size = sizeof(SC_HIT_PACKET);
	p.type = SC_HIT;
	p.bullet_id = bullet_id;
	p.player_id = player_id;
	p.enemy_id = enemy_id;
	do_send(&p);
}

void User_Interface::send_dead_packet(int bullet_id, int player_id, int death_id)
{
	SC_DEATH_PACKET p;
	p.size = sizeof(SC_DEATH_PACKET);
	p.type = SC_DEATH;
	p.bullet_id = bullet_id;
	p.player_id = player_id;
	p.death_id = death_id;
	do_send(&p);
}

void User_Interface::send_respawn_packet(float x, float y, float z, int player_id)
{
	SC_RESPAWN_PACKET p;
	p.size = sizeof(SC_RESPAWN_PACKET);
	p.type = SC_RESPAWN;
	p.player_id = player_id;
	p.respawn_x = x;
	p.respawn_y = y;
	p.respawn_z = z;
	do_send(&p);
}

void User_Interface::send_fall_packet(int fall_id)
{
	SC_FALL_PACKET p;
	p.size = sizeof(SC_FALL_PACKET);
	p.type = SC_FALL;
	p.fall_id = fall_id;
	do_send(&p);
}

void User_Interface::send_result_packet()
{
	SC_RESULT_PACKET p;
	p.size = sizeof(SC_RESULT_PACKET);
	p.type = SC_RESULT;
	p.result = isWin;
	do_send(&p);
}