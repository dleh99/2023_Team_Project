#include "Room.h"

Room::Room()
{
	for (int i{}; i < MAX_PLAYER; ++i) 
		clients_id[i] = -1;		// 초기 클라이언트 id
	clients_number = 0;
	room_state = RS_WAITING;
	room_num = -1;
	max_score = -1;
	score_person = 0;
}

Room::~Room()
{
}

void Room::SettingRoom()
{
	for (int i{}; i < MAX_PLAYER; ++i)
		clients_id[i] = -1;
	clients_number = 0;
	max_score = -1;
	score_person = 0;
	room_state = RS_WAITING;
	map_information.ClearMap();
	map_information.CreateMap();
}

void Room::SetRoomNum(int num)
{
	room_num = num;
	//std::cout << room_num << std::endl;
}

int Room::GetRoomNum()
{
	return room_num;
}

bool Room::PlayerIn(int p_id)
{
	// 여기 CAS 써야 할 것 같은데?
	// CAS를 쓰던가 lock을 쓰던가 일단 lock 씀
	std::lock_guard<std::mutex> ll{ _r_lock };
	if (-1 == FindPlayer(p_id)) {
		for (int i{}; i < MAX_PLAYER; ++i) {
			if (clients_id[i] == -1) {
				clients_id[i] = p_id;
				clients_number++;
				std::cout << "[들어옴] - " << i << "를 바꿈. 현재원 : " << clients_number << std::endl;
				if (clients_number == MAX_PLAYER)
				{
					room_state = RS_READY;
				}
				return true;
			}
		}
	}
	return false;
}

bool Room::PlayerOut(int p_id)
{
	int key = FindPlayer(p_id);
	std::cout << key << std::endl;
	// 여기 CAS 써야 할 것 같은데?
	// CAS를 쓰던가 lock을 쓰던가 일단 lock 씀
	std::lock_guard<std::mutex> ll{ _r_lock };
	if (-1 != key) {
		clients_id[key] = -1;
		clients_number--;
		//std::cout << "[나감] " << clients_number << std::endl;
		return true;
	}
	return false;
}

ROOM_STATE Room::GetRoomState()
{
	return room_state;
}

int* Room::GetPlayerId()
{
	return clients_id;
}

char Room::GetMapKey()
{
	return map_information.MapChar;
}

int Room::FindPlayer(int p_id)
{
	for (int i{}; i < MAX_PLAYER; ++i) {
		if (clients_id[i] == p_id) {
			return i;
		}
	}
	return -1;
}

void Room::SetRoomState(ROOM_STATE rs)
{
	_r_lock.lock();
	room_state = rs;
	_r_lock.unlock();
}

int Room::scoreCalculate(int i_score)
{
	if (i_score > max_score) max_score = i_score;
	score_person++;
	if (score_person == clients_number) return max_score;
	return -1;
}
