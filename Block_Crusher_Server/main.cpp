#include "header.h"
#include "protocol.h"
#include "Overlapped.h"
#include "User_Interface.h"
#include "Timer.h"
#include "Map.h"
#include "Physics.h"
#include "Room.h"

using namespace std;
using namespace chrono;

HANDLE iocp_h;
SOCKET g_s_socket, g_c_socket;
Overlapped g_over;
Timer server_timer;

array<User_Interface, MAX_USER> clients;
array<short, MAX_USER> clients_room;
array<Room, MAX_ROOM> rooms;
mutex room_lock;

//Map Map_infromation;
Physics physics_engine;
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

//atomic_int user_number = 0;

auto start_time = system_clock::now();

SQLHENV henv;
SQLHDBC hdbc;       // connect �� �� �ʿ��� �ڵ�

int set_client_id()
{
	for (int i{}; i < MAX_USER; ++i) {
		lock_guard<mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == US_EMPTY) {
			clients[i]._state = US_CONNECTING;
			return i;
		}
	}
	return -1;
}

void disconnect(int c_id)
{
	// �뿡���� ���������� �ؾ���
	for (auto& r : rooms) {
		if (r.PlayerOut(c_id)) {
			clients_room[c_id] = -1;
			break;
		}
	}
	closesocket(clients[c_id]._socket);
	cout << "[" << c_id << "] Ż����" << endl;
	//user_number--;
	// ������ ��µ��� �ʰԵ� �ؾ���
	lock_guard<mutex> ll{ clients[c_id]._s_lock };
	clients[c_id]._state = US_EMPTY;
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

//bool scoreCalculation()
//{
//	int num = 0;
//	int max_score = 0;
//	for (auto& cl : clients) {
//		{
//			lock_guard<mutex> ll{ cl._s_lock };
//			if (cl._state != US_INGAME) continue;
//		}
//		if (cl.score != -1) {
//			num++;
//			if (max_score < cl.score)
//				max_score = cl.score;
//		}
//	}
//	if (num == 3) {
//		for (auto& cl : clients) {
//			if (cl.score == max_score)
//				cl.isWin = true;
//		}
//		return true;
//	}
//	return false;
//}

void packet_process(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		cout << c_id << " ���� �Ϸ�" << endl;
		//clients[c_id]._state = US_INGAME;
		
		// ��� �ִ� ���� ã�Ƽ� ���� �ڵ� �ۼ� �ؾ� ��
		/*if (false == Map_infromation.isActive) Map_infromation.isActive = true;
		if (Map_infromation.player_num < 6) {
			Map_infromation.player_num++;
			if (Map_infromation.player_num == 6) {

			}
		}*/
		clients[c_id].send_login_info_packet();

		// room ����
		// Ŭ���̾�Ʈ���� ���� �����ϴ� �ڵ带 �ۼ��ϴ� ��� ���� �ʿ�. ������ �׳� ��� �ִ� ���� ������ ����
		for (Room& r : rooms) {
			// �� ���� ã�´�
			{
				lock_guard<mutex> ll{ r._r_lock };
				if (r.GetRoomState() != RS_WAITING) continue;
			}
			// �÷��̾ �ִ� �Ϳ� �����ߴ�
			// �÷��̾ �ְ� ����(6��)�� á��
			// �뿡 �ִ� ��� �÷��̾� id���� ��Ŷ�� ������
			if (r.PlayerIn(c_id)) {
				{
					//lock_guard<mutex> ll{ room_lock };
					clients_room[c_id] =  r.GetRoomNum();
				}
				if (r.GetRoomState() == RS_READY) {
					int* r_in_id = r.GetPlayerId();
					for (int i{}; i < MAX_PLAYER; ++i) {
						{
							lock_guard<mutex> ll{ clients[i]._s_lock };
							clients[i]._state = US_INGAME;
						}
						if (r_in_id[i] != -1)
							clients[r_in_id[i]].send_start_packet(r.GetMapKey());
					}
					// room ���� �����ؾ� ��
					r.SetRoomState(RS_INGAME);
				}
				break;
			}
		}
		//user_number++;
		/*if (user_number == 3)
			for (auto& cl : clients) {
				lock_guard<mutex> ll{ cl._s_lock };
				if (cl._state == US_CONNECTING) {
					cl._state = US_INGAME;
					cl.send_start_packet(Map_infromation.MapChar);
				}
			}*/
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		// ��Ŷ���� ������ ���⿡ ���� ��ġ �̵� �ڵ�
		XMFLOAT3 accept_position{ p->x, p->y, p->z };
		clients[c_id].pos = accept_position;
		clients[c_id].cx = p->cxDelta;
		clients[c_id].cy = p->cyDelta;
		//if (c_id != 0 && c_id != 1 && c_id != 2) cout << c_id << endl;
		//cout << "[" << c_id << "] " << p->animation_state << endl;

		//cout << "[" << c_id << "] Ŭ���̾�Ʈ ���� ������ : " << p->frame_num << endl;
		/*auto now_time = system_clock::now();
		auto exec_time = now_time - start_time;
		auto ms = duration_cast<milliseconds>(exec_time).count();*/
		//cout << ms << endl;

		// �ٸ� Ŭ���̾�Ʈ�鿡�� �Ѹ���
		/*for (auto& cl : clients) {
			{
				lock_guard<mutex> ll{ cl._s_lock };
				if (cl._state != US_INGAME) continue;
			}
			if (cl._id == c_id) continue;
			cl.send_move_packet(clients.data(), c_id, p->animation_state);
	}*/
		// �� �ڵ尡 ������; ���� �ʿ伺 ����
		/*for (auto& r : rooms) {
			if (-1 == r.FindPlayer(c_id)) continue;
			int* same_room_player = r.GetPlayerId();

			for (int round{}; round < MAX_PLAYER; ++round) {
				int player_id = *same_room_player;
				if (player_id == -1) continue;
				clients[player_id].send_move_packet(clients.data(), c_id, p->animation_state);
				same_room_player += 1;
			}
		}*/
		short room_number = clients_room[c_id];
		int* same_room_player = rooms[room_number].GetPlayerId();

		for (int round{}; round < MAX_PLAYER; ++round) {
			int player_id = same_room_player[round];
			if (player_id == -1) continue;
			if (player_id == c_id) continue;
			clients[player_id].send_move_packet(clients.data(), c_id, p->animation_state);
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

				//if (true == clients[c_id].bullet[i].GetisActive()) cout << i << "�� �����̰� �ߴµ�?" << endl;
				// ���� �뿡 �ִ� �÷��̾�Ը� �Ѿ� ������ ������ �Ѵ�
				//for (auto& cl : clients) {
				//	if (cl._state != US_INGAME) continue;
				//	if (cl._id == c_id) continue;
				//	//cout << "�������� " << cl._id << "���� " << c_id << "�� ���� ���ٴ°� ��������ϴ�" << endl;
				//	cl.send_bullet_add_packet(clients.data(), c_id, i);
				//}

				short room_number = clients_room[c_id];
				int* same_room_player = rooms[room_number].GetPlayerId();

				for (int round{}; round < MAX_PLAYER; ++round) {
					int player_id = same_room_player[round];
					//cout << player_id << endl;
					if (player_id == -1) continue;
					if (player_id == c_id) continue;
					clients[player_id].send_bullet_add_packet(clients.data(), c_id, i);
				}
				/*for (auto& r : rooms) {
					if (-1 == r.FindPlayer(c_id)) continue;
					int* same_room_player = r.GetPlayerId();
					
					for (int round{}; round < MAX_PLAYER; ++round) {
						int player_id = *same_room_player;
						if (player_id == -1) continue;
						clients[player_id].send_bullet_add_packet(clients.data(), c_id, i);
						same_room_player += 1;
					}
				}*/
				break;
			}
		}
		break;
	}
	case CS_FALL: {
		if (false == clients[c_id].isDeath) {
			CS_FALL_PACKET* p = reinterpret_cast<CS_FALL_PACKET*>(packet);
			clients[c_id].isDeath = true;
			//cout << "�������ٴ� ��Ŷ�� ����" << endl;
			TIMER_EVENT ev{ c_id, chrono::system_clock::now() + 5s, EV_RESPAWN, 0 };
			timer_queue.push(ev);
			
			short room_number = clients_room[c_id];
			int* same_room_player = rooms[room_number].GetPlayerId();

			for (int round{}; round < MAX_PLAYER; ++round) {
				int player_id = *same_room_player;
				if (player_id == -1) continue;
				if (player_id == c_id) continue;
				clients[player_id].send_fall_packet(c_id);
				same_room_player += 1;
			}

			/*for (auto& r : rooms) {
				if (-1 == r.FindPlayer(c_id)) continue;
				int* same_room_player = r.GetPlayerId();

				for (int round{}; round < MAX_PLAYER; ++round) {
					int player_id = *same_room_player;
					if (player_id == -1) continue;
					if (player_id == c_id) continue;
					clients[player_id].send_fall_packet(c_id);
					same_room_player += 1;
				}
			}*/
			/*for (auto& cl : clients) {
				if (cl._state != US_INGAME) continue;
				if (cl._id == c_id) continue;
				cl.send_fall_packet(c_id);
			}*/
		}
		break;
	}
	case CS_SCORE: {
		CS_SCORE_PACKET* p = reinterpret_cast<CS_SCORE_PACKET*>(packet);
		short room_num = clients_room[c_id];
		clients[c_id].score = p->score;
		if (-1 != rooms[room_num].scoreCalculate(clients[c_id].score)) {
			/*for (auto& cl : clients) {
				if (cl._state != US_INGAME) continue;
				cl.send_result_packet();
			}*/
			int* room_member = rooms[room_num].GetPlayerId();
			for (int i{}; i < MAX_PLAYER; ++i) {
				if (-1 != room_member[i])
				{
					if (rooms[room_num].max_score == clients[room_member[i]].score)
					{
						clients[room_member[i]].isWin = true;
					}
					else
					{
						clients[room_member[i]].isWin = false;
					}
					clients[room_member[i]].send_result_packet();
				}
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
					// �ʱⰪ ����
					clients[client_id].pos.x = 0;
					clients[client_id].pos.y = 0;
					clients[client_id]._id = client_id;
					clients[client_id]._prev_remain = 0;
					clients[client_id]._socket = g_c_socket;

					// ���� ��ȯ(������)
					//clients[client_id]._state = US_CONNECTING;

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
			case OT_RESPAWN:
			{
				// Ÿ�̸� ť���� ������ �϶�� �޽����� �����ٸ� 
				clients[key].hp = 10;
				clients[key].isDeath = false;

				XMFLOAT3 random_pos = physics_engine.PickPos();

				for (auto& send_cl : clients) {
					{
						lock_guard<mutex> ll{ send_cl._s_lock };
						if (send_cl._state != US_INGAME) continue;
					}
					cout << "[" << send_cl._id << "] ���� " << key << "�� ��Ȱ�ߴٰ� ����" << endl;
					send_cl.send_respawn_packet(random_pos.x, random_pos.y, random_pos.z, key);
				}
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
			//cout << "�ΰ��ӵȰ� �ֱ� ��?" << endl;
			for (int i{}; i < MAX_BULLET_NUM; ++i) {
				if (cl.bullet[i].GetisActive()) {
					//cout << i << endl;
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

		for (auto& r : rooms) {
			if (r.GetRoomState() != RS_INGAME) continue;
			int* room_player_ids = r.GetPlayerId();

			for (int i{}; i < MAX_PLAYER; ++i) {
				// �÷��̾� ������ ����
				int check_id = room_player_ids[i];
				if (check_id == -1) continue;
				// �ִ� �÷��̾� �Ѿ˵� �浹 �˻�
				for (int bullet_num{}; bullet_num < MAX_BULLET_NUM; ++bullet_num) {
					if (false == clients[check_id].bullet[bullet_num].GetisActive()) continue;
					//cout << bullet_num << " �Ѿ��� �ֱ� ��" << endl;
					//cout << bullet_num << "// x : " << clients[check_id].bullet[bullet_num].GetPosition().x << ", y : " << clients[check_id].bullet[bullet_num].GetPosition().y << ", z : " << clients[check_id].bullet[bullet_num].GetPosition().z << endl;
					// �Ѿ�, ��� �浹 ó��
					// �켱�� ��� �Ѿ�, ��� �浹 ó����, ����ȭ �ʿ�
					//cout << r.map_information.block_num << endl;
					for (int map_block_num{}; map_block_num < r.map_information.block_num; ++map_block_num) {
						if (false == r.map_information.Map_Block[map_block_num].GetisActive()) continue;
						//cout << map_block_num << "// x : " << r.map_information.Map_Block[map_block_num].GetPosition().x << ", y : " << r.map_information.Map_Block[map_block_num].GetPosition().y << ", z : " << r.map_information.Map_Block[map_block_num].GetPosition().z << endl;
						//cout << map_block_num << "��� �ֱ� ��" << endl;
						//cout << "�Ѿ� rad" << clients[check_id].bullet[bullet_num].GetRadius() << ", ��� rad " << r.map_information.Map_Block[map_block_num].GetRadius() << endl;
						if (CollisionCheck_objects(clients[check_id].bullet[bullet_num].GetPosition(), r.map_information.Map_Block[map_block_num].GetPosition(),
							clients[check_id].bullet[bullet_num].GetRadius(), r.map_information.Map_Block[map_block_num].GetRadius())) {
							// �浹�ߴٸ� �Ѿ�, ��� ��Ȱ��ȭ, �浹������ �˸�
							//cout << "�Ѿ��̶� ��� �浹��" << endl;
							clients[check_id].bullet[bullet_num].SetisActive(false);
							r.map_information.Map_Block[map_block_num].SetisActive(false);

							for (int round{}; round < MAX_PLAYER; ++round) {
								if (room_player_ids[round] == -1) continue;
								clients[room_player_ids[round]].send_bullet_collision_packet(clients[check_id].bullet[bullet_num].GetbulletId(),
									r.map_information.Map_Block[map_block_num].GetId(),
									clients[check_id]._id,
									r.map_information.Map_Block[map_block_num].GetBlockType());
							}
						}
					}

					// �Ѿ˰� �÷��̾� �浹 Ȯ��
					for (int round{}; round < MAX_PLAYER; ++round) {
						int other_id = room_player_ids[round];
						if (other_id == -1) continue;
						if (other_id == check_id) continue;
						if (true == clients[other_id].isDeath) continue;
						// �Ѿ˰� �ٸ� �÷��̾ �浹�ߴٸ�,
						if (CollisionCheck_Person(clients[check_id].bullet[bullet_num].GetPosition(), clients[other_id].pos,
							clients[check_id].bullet[bullet_num].GetRadius(), clients[other_id]._player_radius)) {
							//cout << "�÷��̾� ����" << endl;
							// �Ѿ��� ��Ȱ��ȭ, hp �϶�
							clients[check_id].bullet[bullet_num].SetisActive(false);
							clients[other_id].hp -= 1;
							// hp�� 0�̸� �׾��ٰ�, �ƴϸ� �¾Ҵٴ� ��Ŷ ����

							if (clients[other_id].hp > 0) {
								for (int send_round{}; send_round < MAX_PLAYER; ++send_round) {
									int send_id = room_player_ids[send_round];
									if (send_id == -1) continue;
									//cout << send_id << endl;
									cout << clients[check_id]._id << "�� " << clients[other_id]._id << "�� " << clients[check_id].bullet[bullet_num].GetbulletId() << "�� �Ѿ˷� ����" << endl;
									clients[send_id].send_hit_packet(clients[check_id].bullet[bullet_num].GetbulletId(), clients[check_id]._id, clients[other_id]._id);
								}
							}
							else {
								TIMER_EVENT ev{ clients[other_id]._id, chrono::system_clock::now() + 5s, EV_RESPAWN, 0};
								timer_queue.push(ev);
								clients[other_id].isDeath = true;
								for (int send_round{}; send_round < MAX_PLAYER; ++send_round) {
									int send_id = room_player_ids[send_round];
									if (send_id == -1) continue;
									clients[send_id].send_dead_packet(clients[check_id].bullet[bullet_num].GetbulletId(), clients[check_id]._id, clients[other_id]._id);
								}
							}
						}
					}
				}
			}
		}

		//for (auto& cl : clients) {
		//	if (cl._state != US_INGAME) continue;
		//	for (int i{}; i < MAX_BULLET_NUM; ++i) {
		//		if (false == cl.bullet[i].GetisActive()) continue;
		//		// �Ѿ˰� ��� �浹 ó��
		//		for (int j{}; j < Map_infromation.block_num; ++j) {
		//			if (false == Map_infromation.Map_Block[j].GetisActive()) continue;
		//			if (CollisionCheck_objects(cl.bullet[i].GetPosition(), Map_infromation.Map_Block[j].GetPosition(),
		//				cl.bullet[i].GetRadius(), Map_infromation.Map_Block[j].GetRadius())){
		//				cl.bullet[i].SetisActive(false);
		//				Map_infromation.Map_Block[j].SetisActive(false);
		//				//cout << "�浹��" << endl;
		//				for (auto& send_cl : clients) {
		//					if (send_cl._state != US_INGAME) continue;
		//					send_cl.send_bullet_collision_packet(cl.bullet[i].GetbulletId(), Map_infromation.Map_Block[j].GetId(),
		//						cl._id, Map_infromation.Map_Block[j].GetBlockType());
		//				}
		//			}
		//		}
		//		// �÷��̾�� �Ѿ��� �浹 ó��
		//		if (false == cl.bullet[i].GetisActive()) continue;
		//		for (auto& other_player : clients) {
		//			if (other_player._state != US_INGAME) continue;
		//			if (other_player._id == cl._id) continue;
		//			if (true == other_player.isDeath) continue;
		//			if (CollisionCheck_Person(cl.bullet[i].GetPosition(), other_player.pos,
		//				cl.bullet[i].GetRadius(), other_player._player_radius)) {
		//				cl.bullet[i].SetisActive(false);
		//				
		//				// �¾��� �� ü���� ��´�.
		//				// ü�� ó�� ��� �Ұǰ�? -> �̰� mutex �ɾ�� ��? �ٵ� �̰͵� mutex �ɰŸ� ��ġ�� �ɾ�� �ϴ°� �ƴ�? �ٵ� �׷� ���� �� �����°� �ƴ�?
		//				if (false == other_player.isinvincible) {
		//					//cout << "�÷��̾� [" << other_player._id << "] �¾Ҵ�" << endl;
		//					//other_player.isinvincible = true;
		//					other_player.hp -= 1;

		//					// ����ִٸ� �¾Ҵٴ� ��Ŷ��, �׾��ٸ� �׾��ٴ� ��Ŷ�� ������
		//					if (other_player.hp > 0) {
		//						for (auto& send_cl : clients) {
		//							if (send_cl._state != US_INGAME) continue;
		//							send_cl.send_hit_packet(cl.bullet[i].GetbulletId(), cl._id, other_player._id);
		//						}
		//					}
		//					else
		//					{
		//						// �׾��� �� ó�� ���?
		//						// Ÿ�̸� �����忡�� �ϰ��� �༭ 5�� ������ �϶� ��
		//						TIMER_EVENT ev{ other_player._id, chrono::system_clock::now() + 5s, EV_RESPAWN, 0 };
		//						timer_queue.push(ev);
		//						other_player.isDeath = true;
		//						for (auto& send_cl : clients) {
		//							if (send_cl._state != US_INGAME) continue;
		//							send_cl.send_dead_packet(cl.bullet[i].GetbulletId(), cl._id, other_player._id);
		//						}
		//					}
		//				}
		//			}
		//		}
		//	}
		//}
		// //������ ����Ǿ� �ִ� Ŭ���̾�Ʈ�� �ð� ���ϱ�, �ϴ� �����ð� 3��
		// //���� �÷��̾ �ִٸ� Ÿ�̸� ������. ������ �ð� 5��
		//for (auto& cl : clients) {
		//	{
		//		lock_guard<mutex> ll{ cl._s_lock };
		//		if (cl._state != US_INGAME) continue;
		//	}
		//	bool exp = true;
		//	if (false == cl.isinvincible && false == cl.isDeath) continue;
		//	/*else if (true == cl.isinvincible) {
		//		cl.invincible_time += server_timer.GetTimeElapsed();
		//		if (cl.invincible_time >= 3.f) {
		//			cl.isinvincible = false;
		//			cl.invincible_time = 0.f;
		//		}
		//	}*/
		//	else if (true == cl.isDeath) {
		//	cl.Death_time += server_timer.GetTimeElapsed();
		//		if (cl.Death_time >= 5.f) {
		//			cl.isDeath = false;
		//			cl.Death_time = 0.f;
		//			cl.hp = 10;

		//			XMFLOAT3 random_pos = physics_engine.PickPos();

		//			//cout << "�÷��̾� [" << cl._id << "] ��Ȱ" << endl;
		//			for (auto& send_cl : clients) {
		//				if (send_cl._state != US_INGAME)continue;
		//				cout << "[" << send_cl._id << "] ���� " << cl._id << "�� ��Ȱ�ߴٰ� ����" << endl;
		//				send_cl.send_respawn_packet(random_pos.x, random_pos.y, random_pos.z, cl._id);
		//			}
		//		}
		//	}
		//}
	}
}

void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void InitDB()
{
	SQLRETURN retcode;

	setlocale(LC_ALL, "korean");
	std::wcout.imbue(std::locale("korean"));

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2024_WINTER_ODBC", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "ODBC ����. DB �ʱ�ȭ �Ϸ�" << endl;
				}
			}
		}
	}
}

bool Search_Id(const char* id)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR wstr[100];
	SQLWCHAR user_id[20];

	memset(wstr, 0, sizeof(wstr));
	wsprintf(wstr, L"EXEC id_search %S", id);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wstr, SQL_NTS);
}

bool Search_User(const char* id, const char* password)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR wstr[100];
	SQLWCHAR user_id[20];
	SQLWCHAR user_password[20];
	SQLINTEGER user_high_score;
	SQLLEN len[3];

	memset(wstr, 0, sizeof(wstr));
	wsprintf(wstr, L"EXEC Login %S, %S", id, password);
	//wsprintf(wstr, L"EXEC Nest_search_user %S", id);
	//cout << "�� ������ ����?" << endl;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wstr, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &user_id, 20, &len[0]);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_password, 20, &len[1]);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_high_score, 4, &len[2]);

		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			show_error(hstmt, SQL_FETCH_ABSOLUTE, retcode);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			cout << "����" << endl;
		}
		else
		{
			cout << "����" << endl;
		}
	}
	else
	{
		show_error(hstmt, SQL_HANDLE_STMT, retcode);
	}
	return true;
}

void InitRoom()
{
	int room_num = 0;
	for (Room& r : rooms) {
		r.SetRoomNum(room_num);
		room_num++;
	}

	for (char c : clients_room) {
		c = -1;
	}
}

void do_timer()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		//cout << "Ÿ�̸� ������ ���� ��~" << endl;
		if (true == timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);			// �� �κ��� ����ȭ�� �ʿ���
				// �ְ� ���� �۾� ���� �� �� �ִ� ����� ����?
				//cout << "�ְ� ���� ��" << endl;
				this_thread::sleep_for(1ms);
				continue;
			}
			switch (ev.event_id) {
			case EV_RESPAWN: {
				cout << "Ÿ�̸� ������ �ð� �� ��" << endl;
				Overlapped* ov = new Overlapped;
				ov->_overlapped_type = OT_RESPAWN;
				PostQueuedCompletionStatus(iocp_h, 1, ev.obj_id, &ov->_over);
				break;
			}
			}
			continue;
		}
		this_thread::sleep_for(1ms);		// ť�� �۾��� ������ ��� ����ߴٰ� �ٽ� ����
	}
}

void do_db()
{
	//Search_User("dleh99", "123");
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

	InitDB();
	InitRoom();	

	vector<thread> worker_threads;
	int thread_num = thread::hardware_concurrency();
	for (int i{}; i < thread_num - 3; ++i)
		worker_threads.emplace_back(worker_thread, iocp_h);
	
	// ��� ������
	thread physics_thread{ Physics_Calculation_thread };
	
	// Ÿ�̸� ������
	thread timer_thread{ do_timer };

	// DB ������
	thread db_thread{ do_db };

	db_thread.join();
	timer_thread.join();
	physics_thread.join();
	for (auto& thread : worker_threads) 
		thread.join();

	// DB ����
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);

	closesocket(g_s_socket);
	WSACleanup();
}