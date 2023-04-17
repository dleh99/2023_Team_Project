#pragma once

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;

constexpr int MAX_USER = 2000;

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
	char				direction;
};