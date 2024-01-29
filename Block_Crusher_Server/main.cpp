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
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

atomic_int user_number = 0;

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
	closesocket(clients[c_id]._socket);
	cout << "[" << c_id << "] 탈퇴함" << endl;
	user_number--;
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

bool scoreCalculation()
{
	int num = 0;
	int max_score = 0;
	for (auto& cl : clients) {
		{
			lock_guard<mutex> ll{ cl._s_lock };
			if (cl._state != US_INGAME) continue;
		}
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
		cout << c_id << " 접속 완료" << endl;
		//clients[c_id]._state = US_INGAME;
		
		// 비어 있는 맵을 찾아서 들어가는 코드 작성 해야 함
		/*if (false == Map_infromation.isActive) Map_infromation.isActive = true;
		if (Map_infromation.player_num < 6) {
			Map_infromation.player_num++;
			if (Map_infromation.player_num == 6) {

			}
		}*/
		clients[c_id].send_login_info_packet();
		user_number++;
		if (user_number == 3)
			for (auto& cl : clients) {
				lock_guard<mutex> ll{ cl._s_lock };
				if (cl._state == US_CONNECTING) {
					cl._state = US_INGAME;
					cl.send_start_packet(Map_infromation.MapChar);
				}
			}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		// 패킷에서 보내온 방향에 따라 위치 이동 코드
		XMFLOAT3 accept_position{ p->x, p->y, p->z };
		clients[c_id].pos = accept_position;
		clients[c_id].cx = p->cxDelta;
		clients[c_id].cy = p->cyDelta;
		if (c_id != 0 && c_id != 1 && c_id != 2) cout << c_id << endl;
		//cout << "[" << c_id << "] " << p->animation_state << endl;

		//cout << "[" << c_id << "] 클라이언트 받은 프레임 : " << p->frame_num << endl;
		/*auto now_time = system_clock::now();
		auto exec_time = now_time - start_time;
		auto ms = duration_cast<milliseconds>(exec_time).count();*/
		//cout << ms << endl;

		// 다른 클라이언트들에게 뿌리기
		for (auto& cl : clients) {
			{
				lock_guard<mutex> ll{ cl._s_lock };
				if (cl._state != US_INGAME) continue;
			}
			if (cl._id == c_id) continue;
			cl.send_move_packet(clients.data(), c_id, p->animation_state);
	}
		break;
	}
	case CS_BULLET_ADD: {
		CS_BULLET_ADD_PACKET* p = reinterpret_cast<CS_BULLET_ADD_PACKET*>(packet);
		//cout << "[" << c_id << "] 총알 발사" << endl;
		for (int i{}; i < MAX_BULLET_NUM; ++i) {
			if (false == clients[c_id].bullet[i].GetisActive()) {
				clients[c_id].bullet[i].SetisActive(true);
				clients[c_id].bullet[i].SetPosition(p->s_x, p->s_y, p->s_z);
				clients[c_id].bullet[i].SetBulletVec(p->b_x, p->b_y, p->b_z);
				clients[c_id].bullet[i].SetBulletId(p->bullet_id);

				for (auto& cl : clients) {
					if (cl._state != US_INGAME) continue;
					if (cl._id == c_id) continue;
					//cout << "서버에서 " << cl._id << "에게 " << c_id << "가 총을 쐈다는걸 보내줬습니다" << endl;
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

		// 오류가 났을 때 처리
		if (FALSE == ret) {
			if (ex_over->_overlapped_type == OT_ACCEPT) std::cout << "Error of Accept";
			else {
				//std::cout << "Error on client [" << key << "] in GQCS" << std::endl;
				disconnect(key);
				if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
				continue;
			}
		}

		// 온 데이터가 없을 때
		if ((byte_size == 0) && ((ex_over->_overlapped_type == OT_RECV) || (ex_over->_overlapped_type == OT_SEND))) {
			disconnect(key);
			if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
			continue;
		}

		// 받은 데이터 타입에 따른 처리
		switch (ex_over->_overlapped_type)
		{
			case OT_ACCEPT:
			{
				int client_id = set_client_id();
				if (client_id != -1)
				{
					// 초기값 지정
					clients[client_id].pos.x = 0;
					clients[client_id].pos.y = 0;
					clients[client_id]._id = client_id;
					clients[client_id]._prev_remain = 0;
					clients[client_id]._socket = g_c_socket;

					// 상태 변환(접속중)
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
				// 패킷 재조립
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						packet_process(static_cast<int>(key), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				// 데이터 꼭다리
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
				// 타이머 큐에서 리스폰 하라는 메시지를 보낸다면 
				clients[key].hp = 10;

				XMFLOAT3 random_pos = physics_engine.PickPos();

				for (auto& send_cl : clients) {
					{
						lock_guard<mutex> ll{ send_cl._s_lock };
						if (send_cl._state != US_INGAME) continue;
					}
					cout << "[" << send_cl._id << "] 에게 " << cl._id << "가 부활했다고 보냄" << endl;
					send_cl.send_respawn_packet(random_pos.x, random_pos.y, random_pos.z, cl._id);
				}
				break;
			}
		}
	}
}

void Physics_Calculation_thread()
{
	/* 이 스레드가 하는 일
	* 1. 활성화 되어 있는 총알들의 움직임 제어
	*		-> 총알을 일정 시간 마다 움직임
	* 2. 총알들과 오브젝트들의 충돌 계산
	*		-> 충돌 시 패킷 보내기
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

		// 총알과 충돌 처리
		/*
		* 클라이언트 전체를 돌아서 인게임 중인 클라이언트의 총알들과 맵의 충돌을 검사하고
		* 충돌했다면 Active를 false 시키고 모든 클라이언트들에게 충돌했다는 패킷을 보낸다
		*/
		for (auto& cl : clients) {
			if (cl._state != US_INGAME) continue;
			for (int i{}; i < MAX_BULLET_NUM; ++i) {
				if (false == cl.bullet[i].GetisActive()) continue;
				// 총알과 블록 충돌 처리
				for (int j{}; j < Map_infromation.block_num; ++j) {
					if (false == Map_infromation.Map_Block[j].GetisActive()) continue;
					if (CollisionCheck_objects(cl.bullet[i].GetPosition(), Map_infromation.Map_Block[j].GetPosition(),
						cl.bullet[i].GetRadius(), Map_infromation.Map_Block[j].GetRadius())){
						cl.bullet[i].SetisActive(false);
						Map_infromation.Map_Block[j].SetisActive(false);
						//cout << "충돌함" << endl;
						for (auto& send_cl : clients) {
							if (send_cl._state != US_INGAME) continue;
							send_cl.send_bullet_collision_packet(cl.bullet[i].GetbulletId(), Map_infromation.Map_Block[j].GetId(),
								cl._id, Map_infromation.Map_Block[j].GetBlockType());
						}
					}
				}
				// 플레이어와 총알의 충돌 처리
				if (false == cl.bullet[i].GetisActive()) continue;
				for (auto& other_player : clients) {
					if (other_player._state != US_INGAME) continue;
					if (other_player._id == cl._id) continue;
					//if (true == other_player.isDeath) continue;
					if (CollisionCheck_Person(cl.bullet[i].GetPosition(), other_player.pos,
						cl.bullet[i].GetRadius(), other_player._player_radius)) {
						cl.bullet[i].SetisActive(false);
						
						// 맞았을 때 체력을 깎는다.
						// 체력 처리 어떻게 할건가? -> 이거 mutex 걸어야 함? 근데 이것도 mutex 걸거면 위치도 걸어야 하는거 아님? 근데 그럼 성능 안 나오는거 아님?
						if (false == other_player.isinvincible) {
							//cout << "플레이어 [" << other_player._id << "] 맞았다" << endl;
							//other_player.isinvincible = true;
							other_player.hp -= 1;

							// 살아있다면 맞았다는 패킷을, 죽었다면 죽었다는 패킷을 보낸다
							if (other_player.hp > 0) {
								for (auto& send_cl : clients) {
									if (send_cl._state != US_INGAME) continue;
									send_cl.send_hit_packet(cl.bullet[i].GetbulletId(), cl._id, other_player._id);
								}
							}
							else
							{
								// 죽었을 때 처리 어떻게?
								// 타이머 스레드에게 일감을 줘서 5초 리스폰 하라 해
								TIMER_EVENT ev{ other_player._id, chrono::system_clock::now() + 5s, EV_RESPAWN, 0 };
								timer_queue.push(ev);
								//other_player.isDeath = true;
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
		// 무적이 적용되어 있는 클라이언트들 시간 더하기, 일단 무적시간 3초
		// 죽은 플레이어가 있다면 타이머 돌리기. 리스폰 시간 5초
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

		//			//cout << "플레이어 [" << cl._id << "] 부활" << endl;
		//			for (auto& send_cl : clients) {
		//				if (send_cl._state != US_INGAME)continue;
		//				cout << "[" << send_cl._id << "] 에게 " << cl._id << "가 부활했다고 보냄" << endl;
		//				send_cl.send_respawn_packet(random_pos.x, random_pos.y, random_pos.z, cl._id);
		//			}
		//		}
		//	}
		//}
	}
}

void InitDB()
{

}

void do_timer()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);			// 이 부분은 최적화가 필요함
				// 넣고 빼는 작업 없이 할 수 있는 방법은 없나?
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
		this_thread::sleep_for(1ms);		// 큐에 작업이 없으니 잠시 대기했다가 다시 시작
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

	InitDB();

	vector<thread> worker_threads;
	int thread_num = thread::hardware_concurrency();
	for (int i{}; i < thread_num - 2; ++i)
		worker_threads.emplace_back(worker_thread, iocp_h);
	
	// 계산 스레드
	thread physics_thread{ Physics_Calculation_thread };
	physics_thread.join();

	// 타이머 스레드
	thread timer_thread{ do_timer };
	timer_thread.join();

	for (auto& thread : worker_threads) 
		thread.join();

	closesocket(g_s_socket);
	WSACleanup();
}