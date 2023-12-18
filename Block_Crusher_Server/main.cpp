#include "header.h"
#include "protocol.h"
#include "Overlapped.h"
#include "User_Interface.h"
#include "Timer.h"
#include "Map.h"
#include "Physics.h"

using namespace std;
using namespace chrono;

HANDLE iocp_h;
SOCKET g_s_socket, g_c_socket;
Overlapped g_over;
Timer server_timer;

array<User_Interface, MAX_USER> clients;

Map Map_infromation;
Physics physics_engine;

atomic_int user_number = 0;

auto start_time = system_clock::now();

int set_client_id()
{
	for (int i{}; i < MAX_USER; ++i) {
		if (clients[i]._state == US_EMPTY)
			return i;
	}
	return -1;
}

void disconnect(int c_id)
{
	closesocket(clients[c_id]._socket);
	clients[c_id]._state = US_EMPTY;
	cout << "[" << c_id << "] Ż����" << endl;
	user_number--;
}

bool CollisionCheck_Person(XMFLOAT3 bullet, XMFLOAT3 player, float bullet_r, float player_r)
{
	float x = bullet.x - player.x;
	float y = bullet.y - (player.y + 7.f);
	float z = bullet.z - player.z;

	if (bullet_r + player_r > sqrt(x * x + y * y + z * z)) return true;
	return false;
}

bool CollisionCheck_objects(XMFLOAT3 p1, XMFLOAT3 p2, float r1, float r2)
{
	float x = p1.x - p2.x;
	float y = p1.y - p2.y;
	float z = p1.z - p2.z;

	if (r1 + r2 - 4.f > sqrt(x * x + y * y + z * z)) return true;
	return false;
}

bool scoreCalculation()
{
	int num = 0;
	int max_score = 0;
	for (auto& cl : clients) {
		if (cl._state != US_INGAME) continue;
		if (cl.score != -1) {
			num++;
			if (max_score < cl.score)
				max_score = cl.score;
		}
	}
	if (num == 3) {
		for (auto& cl : clients) {
			if (cl.score == max_score)
				cl.isWin = true;
		}
		return true;
	}
	return false;
}

void packet_process(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		cout << c_id << " ���� �Ϸ�" << endl;
		//clients[c_id]._state = US_INGAME;
		clients[c_id].send_login_info_packet();
		user_number++;
		if (user_number == 3)
			for (auto& cl : clients)
				if (cl._state == US_CONNECTING) {
					cl._state = US_INGAME;
					cl.send_start_packet(Map_infromation.MapChar);
				}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		// ��Ŷ���� ������ ���⿡ ���� ��ġ �̵� �ڵ�
		XMFLOAT3 accept_position{ p->x, p->y, p->z };
		clients[c_id].pos = accept_position;
		clients[c_id].cx = p->cxDelta;
		clients[c_id].cy = p->cyDelta;
		//cout << "[" << c_id << "] " << p->animation_state << endl;

		//cout << "[" << c_id << "] Ŭ���̾�Ʈ ���� ������ : " << p->frame_num << endl;
		/*auto now_time = system_clock::now();
		auto exec_time = now_time - start_time;
		auto ms = duration_cast<milliseconds>(exec_time).count();*/
		//cout << ms << endl;

		// �ٸ� Ŭ���̾�Ʈ�鿡�� �Ѹ���
		for (auto& cl : clients) {
			if (cl._state != US_INGAME) continue;
			if (cl._id == c_id) continue;
			cl.send_move_packet(clients.data(), c_id, p->animation_state);
		}
		break;
	}
	case CS_BULLET_ADD: {
		CS_BULLET_ADD_PACKET* p = reinterpret_cast<CS_BULLET_ADD_PACKET*>(packet);
		//cout << "[" << c_id << "] �Ѿ� �߻�" << endl;
		for (int i{}; i < MAX_BULLET_NUM; ++i) {
			if (false == clients[c_id].bullet[i].GetisActive()) {
				clients[c_id].bullet[i].SetisActive(true);
				clients[c_id].bullet[i].SetPosition(p->s_x, p->s_y, p->s_z);
				clients[c_id].bullet[i].SetBulletVec(p->b_x, p->b_y, p->b_z);
				clients[c_id].bullet[i].SetBulletId(p->bullet_id);

				for (auto& cl : clients) {
					if (cl._state != US_INGAME) continue;
					if (cl._id == c_id) continue;
					//cout << "�������� " << cl._id << "���� " << c_id << "�� ���� ���ٴ°� ��������ϴ�" << endl;
					cl.send_bullet_add_packet(clients.data(), c_id, i);
				}
				break;
			}
		}
		break;
	}
	case CS_FALL: {
		CS_FALL_PACKET* p = reinterpret_cast<CS_FALL_PACKET*>(packet);
		clients[c_id].isDeath = true;
		for (auto& cl : clients) {
			if (cl._state != US_INGAME) continue;
			if (cl._id == c_id) continue;
			cl.send_fall_packet(c_id);
		}
		break;
	}
	case CS_SCORE: {
		CS_SCORE_PACKET* p = reinterpret_cast<CS_SCORE_PACKET*>(packet);
		clients[c_id].score = p->score;
		if (true == scoreCalculation()) {
			for (auto& cl : clients) {
				if (cl._state != US_INGAME) continue;
				cl.send_result_packet();
			}
		}
		break;
	}
	}
}

void worker_thread(HANDLE iocp_h)
{
	while (true) {
		DWORD byte_size;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;

		BOOL ret = GetQueuedCompletionStatus(iocp_h, &byte_size, &key, &over, INFINITE);
		Overlapped* ex_over = reinterpret_cast<Overlapped*>(over);

		// ������ ���� �� ó��
		if (FALSE == ret) {
			if (ex_over->_overlapped_type == OT_ACCEPT) std::cout << "Error of Accept";
			else {
				//std::cout << "Error on client [" << key << "] in GQCS" << std::endl;
				disconnect(key);
				if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
				continue;
			}
		}

		// �� �����Ͱ� ���� ��
		if ((byte_size == 0) && ((ex_over->_overlapped_type == OT_RECV) || (ex_over->_overlapped_type == OT_SEND))) {
			disconnect(key);
			if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
			continue;
		}

		// ���� ������ Ÿ�Կ� ���� ó��
		switch (ex_over->_overlapped_type)
		{
			case OT_ACCEPT:
			{
				int client_id = set_client_id();
				if (client_id != -1)
				{
					// ���� ��ȯ(������)
					clients[client_id]._state = US_CONNECTING;

					// �ʱⰪ ����
					clients[client_id].pos.x = 0;
					clients[client_id].pos.y = 0;
					clients[client_id]._id = client_id;
					clients[client_id]._prev_remain = 0;
					clients[client_id]._socket = g_c_socket;

					CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), iocp_h, client_id, 0);
					clients[client_id].do_recv();
					g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else
				{
					std::cout << "Max User in Game" << std::endl;
				}
				ZeroMemory(&g_over._over, sizeof(g_over._over));
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_s_socket, g_c_socket, g_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_over._over);
				break;
			}
			case OT_RECV:
			{
				int remain_data = byte_size + clients[key]._prev_remain;
				char* p = ex_over->_send_buf;
				// ��Ŷ ������
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						packet_process(static_cast<int>(key), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				// ������ ���ٸ�
				clients[key]._prev_remain = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->_send_buf, p, remain_data);
				}
				clients[key].do_recv();
				break;
			}
			case OT_SEND:
			{
				delete ex_over;
				break;
			}
		}
	}
}

void Physics_Calculation_thread()
{
	/* �� �����尡 �ϴ� ��
	* 1. Ȱ��ȭ �Ǿ� �ִ� �Ѿ˵��� ������ ����
	*		-> �Ѿ��� ���� �ð� ���� ������
	* 2. �Ѿ˵�� ������Ʈ���� �浹 ���
	*		-> �浹 �� ��Ŷ ������
	*/
	while (true) {
		server_timer.Tick(0.f);
		for (auto& cl : clients) {
			if (cl._state != US_INGAME) continue;
			for (int i{}; i < MAX_BULLET_NUM; ++i) {
				if (cl.bullet[i].GetisActive()) {
					cl.bullet[i].Move(server_timer.GetTimeElapsed());
					//cout << "[" << cl._id << "] " << cl.bullet[i].GetPosition().x << ", " << cl.bullet[i].GetPosition().y << ", " << cl.bullet[i].GetPosition().z << endl;
				}
			}
		}

		// �Ѿ˰� �浹 ó��
		/*
		* Ŭ���̾�Ʈ ��ü�� ���Ƽ� �ΰ��� ���� Ŭ���̾�Ʈ�� �Ѿ˵�� ���� �浹�� �˻��ϰ�
		* �浹�ߴٸ� Active�� false ��Ű�� ��� Ŭ���̾�Ʈ�鿡�� �浹�ߴٴ� ��Ŷ�� ������
		*/
		for (auto& cl : clients) {
			if (cl._state != US_INGAME) continue;
			for (int i{}; i < MAX_BULLET_NUM; ++i) {
				if (false == cl.bullet[i].GetisActive()) continue;
				// �Ѿ˰� ��� �浹 ó��
				for (int j{}; j < Map_infromation.block_num; ++j) {
					if (false == Map_infromation.Map_Block[j].GetisActive()) continue;
					if (CollisionCheck_objects(cl.bullet[i].GetPosition(), Map_infromation.Map_Block[j].GetPosition(),
						cl.bullet[i].GetRadius(), Map_infromation.Map_Block[j].GetRadius())){
						cl.bullet[i].SetisActive(false);
						Map_infromation.Map_Block[j].SetisActive(false);
						//cout << "�浹��" << endl;
						for (auto& send_cl : clients) {
							if (send_cl._state != US_INGAME) continue;
							send_cl.send_bullet_collision_packet(cl.bullet[i].GetbulletId(), Map_infromation.Map_Block[j].GetId(),
								cl._id, Map_infromation.Map_Block[j].GetBlockType());
						}
					}
				}
				// �÷��̾�� �Ѿ��� �浹 ó��
				if (false == cl.bullet[i].GetisActive()) continue;
				for (auto& other_player : clients) {
					if (other_player._state != US_INGAME) continue;
					if (other_player._id == cl._id) continue;
					if (true == other_player.isDeath) continue;
					if (CollisionCheck_Person(cl.bullet[i].GetPosition(), other_player.pos,
						cl.bullet[i].GetRadius(), other_player._player_radius)) {
						cl.bullet[i].SetisActive(false);
						
						// �¾��� �� ü���� ��´�.
						// ü�� ó�� ��� �Ұǰ�? -> �̰� mutex �ɾ�� ��? �ٵ� �̰͵� mutex �ɰŸ� ��ġ�� �ɾ�� �ϴ°� �ƴ�? �ٵ� �׷� ���� �� �����°� �ƴ�?
						if (false == other_player.isinvincible) {
							//cout << "�÷��̾� [" << other_player._id << "] �¾Ҵ�" << endl;
							//other_player.isinvincible = true;
							other_player.hp -= 1;

							// ����ִٸ� �¾Ҵٴ� ��Ŷ��, �׾��ٸ� �׾��ٴ� ��Ŷ�� ������
							if (other_player.hp > 0) {
								for (auto& send_cl : clients) {
									if (send_cl._state != US_INGAME) continue;
									send_cl.send_hit_packet(cl.bullet[i].GetbulletId(), cl._id, other_player._id);
								}
							}
							else
							{
								// �׾��� �� ó�� ���?
								other_player.isDeath = true;
								for (auto& send_cl : clients) {
									if (send_cl._state != US_INGAME) continue;
									send_cl.send_dead_packet(cl.bullet[i].GetbulletId(), cl._id, other_player._id);
								}
							}
						}
					}
				}
			}
		}
		// ������ ����Ǿ� �ִ� Ŭ���̾�Ʈ�� �ð� ���ϱ�, �ϴ� �����ð� 3��
		// ���� �÷��̾ �ִٸ� Ÿ�̸� ������. ������ �ð� 5��
		for (auto& cl : clients) {
			if (cl._state != US_INGAME) continue;
			if (false == cl.isinvincible && false == cl.isDeath) continue;
			else if (true == cl.isinvincible) {
				cl.invincible_time += server_timer.GetTimeElapsed();
				if (cl.invincible_time >= 3.f) {
					cl.isinvincible = false;
					cl.invincible_time = 0.f;
				}
			}
			else if (true == cl.isDeath) {
			cl.Death_time += server_timer.GetTimeElapsed();
				if (cl.Death_time >= 5.f) {
					cl.isDeath = false;
					cl.Death_time = 0.f;
					cl.hp = 10;

					XMFLOAT3 random_pos = physics_engine.PickPos();

					//cout << "�÷��̾� [" << cl._id << "] ��Ȱ" << endl;
					for (auto& send_cl : clients) {
						if (send_cl._state != US_INGAME)continue;
						send_cl.send_respawn_packet(random_pos.x, random_pos.y, random_pos.z, cl._id);
					}
				}
			}
		}
	}
}

void do_timer()
{
	while (true) {
		
	}
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	iocp_h = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), iocp_h, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_over._overlapped_type = OT_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_over._over);

	vector<thread> worker_threads;
	int thread_num = thread::hardware_concurrency();
	for (int i{}; i < thread_num - 1; ++i)
		worker_threads.emplace_back(worker_thread, iocp_h);
	
	// ��� ������
	thread physics_thread{ Physics_Calculation_thread };
	physics_thread.join();

	// Ÿ�̸ӷ� ó���� �͵� ó��
	/*thread timer_thread{ do_timer };

	timer_thread.join();*/
	for (auto& thread : worker_threads) 
		thread.join();

	closesocket(g_s_socket);
	WSACleanup();
}