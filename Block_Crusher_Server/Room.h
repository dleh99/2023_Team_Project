#pragma once
#include "header.h"
#include "Map.h"

constexpr int MAX_PLAYER = 3;
enum ROOM_STATE { RS_WAITING, RS_READY, RS_INGAME };

class Room
{
private:
	int					clients_id[MAX_PLAYER];		// �뿡 �������� Ŭ���̾�Ʈ���� id
	std::atomic_int		clients_number;				// �������� Ŭ���̾�Ʈ�� ��
	int					room_num;					// �� ��ȣ
	ROOM_STATE			room_state;					// ���� ����
	std::atomic_int		score_person;				// ���ھ ������ ��
public:
	std::mutex			_r_lock;					// 
	Map					map_information;			// ���� ��
	std::atomic_int		max_score;					// �ִ� ���ھ�
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
};