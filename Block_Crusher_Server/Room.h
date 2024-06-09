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
	int									clients_id[MAX_PLAYER];		// �뿡 �������� Ŭ���̾�Ʈ���� id
	int									room_num;					// �� ��ȣ
	ROOM_STATE							room_state;					// ���� ����
	std::atomic_int						score_person;				// ���ھ ������ ��
	float								block_spawn_time;			// ��� ���� �ð�
	int									map_block_num;				// �� ��� ����
public:
	std::mutex							_r_lock;					// 
	Map									map_information;			// ���� ��
	std::atomic_int						max_score;					// �ִ� ���ھ�

	std::atomic_int						clients_number;				// �������� Ŭ���̾�Ʈ�� ��
	std::set<Block_pos>					Block_Spawn_Pos;			// ��� ���� ��ǥ��
	std::vector<Falling_Block_pos>		Falling_Blocks;				// �������� �ִ� ��ϵ��� ��ǥ��
public:
	Room();
	~Room();
	void SettingRoom();								// ������ �� �� �̿��ϰ� �ٽ� �̿��� ��
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