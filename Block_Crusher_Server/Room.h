#pragma once
#include "header.h"
#include "Map.h"

constexpr int MAX_PLAYER = 3;
enum ROOM_STATE { RS_WAITING, RS_READY, RS_INGAME };

class Room
{
private:
	int					clients_id[MAX_PLAYER];		// 룸에 참가중인 클라이언트들의 id
	std::atomic_int		clients_number;				// 참가중인 클라이언트의 수
	int					room_num;					// 룸 번호
	ROOM_STATE			room_state;					// 룸의 상태
	std::atomic_int		score_person;				// 스코어에 참여한 수
public:
	std::mutex			_r_lock;					// 
	Map					map_information;			// 룸의 맵
	std::atomic_int		max_score;					// 최대 스코어
public:
	Room();
	~Room();
	void SettingRoom();								// 게임을 한 번 이용하고 다시 이용할 때
	void SetRoomNum(int num);
	int GetRoomNum();
	bool PlayerIn(int p_id);
	bool PlayerOut(int p_id);
	ROOM_STATE GetRoomState();
	int* GetPlayerId();
	char GetMapKey();
	int FindPlayer(int p_id);
	void SetRoomState(ROOM_STATE rs);
	int scoreCalculate(int i_score);
};