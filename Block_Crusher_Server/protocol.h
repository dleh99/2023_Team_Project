#pragma once

#define USE_SERVER

constexpr int SERVER_PORT = 4000;
constexpr int BUF_SIZE = 1024;

constexpr int MAX_USER = 2000;

constexpr int WORLD_WIDTH = 12000;		// �� = 60cm(px), ���� = �� 200�� 60 * 200
constexpr int WORLD_LENGHT = 12000;		// �� = 60cm(px), ���� = �� 200�� 60 * 200
constexpr int WORLD_HEIGHT = 3600;		// �� = 60cm(px), ���� = �Ʒ� 30��, �� 30�� 60 * 60

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
};

#pragma pack(pop)