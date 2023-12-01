#pragma once

#define USE_SERVER

constexpr int SERVER_PORT = 4000;
constexpr int BUF_SIZE = 1024;

constexpr int MAX_USER = 2000;

constexpr int WORLD_WIDTH = 12000;		// 블럭 = 60cm(px), 가로 = 블럭 200개 60 * 200
constexpr int WORLD_LENGHT = 12000;		// 블럭 = 60cm(px), 세로 = 블럭 200개 60 * 200
constexpr int WORLD_HEIGHT = 3600;		// 블럭 = 60cm(px), 높이 = 아래 30개, 위 30개 60 * 60

/*
	패킷 생성 규칙
	패킷의 맨 처음은 unsigned char size로 패킷의 사이즈를 나타낼 변수로 사용할 것.
	그 다음은 char type로 패킷의 타입, 즉, 무슨 종류의 패킷을 보내는지 표시할 것 -> 아이디를 만들어서 구분

	CS -> '클라이언트'가 '서버'에게
	SC -> '서버'가 '클라이언트'에게
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
	// 초기 지형 위치
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