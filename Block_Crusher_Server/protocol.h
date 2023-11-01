#pragma once

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;

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

struct CS_MOVE_PACKET {
	unsigned char		size;
	char				type;
	char				direction;		// 0 = Up, 1 = Down, 2 = Left, 3 = Right
	short				x, y;			// 이동하는 좌표, 나중에 바꿀 것
};