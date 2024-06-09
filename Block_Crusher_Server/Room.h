#pragma once
#include "header.h"
#include "Map.h"

constexpr int MAX_PLAYER = 3;
enum ROOM_STATE { RS_WAITING, RS_READY, RS_INGAME };

struct Block_pos {
	int x;
	int z;

	bool operator<(const Block_pos& other) const {
		if (x == other.x) {
			return z < other.z;
		}
		return x < other.x;
	}
};

struct Falling_Block_pos {
	int x;
	int y;
	int z;
};

class Room
{
private:
	int									clients_id[MAX_PLAYER];		// 룸에 참가중인 클라이언트들의 id
	int									room_num;					// 룸 번호
	ROOM_STATE							room_state;					// 룸의 상태
	std::atomic_int						score_person;				// 스코어에 참여한 수
	float								block_spawn_time;			// 블록 생성 시계
	int									map_block_num;				// 맵 블록 개수
public:
	std::mutex							_r_lock;					// 
	Map									map_information;			// 룸의 맵
	std::atomic_int						max_score;					// 최대 스코어

	std::atomic_int						clients_number;				// 참가중인 클라이언트의 수
	std::set<Block_pos>					Block_Spawn_Pos;			// 블록 생성 좌표들
	std::vector<Falling_Block_pos>		Falling_Blocks;				// 떨어지고 있는 블록들의 좌표들
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
	void AddTime(float input_time);
	float GetTime() { return block_spawn_time; };
	void SpawnBlock();
	int GetMapBlockNum() { return map_block_num; };
	void AddMapBlockNum(int input_num);
};