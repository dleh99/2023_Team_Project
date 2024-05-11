#include "header.h"
#include "protocol.h"
#include "Overlapped.h"
#include "User_Interface.h"
#include "Timer.h"
#include "Map.h"
#include "Physics.h"
#include "Room.h"
#include "DB.h"

using namespace std;
using namespace chrono;

HANDLE iocp_h;
SOCKET g_s_socket, g_c_socket;
Overlapped g_over;
Timer server_timer;
DB db_controll;

array<User_Interface, MAX_USER> clients;
array<short, MAX_USER> clients_room;
array<Room, MAX_ROOM> rooms;
mutex room_lock;

//Map Map_infromation;
Physics physics_engine;
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
concurrency::concurrent_priority_queue<DB_EVENT> db_queue;

auto start_time = system_clock::now();

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
	// DB���� login_state ����
	if (clients[c_id].login_id != L"") {
		DB_EVENT ev{ clients[c_id]._id, chrono::system_clock::now(), LOGIN_DISCONNECT, clients[c_id].login_id, L"" };
		db_queue.push(ev);
	}

	// �뿡���� ���������� �ؾ���
	short room_num = clients_room[c_id];
	if (room_num != -1) {
		if (rooms[room_num].PlayerOut(c_id))
			cout << "[" << c_id << "] �뿡�� Ż�� �Ϸ�" << endl;
	}

	// �� ��ȣ ����� �ʱ�ȭ
	clients_room[c_id] = -1;
	
	// ���� ����
	closesocket(clients[c_id]._socket);
	cout << "[" << c_id << "] Ż����" << endl;
	//user_number--;
	// ������ ��µ��� �ʰԵ� �ؾ���
	lock_guard<mutex> ll{ clients[c_id]._s_lock };
	clients[c_id]._state = US_EMPTY;
	clients[c_id]._room_id = -1;
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

void checking_room(int key)
{
	short room_num = clients_room[key];

	{
		lock_guard<mutex> ll{ rooms[room_num]._r_lock };
		if (RS_READY != rooms[room_num].GetRoomState()) return;
	}

	//cout << room_num  << "�� �� �ο� �� ��" << endl;

	rooms[room_num].SetRoomState(RS_INGAME);
	int* room_mambers = rooms[room_num].GetPlayerId();
	//cout << "������" << endl;
	for (int i{}; i < MAX_PLAYER; ++i) {
		if (room_mambers[i] != -1) {
			//cout << rooms[room_num].GetMapKey() << endl;
			{
				lock_guard<mutex> ll{ clients[room_mambers[i]]._s_lock };
				clients[room_mambers[i]]._state = US_INGAME;
			}
			clients[room_mambers[i]]._room_id = i;
			//cout << room_mambers[i] << "���� id ���� : " << i << endl;
			XMFLOAT3 start_pos = physics_engine.StartPos(i);

			clients[room_mambers[i]].send_start_packet(rooms[room_num].GetMapKey(), i, start_pos.x, start_pos.y, start_pos.z);
		}
	}
}

void FindEmptyRoom(int c_id)
{
	for (auto& r : rooms) {
		if (r.GetRoomState() == RS_INGAME) continue; 
		if (false == r.PlayerIn(c_id)) continue;
		clients_room[c_id] = r.GetRoomNum();
		checking_room(c_id);
		cout << clients_room[c_id] << "�� �뿡 ������" << endl;
		break;
	}
}

void packet_process(int c_id, char* packet)
{
	//cout << "��Ŷ �ؼ�" << endl;
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		//cout <<  packet[0] << ", ";
		//wcout << "ID : " << p->id << ", PW : " << p->password << ", RN : " << p->room_num << endl;
		//clients[c_id].send_login_info_packet();

		/*
		�� ��ȣ �Է��ϸ� �α��� �ǰԲ� �ϴ� �κ�

		//if (p->room_num > 332 || p->room_num < 0) {
		//	clients[c_id].send_login_fail_packet(LS_OUTOFROOM);
		//}
		//else {
		//	if (false == rooms[p->room_num].PlayerIn(c_id)) {
		//		clients[c_id].send_login_fail_packet(LS_FULLROOM);
		//	}
		//	else {
		//		//cout << "��� ����" << endl;
		//		//wcout << L"���̵� : " << p->id << L", ��й�ȣ : " << p->password << L", �� ��ȣ : " << p->room_num << endl;
		//		//cout << rooms[p->room_num].GetRoomNum() << "�� �� �ο��� : " << rooms[p->room_num].clients_number << endl;
		//		clients_room[c_id] = p->room_num;
 	//			clients[c_id].login_id = p->id;
		//		DB_EVENT ev{ clients[c_id]._id, chrono::system_clock::now(), TRY_LOGIN, p->id, p->password };
		//		db_queue.push(ev);
		//	}
		//}
		*/

		// DB���� �α��� �õ�
		clients[c_id].login_id = p->id;
		DB_EVENT ev{ clients[c_id]._id, chrono::system_clock::now(), TRY_LOGIN, p->id, p->password };
		db_queue.push(ev);

		break;
	}
	case CS_MATCH: {
		// ��� �ִ� �� ã��
		FindEmptyRoom(c_id);
		clients[c_id].send_match_finish_packet(clients_room[c_id]);
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		// ��Ŷ���� ������ ���⿡ ���� ��ġ �̵� �ڵ�
		XMFLOAT3 accept_position{ p->x, p->y, p->z };
		clients[c_id].pos = accept_position;
		clients[c_id].cx = p->cxDelta;
		clients[c_id].cy = p->cyDelta;

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
		for (int i{}; i < MAX_BULLET_NUM; ++i) {
			if (false == clients[c_id].bullet[i].GetisActive()) {
				clients[c_id].bullet[i].SetisActive(true);
				clients[c_id].bullet[i].SetPosition(p->s_x, p->s_y, p->s_z);
				clients[c_id].bullet[i].SetBulletVec(p->b_x, p->b_y, p->b_z);
				clients[c_id].bullet[i].SetBulletId(p->bullet_id);
				clients[c_id].bullet[i].SetBulletRange(p->s_x, p->s_y, p->s_z);

				short room_number = clients_room[c_id];
				int* same_room_player = rooms[room_number].GetPlayerId();

				for (int round{}; round < MAX_PLAYER; ++round) {
					int player_id = same_room_player[round];
					//cout << player_id << endl;
					if (player_id == -1) continue;
					if (player_id == c_id) continue;
					//cout << player_id << "�� ���� ����" << endl;
					clients[player_id].send_bullet_add_packet(clients.data(), c_id, i);
				}
				break;
			}
		}
		break;
	}
	case CS_FALL: {
		if (false == clients[c_id].isDeath) {
			CS_FALL_PACKET* p = reinterpret_cast<CS_FALL_PACKET*>(packet);
			clients[c_id].isDeath = true;
			TIMER_EVENT ev{ c_id, chrono::system_clock::now() + 5s, EV_RESPAWN, 0 };
			timer_queue.push(ev);
			
			short room_number = clients_room[c_id];
			int* same_room_player = rooms[room_number].GetPlayerId();

			for (int round{}; round < MAX_PLAYER; ++round) {
				int player_id = same_room_player[round];
				if (player_id == -1) continue;
				if (player_id == c_id) continue;
				clients[player_id].send_fall_packet(c_id);
			}
		}
		break;
	}
	case CS_SCORE: {
		CS_SCORE_PACKET* p = reinterpret_cast<CS_SCORE_PACKET*>(packet);
		short room_num = clients_room[c_id];
		clients[c_id].score = p->score;
		
		DB_EVENT ev{ clients[c_id]._id, chrono::system_clock::now(), UPDATE_SCORE, clients[c_id].login_id, clients[c_id].score };
		db_queue.push(ev);
		
		if (-1 != rooms[room_num].scoreCalculate(clients[c_id].score)) {
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

		//cout << static_cast<int>(key) << endl;

		// ������ ���� �� ó��
		if (FALSE == ret) {
			if (ex_over->_overlapped_type == OT_ACCEPT) std::cout << "Error of Accept";
			else {
				//std::cout << "Error on client [" << key << "] in GQCS" << std::endl;
				disconnect(static_cast<int>(key));
				if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
				continue;
			}
		}

		// �� �����Ͱ� ���� ��
		if ((byte_size == 0) && ((ex_over->_overlapped_type == OT_RECV) || (ex_over->_overlapped_type == OT_SEND))) {
			disconnect(static_cast<int>(key));
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

					cout << "���� ����" << endl;

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
				//cout << "���ú� ��" << endl;
				if (byte_size == 0)
				{
					disconnect(static_cast<int>(key));
					continue;
				}
				int remain_data = byte_size + clients[key]._prev_remain;
				char* p = ex_over->_send_buf;
				// ��Ŷ ������
				while (remain_data > 0) {
					//cout << "��Ŷ ������ ��" << endl;
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						packet_process(static_cast<int>(key), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				//cout << "���Դٸ�" << endl;
				// ������ ���ٸ�
				clients[key]._prev_remain = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->_send_buf, p, remain_data);
				}
				if (remain_data == 0) {
					clients[key]._prev_remain = 0;
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

				short room_number = clients_room[key];
				int* same_room_player = rooms[room_number].GetPlayerId();

				for (int round{}; round < MAX_PLAYER; ++round) {
					int player_id = same_room_player[round];
					if (player_id == -1) continue;
					//cout << player_id << "���� " << key << "�� ��Ƴ����� �˸�" << endl;
					clients[player_id].send_respawn_packet(random_pos.x, random_pos.y, random_pos.z, clients[key]._room_id);
				}
				delete ex_over;
				break;
			}
			case OT_SIGNUP:
			{
				cout << "worker thread���� ���ο� ���̵�" << endl;
				clients[key].send_login_success_packet(LS_SIGNUP);
				//checking_room(key);
				delete ex_over;
				break;
			}
			case OT_LOGIN_SUCCESS:
			{
				cout << "worker thread���� �α��� ����" << endl;
				clients[key].send_login_success_packet(LS_LOGIN_SUCCESS);
				//checking_room(key);
				delete ex_over;
				break;
			}
			case OT_LOGIN_FAIL:
			{
				cout << "worker thread���� �α��� ����" << endl;
				clients[key].send_login_fail_packet(LS_LOGIN_FAIL);
				if (clients_room[key] != -1) {
					rooms[clients_room[key]].PlayerOut(key);
					clients_room[key] = -1;
				}
				clients[key].login_id = L"";
				delete ex_over;
				break;
			}
			case OT_ALREADY_INGAME:
			{
				cout << "worker thread���� �̹� ������" << endl;
				clients[key].send_login_fail_packet(LS_ALREADY_INGAME);
				if (clients_room[key] != -1) {
					rooms[clients_room[key]].PlayerOut(key);
					clients_room[key] = -1;
				}
				clients[key].login_id = L"";
				delete ex_over;
				break;
			}
			case OT_DISCONNECT:
			{
				cout << "DB���� login state ���� �Ϸ�" << endl;
				delete ex_over;
				break;
			}
			case OT_UPDATE_SCORE:
			{
				cout << "DB���� [" << key << "]�� ���ھ� ������Ʈ �Ϸ�" << endl;
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
					// �Ѿ�, ��� �浹 ó��
					// �켱�� ��� �Ѿ�, ��� �浹 ó����, ����ȭ �ʿ�
					{
						//for (int map_block_num{}; map_block_num < r.map_information.block_num; ++map_block_num) {
						//	if (false == r.map_information.Map_Block[map_block_num].GetisActive()) continue;
						//	if (CollisionCheck_objects(clients[check_id].bullet[bullet_num].GetPosition(), r.map_information.Map_Block[map_block_num].GetPosition(),
						//		clients[check_id].bullet[bullet_num].GetRadius(), r.map_information.Map_Block[map_block_num].GetRadius())) {
						//		// �浹�ߴٸ� �Ѿ�, ��� ��Ȱ��ȭ, �浹������ �˸�
						//		//cout << "�浹��" << endl;
						//		clients[check_id].bullet[bullet_num].SetisActive(false);
						//		r.map_information.Map_Block[map_block_num].SetisActive(false);

						//		for (int round{}; round < MAX_PLAYER; ++round) {
						//			if (room_player_ids[round] == -1) continue;
						//			clients[room_player_ids[round]].send_bullet_collision_packet(clients[check_id].bullet[bullet_num].GetbulletId(),
						//				r.map_information.Map_Block[map_block_num].GetId(),
						//				clients[check_id]._room_id,
						//				r.map_information.Map_Block[map_block_num].GetBlockType());
						//		}
						//	}
						//}
					}

					// �Ѿ��� ���� �ִ� ���� �˻�
					Range_Pos temp = clients[check_id].bullet[bullet_num].GetBulletRange();

					// �� ������ ����� �ֳ�? ������ �Ѿ
					if (0 <= temp.x && temp.x < 50 && 0 <= temp.y && temp.y < 20 && 0 <= temp.z && temp.z < 50) {
						if (true == r.map_information.Map_B[temp.z + 50 * temp.x][temp.y].GetisActive()) {
							//cout << temp.x << ", " << temp.y << ", " << temp.z << endl;
							if (CollisionCheck_objects(clients[check_id].bullet[bullet_num].GetPosition(), r.map_information.Map_B[temp.z + 50 * temp.x][temp.y].GetPosition(),
								clients[check_id].bullet[bullet_num].GetRadius(), r.map_information.Map_B[temp.z + 50 * temp.x][temp.y].GetRadius())) {
								clients[check_id].bullet[bullet_num].SetisActive(false);
								r.map_information.Map_B[temp.z + 50 * temp.x][temp.y].SetisActive(false);

								for (int round{}; round < MAX_PLAYER; ++round) {
									if (room_player_ids[round] == -1) continue;
									clients[room_player_ids[round]].send_bullet_collision_packet(clients[check_id].bullet[bullet_num].GetbulletId(),
										r.map_information.Map_B[temp.z + 50 * temp.x][temp.y].GetId(),
										clients[check_id]._room_id,
										r.map_information.Map_B[temp.z + 50 * temp.x][temp.y].GetBlockType());
								}

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
							// �Ѿ��� ��Ȱ��ȭ, hp �϶�
							//cout << "�÷��̾� [" << clients[check_id]._id << "] �� �Ѿ��� �÷��̾� [" << clients[other_id]._id << "] �� ������ϴ�" << endl;
							clients[check_id].bullet[bullet_num].SetisActive(false);
							clients[other_id].hp -= 1;
							
							// hp�� 0�̸� �׾��ٰ�, �ƴϸ� �¾Ҵٴ� ��Ŷ ����
							if (clients[other_id].hp > 0) {
								for (int send_round{}; send_round < MAX_PLAYER; ++send_round) {
									int send_id = room_player_ids[send_round];
									if (send_id == -1) continue;
									clients[send_id].send_hit_packet(clients[check_id].bullet[bullet_num].GetbulletId(), clients[check_id]._room_id, clients[other_id]._room_id);
								}
							}
							else {
								TIMER_EVENT ev{ clients[other_id]._id, chrono::system_clock::now() + 5s, EV_RESPAWN, 0};
								timer_queue.push(ev);
								clients[other_id].isDeath = true;
								for (int send_round{}; send_round < MAX_PLAYER; ++send_round) {
									int send_id = room_player_ids[send_round];
									if (send_id == -1) continue;
									//cout << "�÷��̾� [" << clients[other_id]._room_id << "] �� �׾��ٴ� �� �÷��̾� [" << clients[send_id]._room_id << "] ���� ����" << endl;
									clients[send_id].send_dead_packet(clients[check_id].bullet[bullet_num].GetbulletId(), clients[check_id]._room_id, clients[other_id]._room_id);
								}
							}
						}
					}
				}
			}
		}
	}
}

void InitRoom()
{
	int room_num = 0;
	for (Room& r : rooms) {
		r.SetRoomNum(room_num);
		room_num++;
	}

	for (short c : clients_room) {
		c = -1;
	}
}

void do_timer()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);
				this_thread::sleep_for(1ms);
				continue;
			}
			switch (ev.event_id) {
			case EV_RESPAWN: {
				Overlapped* ov = new Overlapped;
				ov->_overlapped_type = OT_RESPAWN;
				PostQueuedCompletionStatus(iocp_h, 1, ev.obj_id, &ov->_over);
				break;
			}
			}
			continue;
		}
		this_thread::sleep_for(1ms);
	}
}

void do_db()
{
	//db_controll.Search_User(L"akjsnb12", L"123");
	while (true) {
		DB_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == db_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				db_queue.push(ev);
				this_thread::sleep_for(1ms);
				continue;
			}
			switch (ev.event_id) {
			case TRY_LOGIN: {
				cout << "�α��� �õ�" << endl;
				int res = db_controll.Search_User(ev._id, ev._password);
				Overlapped* ov = new Overlapped;
				if (res == DB_SIGN_UP) {
					ov->_overlapped_type = OT_SIGNUP;
				}
				else if (res == DB_LOGIN_SUCCESS) {
					ov->_overlapped_type = OT_LOGIN_SUCCESS;
				}
				else if (res == DB_LOGIN_FAIL) {
					ov->_overlapped_type = OT_LOGIN_FAIL;
				}
				else if (res == DB_ALREADY_INGAME) {
					ov->_overlapped_type = OT_ALREADY_INGAME;
				}

				PostQueuedCompletionStatus(iocp_h, 1, ev.obj_id, &ov->_over);
				break;
			}
			case LOGIN_DISCONNECT: {
				db_controll.Disconnect_User(ev._id);
				Overlapped* ov = new Overlapped;
				ov->_overlapped_type = OT_DISCONNECT;
				PostQueuedCompletionStatus(iocp_h, 1, ev.obj_id, &ov->_over);
				break;
			}
			case UPDATE_SCORE: {
				db_controll.Update_score(ev._id, ev._score);
				Overlapped* ov = new Overlapped;
				ov->_overlapped_type = OT_UPDATE_SCORE;
				PostQueuedCompletionStatus(iocp_h, 1, ev.obj_id, &ov->_over);
				break;
			}
			}
			continue;
		}
		this_thread::sleep_for(1ms);
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

	db_controll.InitDB();
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
	db_controll.ReleaseDB();

	closesocket(g_s_socket);
	WSACleanup();
}