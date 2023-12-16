#pragma once

#define USE_SERVER

constexpr int SERVER_PORT = 4000;
constexpr int BUF_SIZE = 1024;

constexpr int MAX_USER = 2000;

constexpr int WORLD_WIDTH = 12000;		// ���� = 60cm(px), ���� = ���� 200�� 60 * 200
constexpr int WORLD_LENGHT = 12000;		// ���� = 60cm(px), ���� = ���� 200�� 60 * 200
constexpr int WORLD_HEIGHT = 3600;		// ���� = 60cm(px), ���� = �Ʒ� 30��, �� 30�� 60 * 60

/*
	��Ŷ ���� ��Ģ
	��Ŷ�� �� ó���� unsigned char size�� ��Ŷ�� ����� ��Ÿ�� ������ ����� ��.
	�� ������ char type�� ��Ŷ�� Ÿ��, ��, ���� ������ ��Ŷ�� �������� ǥ���� �� -> ���̵� ���� ����

	CS -> 'Ŭ���̾�Ʈ'�� '����'����
	SC -> '����'�� 'Ŭ���̾�Ʈ'����
*/

// packet id
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_BULLET_ADD = 5;

constexpr char SC_LOGIN = 2;
constexpr char SC_START = 3;
constexpr char SC_MOVE_PLAYER = 4;
constexpr char SC_BULLET_ADD = 6;
constexpr char SC_COLLISION = 7;
constexpr char SC_BULLET_COLLISION = 8;
constexpr char SC_HIT = 9;
constexpr char SC_DEATH = 10;
constexpr char SC_RESPAWN = 11;

#pragma pack(push, 1)

struct CS_LOGIN_PACKET {
	unsigned char		size;
	char				type;
};

struct CS_MOVE_PACKET {
	unsigned char		size;
	char				type;
	float				x;
	float				y;
	float				z;
	float				cxDelta;
	float				cyDelta;
};

struct CS_BULLET_ADD_PACKET {
	unsigned char		size;
	char				type;
	float				s_x;
	float				s_y;
	float				s_z;
	float				b_x;
	float				b_y;
	float				b_z;
	int					bullet_id;
};

//===========================
struct SC_LOGININFO_PACKET
{
	unsigned char		size;
	char				type;
	int					id;
	float				x;
	float				y;
	float				z;
};
struct SC_START_PACKET
{
	unsigned char		size;
	char				type;
	// �ʱ� ���� ��ġ
};

struct SC_MOVE_PACKET {
	unsigned char		size;
	char				type;
	int					id;
	float				x;
	float				y;
	float				z;
	float				cxDelta;
	float				cyDelta;
};

struct SC_BULLET_ADD_PACKET {
	unsigned char		size;
	char				type;
	float				s_x;
	float				s_y;
	float				s_z;
	float				b_x;
	float				b_y;
	float				b_z;
	int					player_id;
	int					bullet_id;
};

struct SC_COLLISION_PACKET {
	unsigned char		size;
	char				type;
	int					coll_obj_id1;
	int					coll_obj_id2;
};

struct SC_BULLET_COLLISION_PACKET {
	unsigned char		size;
	char				type;
	int					bullet_id;
	int					block_id;
	int					player_id;
};

struct SC_HIT_PACKET {
	unsigned char		size;
	char				type;
	int					bullet_id;
	int					player_id;
};

struct SC_DEATH_PACKET {
	unsigned char		size;
	char				type;
	int					bullet_id;
	int					player_id;
	int					death_id;
};

struct SC_RESPAWN_PACKET {
	unsigned char		size;
	char				type;
	int					player_id;
	float				respawn_x;
	float				respawn_y;
	float				respawn_z;
};

#pragma pack(pop)