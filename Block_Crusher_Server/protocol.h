#pragma once

#define USE_SERVER

constexpr int SERVER_PORT = 4000;
constexpr int BUF_SIZE = 1024;

constexpr int MAX_USER = 2000;
constexpr int MAX_ROOM = 333;

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
constexpr char CS_FALL = 12;
constexpr char CS_SCORE = 14;

constexpr char SC_LOGIN = 2;
constexpr char SC_LOGIN_FAIL = 16;
constexpr char SC_LOGIN_SUCCESS = 17;
constexpr char SC_START = 3;
constexpr char SC_MOVE_PLAYER = 4;
constexpr char SC_BULLET_ADD = 6;
constexpr char SC_COLLISION = 7;
constexpr char SC_BULLET_COLLISION = 8;
constexpr char SC_HIT = 9;
constexpr char SC_DEATH = 10;
constexpr char SC_RESPAWN = 11;
constexpr char SC_FALL = 13;
constexpr char SC_RESULT = 15;

enum Animation
{
	ANIMATION_IDLE,					// assigned 0
	ANIMATION_WALK_FORAWRRD,		// assigned 1
	ANIMATION_WALK_LEFT,			// assigned 2
	ANIMATION_WALK_RIGHT,			// assigned 3
	ANIMATION_WALK_BACKWARD,		// assigned 4
	ANIMATION_SHOOT,				// assigned 5
	ANIMATION_DAMAGED,				// assigned 6
	ANIMATION_DEATH,				// assigned 7
	ANIMATION_SHOOT_FORWARD,		// assigned 8
	ANIMATION_SHOOT_LEFT,			// assigned 9
	ANIMATION_SHOOT_RIGHT,			// assigned 10
	ANIMATION_SHOOT_BACKWARD,		// assigned 11
};

enum LOGIN_STATE
{
	LS_OUTOFROOM,					// 룸 번호가 엇나감
	LS_FULLROOM,					// 선택한 룸이 다 참
	LS_SIGNUP,						// 새로운 id
	LS_LOGIN_SUCCESS,
	LS_LOGIN_FAIL,
	LS_ALREADY_INGAME,
};

#pragma pack(push, 1)

struct CS_LOGIN_PACKET {
	unsigned char		size;
	char				type;
	wchar_t				id[20];
	wchar_t				password[20];
	int					room_num;
};

struct CS_MOVE_PACKET {
	unsigned char		size;
	char				type;
	float				x;
	float				y;
	float				z;
	float				cxDelta;
	float				cyDelta;
	Animation			animation_state;
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

struct CS_FALL_PACKET {
	unsigned char		size;
	char				type;
};

struct CS_SCORE_PACKET {
	unsigned char		size;
	char				type;
	int					score;
};

//===========================
struct SC_LOGIN_FAIL_PACKET
{
	unsigned char		size;
	char				type;
	LOGIN_STATE			login_state;
};
struct SC_LOGIN_SUCCESS_PACKET
{
	unsigned char		size;
	char				type;
	LOGIN_STATE			login_state;
};
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
	char				map_key;
	int					player_id;
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
	Animation			animation_state;
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
	int					block_type;
};

struct SC_HIT_PACKET {
	unsigned char		size;
	char				type;
	int					bullet_id;
	int					player_id;
	int					enemy_id;
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

struct SC_FALL_PACKET {
	unsigned char		size;
	char				type;
	int					fall_id;
};

struct SC_RESULT_PACKET {
	unsigned char		size;
	char				type;
	bool				result;
};

#pragma pack(pop)